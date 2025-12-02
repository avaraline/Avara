
#include "backward.hpp"
#include "Logging.h"
/*************** SIGNALS HANDLING ***************/

#if defined(BACKWARD_SYSTEM_LINUX) || defined(BACKWARD_SYSTEM_DARWIN)

class SignalHandling {
public:
  static std::vector<int> make_default_signals() {
    const int posix_signals[] = {
      // Signals for which the default action is "Core".
      SIGABRT, // Abort signal from abort(3)
      SIGBUS,  // Bus error (bad memory access)
      SIGFPE,  // Floating point exception
      SIGILL,  // Illegal Instruction
      SIGIOT,  // IOT trap. A synonym for SIGABRT
      SIGQUIT, // Quit from keyboard
      SIGSEGV, // Invalid memory reference
      SIGSYS,  // Bad argument to routine (SVr4)
      SIGTRAP, // Trace/breakpoint trap
      SIGXCPU, // CPU time limit exceeded (4.2BSD)
      SIGXFSZ, // File size limit exceeded (4.2BSD)
#if defined(BACKWARD_SYSTEM_DARWIN)
      SIGEMT, // emulation instruction executed
#endif
    };
    return std::vector<int>(posix_signals,
                            posix_signals +
                                sizeof posix_signals / sizeof posix_signals[0]);
  }

  SignalHandling(const std::vector<int> &posix_signals = make_default_signals())
      : _loaded(false) {
    bool success = true;

    const size_t stack_size = 1024 * 1024 * 8;
    _stack_content.reset(static_cast<char *>(malloc(stack_size)));
    if (_stack_content) {
      stack_t ss;
      ss.ss_sp = _stack_content.get();
      ss.ss_size = stack_size;
      ss.ss_flags = 0;
      if (sigaltstack(&ss, nullptr) < 0) {
        success = false;
      }
    } else {
      success = false;
    }

    for (size_t i = 0; i < posix_signals.size(); ++i) {
      struct sigaction action;
      memset(&action, 0, sizeof action);
      action.sa_flags =
          static_cast<int>(SA_SIGINFO | SA_ONSTACK | SA_NODEFER | SA_RESETHAND);
      sigfillset(&action.sa_mask);
      sigdelset(&action.sa_mask, posix_signals[i]);
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif
      action.sa_sigaction = &sig_handler;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

      int r = sigaction(posix_signals[i], &action, nullptr);
      if (r < 0)
        success = false;
    }

    _loaded = success;
  }

  bool loaded() const { return _loaded; }

  static void handleSignal(int, siginfo_t *info, void *_ctx) {
    ucontext_t *uctx = static_cast<ucontext_t *>(_ctx);

    backward::StackTrace st;
    void *error_addr = nullptr;
#ifdef REG_RIP // x86_64
    error_addr = reinterpret_cast<void *>(uctx->uc_mcontext.gregs[REG_RIP]);
#elif defined(REG_EIP) // x86_32
    error_addr = reinterpret_cast<void *>(uctx->uc_mcontext.gregs[REG_EIP]);
#elif defined(__arm__)
    error_addr = reinterpret_cast<void *>(uctx->uc_mcontext.arm_pc);
#elif defined(__aarch64__)
    #if defined(__APPLE__)
      error_addr = reinterpret_cast<void *>(uctx->uc_mcontext->__ss.__pc);
    #else
      error_addr = reinterpret_cast<void *>(uctx->uc_mcontext.pc);
    #endif
#elif defined(__mips__)
    error_addr = reinterpret_cast<void *>(
        reinterpret_cast<struct sigcontext *>(&uctx->uc_mcontext)->sc_pc);
#elif defined(__APPLE__) && defined(__POWERPC__)
    error_addr = reinterpret_cast<void *>(uctx->uc_mcontext->__ss.__srr0);
#elif defined(__ppc__) || defined(__powerpc) || defined(__powerpc__) ||        \
    defined(__POWERPC__)
    error_addr = reinterpret_cast<void *>(uctx->uc_mcontext.regs->nip);
#elif defined(__riscv)
    error_addr = reinterpret_cast<void *>(uctx->uc_mcontext.__gregs[REG_PC]);
#elif defined(__s390x__)
    error_addr = reinterpret_cast<void *>(uctx->uc_mcontext.psw.addr);
#elif defined(__APPLE__) && defined(__x86_64__)
    error_addr = reinterpret_cast<void *>(uctx->uc_mcontext->__ss.__rip);
#elif defined(__APPLE__)
    error_addr = reinterpret_cast<void *>(uctx->uc_mcontext->__ss.__eip);
#elif defined(__loongarch__)
    error_addr = reinterpret_cast<void *>(uctx->uc_mcontext.__pc);
#else
#warning ":/ sorry, ain't know no nothing none not of your architecture!"
#endif
    if (error_addr) {
      st.load_from(error_addr, 32, reinterpret_cast<void *>(uctx),
                   info->si_addr);
    } else {
      st.load_here(32, reinterpret_cast<void *>(uctx), info->si_addr);
    }

    backward::Printer printer;
    printer.address = true;
    printer.snippet = true;
    printer.print(st, stderr);
    // print a copy to the log file
    auto &log = Logging::LogFile();
    printer.print(st, log);
    log.flush();

#if (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 700) || \
    (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L)
    psiginfo(info, nullptr);
#else
    (void)info;
#endif
  }

private:
  backward::details::handle<char *> _stack_content;
  bool _loaded;

#ifdef __GNUC__
  __attribute__((noreturn))
#endif
  static void
  sig_handler(int signo, siginfo_t *info, void *_ctx) {
    handleSignal(signo, info, _ctx);

    // try to forward the signal.
    raise(info->si_signo);

    // terminate the process immediately.
    puts("watf? exit");
    _exit(EXIT_FAILURE);
  }
};

#endif // BACKWARD_SYSTEM_LINUX || BACKWARD_SYSTEM_DARWIN

#ifdef BACKWARD_SYSTEM_WINDOWS

class SignalHandling {
public:
  SignalHandling(const std::vector<int> & = std::vector<int>())
      : reporter_thread_([]() {
          /* We handle crashes in a utility thread:
            backward structures and some Windows functions called here
            need stack space, which we do not have when we encounter a
            stack overflow.
            To support reporting stack traces during a stack overflow,
            we create a utility thread at startup, which waits until a
            crash happens or the program exits normally. */

          {
            std::unique_lock<std::mutex> lk(mtx());
            cv().wait(lk, [] { return crashed() != crash_status::running; });
          }
          if (crashed() == crash_status::crashed) {
            handle_stacktrace(skip_recs());
          }
          {
            std::unique_lock<std::mutex> lk(mtx());
            crashed() = crash_status::ending;
          }
          cv().notify_one();
        }) {
    SetUnhandledExceptionFilter(crash_handler);

    signal(SIGABRT, signal_handler);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    std::set_terminate(&terminator);
#ifndef BACKWARD_ATLEAST_CXX17
    std::set_unexpected(&terminator);
#endif
    _set_purecall_handler(&terminator);
    _set_invalid_parameter_handler(&invalid_parameter_handler);
  }
  bool loaded() const { return true; }

  ~SignalHandling() {
    {
      std::unique_lock<std::mutex> lk(mtx());
      crashed() = crash_status::normal_exit;
    }

    cv().notify_one();

    reporter_thread_.join();
  }

private:
  static CONTEXT *ctx() {
    static CONTEXT data;
    return &data;
  }

  enum class crash_status { running, crashed, normal_exit, ending };

  static crash_status &crashed() {
    static crash_status data;
    return data;
  }

  static std::mutex &mtx() {
    static std::mutex data;
    return data;
  }

  static std::condition_variable &cv() {
    static std::condition_variable data;
    return data;
  }

  static HANDLE &thread_handle() {
    static HANDLE handle;
    return handle;
  }

  std::thread reporter_thread_;

  // TODO: how not to hardcode these?
  static const constexpr int signal_skip_recs =
#ifdef __clang__
      // With clang, RtlCaptureContext also captures the stack frame of the
      // current function Below that, there are 3 internal Windows functions
      4
#else
      // With MSVC cl, RtlCaptureContext misses the stack frame of the current
      // function The first entries during StackWalk are the 3 internal Windows
      // functions
      3
#endif
      ;

  static int &skip_recs() {
    static int data;
    return data;
  }

  static inline void terminator() {
    crash_handler(signal_skip_recs);
    abort();
  }

  static inline void signal_handler(int) {
    crash_handler(signal_skip_recs);
    abort();
  }

  static inline void __cdecl invalid_parameter_handler(const wchar_t *,
                                                       const wchar_t *,
                                                       const wchar_t *,
                                                       unsigned int,
                                                       uintptr_t) {
    crash_handler(signal_skip_recs);
    abort();
  }

  NOINLINE static LONG WINAPI crash_handler(EXCEPTION_POINTERS *info) {
    // The exception info supplies a trace from exactly where the issue was,
    // no need to skip records
    crash_handler(0, info->ContextRecord);
    return EXCEPTION_CONTINUE_SEARCH;
  }

  NOINLINE static void crash_handler(int skip, CONTEXT *ct = nullptr) {

    if (ct == nullptr) {
      RtlCaptureContext(ctx());
    } else {
      memcpy(ctx(), ct, sizeof(CONTEXT));
    }
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
                    GetCurrentProcess(), &thread_handle(), 0, FALSE,
                    DUPLICATE_SAME_ACCESS);

    skip_recs() = skip;

    {
      std::unique_lock<std::mutex> lk(mtx());
      crashed() = crash_status::crashed;
    }

    cv().notify_one();

    {
      std::unique_lock<std::mutex> lk(mtx());
      cv().wait(lk, [] { return crashed() != crash_status::crashed; });
    }
  }

  static void handle_stacktrace(int skip_frames = 0) {
    // printer creates the TraceResolver, which can supply us a machine type
    // for stack walking. Without this, StackTrace can only guess using some
    // macros.
    // StackTrace also requires that the PDBs are already loaded, which is done
    // in the constructor of TraceResolver
    backward::Printer printer;

    backward::StackTrace st;
    st.set_machine_type(printer.resolver().machine_type());
    st.set_thread_handle(thread_handle());
    st.load_here(32 + skip_frames, ctx());
    st.skip_n_firsts(skip_frames);

    printer.address = true;
    printer.snippet = true;
    printer.print(st, std::cerr);
    // print a copy to the log file
    auto &log = Logging::LogFile();
    printer.print(st, log);
    log.flush();
  }
};

#endif // BACKWARD_SYSTEM_WINDOWS

#ifdef BACKWARD_SYSTEM_UNKNOWN

class SignalHandling {
public:
  SignalHandling(const std::vector<int> & = std::vector<int>()) {}
  bool init() { return false; }
  bool loaded() { return false; }
};

#endif // BACKWARD_SYSTEM_UNKNOWN
