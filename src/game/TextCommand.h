
#pragma once

// move most/all? includes into TextCommand.cpp
#include <string>
#include <vector>
#include <functional>   // std::function
#include <sstream>      // std::istringstream

typedef std::vector<std::string> VectorOfArgs;
typedef std::function<bool(VectorOfArgs)> TextCommandCallback;

// This crazy macro allows you to use an instance method as a lambda/callback
// For example: METHOD_TO_LAMBDA(MyClass::SomeMethod) allows SomeMethod to be used as a callback
#define METHOD_TO_LAMBDA(method) std::bind(&method, this, std::placeholders::_1)

class TextCommand {
private:
    // TextCommand instances are registered in this static variable
    static std::vector<TextCommand*> registeredCommands;

    std::string commandStr;
    std::string usageStr;
    TextCommandCallback callback;

public:
    //
    // Class methods - much of this class is handled at the class level.
    // It's basically a singleton where commands are registered and matched against user input for execution.
    //
    static void Register(TextCommand* command);
    static bool FindMatchingCommands(std::string& fullCommand,
                                     std::function<bool(TextCommand *, VectorOfArgs)> matchCb);
    static const bool ExecuteMatchingCallbacks(std::string& fullCommand);
    static const std::string ListOfCommands(std::string delimeter = ", ");
    static std::string UsageForCommand(std::string& fullCommand);

    //
    // Instance methods - an instance of TextCommand describes a single command and it's associated callback.
    //

    // construct a TextCommand like this:
    //   TextCommand("/something  <- description of command",
    //               [](VectorOfArgs vargs) -> bool { /* define what '/something' does here */ });
    // OR like this:
    //   TextCommand("/something  <- description of command",
    //               METHOD_TO_LAMBDA(MyClass::DoSomething));
    //
    TextCommand(const char* usage, TextCommandCallback cback) {
        usageStr = usage;
        // it is assumed that /command is the first word in usage
        std::istringstream iss(usage);
        iss >> commandStr;
        callback = cback;
    }

    const std::string& GetUsage() { return usageStr; }
};
