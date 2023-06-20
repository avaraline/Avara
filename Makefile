# Taken from https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

CC = clang
CXX = clang++

GIT_HASH := $(shell git describe --always --dirty)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)

BUILD_DIR ?= build
ZIPFLAGS := -rq
RMDIR := $(RM) -r
MKDIR_P ?= mkdir -p

SRC_DIRS ?= $(shell find src -type d -not -path src)
SRC_DIRS += vendor/glad vendor/nanovg vendor/pugixml vendor

UNAME := $(shell uname)
SRCS := $(shell find $(SRC_DIRS) -maxdepth 1 -name '*.cpp' -or -name '*.c')

INCFLAGS := $(addprefix -I, $(SRC_DIRS))

CPPFLAGS := ${CPPFLAGS}
CPPFLAGS += $(INCFLAGS) -MMD -MP -DNANOGUI_GLAD -g -Wall

ifeq ($(AVARA_WARNINGS), TRUE)
CPPFLAGS += -pedantic -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy
CPPFLAGS += -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations
CPPFLAGS += -Wmissing-include-dirs -Woverloaded-virtual -Wredundant-decls -Wshadow
CPPFLAGS += -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef
CPPFLAGS += -Wno-unused-function -Wno-multichar -Wno-gnu-anonymous-struct -Wno-unused-parameter
endif

CXXFLAGS := ${CXXFLAGS}
CXXFLAGS += -std=c++17
LDFLAGS := ${LDFLAGS}

ifeq ($(UNAME), Darwin)
	# MacOS
	SRCS += $(shell find $(SRC_DIRS) -maxdepth 1 -name '*.mm')
	INC_EXTRA := -I/opt/homebrew/include -I/usr/local/include
	CXXFLAGS += -mmacosx-version-min=10.9 $(INC_EXTRA)
	CPPFLAGS += -mmacosx-version-min=10.9 $(INC_EXTRA)
	CFLAGS += -mmacosx-version-min=10.9 $(INC_EXTRA)
ifneq ("$(wildcard $(HOME)/Library/Frameworks/SDL2.framework)", "")
	FRAMEWORK_PATH = $(HOME)/Library/Frameworks
else
	FRAMEWORK_PATH = /Library/Frameworks
endif
	CPPFLAGS += -F$(FRAMEWORK_PATH)
	LDFLAGS += -F$(FRAMEWORK_PATH) -L/opt/homebrew/lib -L/usr/local/lib -lstdc++ -lm -lpthread -framework SDL2 -framework OpenGL -framework AppKit
	POST_PROCESS ?= dsymutil

	JOBS := $(shell sysctl -n hw.ncpu)
	# if the -j option was passed, use that instead
	MAKE_PID := $(shell echo $$PPID)
	JOB_FLAG := $(filter -j%, $(subst -j ,-j,$(shell ps T | grep "^\s*$(MAKE_PID).*$(MAKE)")))
	ifneq ($(JOB_FLAG),)
		JOBS := $(subst -j,,$(JOB_FLAG))
	endif

else ifneq (,$(findstring NT-10.0,$(UNAME)))
	# Windows - should match for MSYS2 on Win10
	WINDRES := $(shell which windres)
	PLATFORM := platform/windows
	PRE_PROCESS += $(WINDRES) $(PLATFORM)/appicon.rc -O coff $(BUILD_DIR)/appicon.o;
	PRE_PROCESS += $(WINDRES) $(PLATFORM)/version.rc -O coff $(BUILD_DIR)/version.o;
	EXTRA_OBJS += $(BUILD_DIR)/appicon.o $(BUILD_DIR)/version.o
	LDFLAGS += -lstdc++ -lm -lpthread -lmingw32 -lSDL2main -lSDL2 -lglu32 -lopengl32 -lws2_32 -lcomdlg32
	CFLAGS += -D_WIN32_WINNT=0x501
	POST_PROCESS ?= ls -lh
else
	# Linux
	PKG_CONFIG ?= pkg-config
	LDFLAGS += -lstdc++ -lm -lpthread -ldl
	LDFLAGS += $(shell ${PKG_CONFIG} --libs-only-l glu)
	LDFLAGS += $(shell ${PKG_CONFIG} --libs-only-l sdl2)
	CPPFLAGS += $(shell ${PKG_CONFIG} --cflags-only-I directfb)
	CPPFLAGS += $(shell ${PKG_CONFIG} --cflags-only-I sdl2)
	CPPFLAGS += -fPIC
	POST_PROCESS ?= ls -lh
endif

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Use the command "make macapp SIGNING_ID=yourid" if you want to use your signing id.
# Alternatively set this to "NONE" for no code signing.
SIGNING_ID := NONE

avara: set-version $(BUILD_DIR)/Avara resources

tests: set-version $(BUILD_DIR)/tests resources
	$(BUILD_DIR)/tests

bspviewer: $(BUILD_DIR)/BSPViewer resources

levelviewer: $(BUILD_DIR)/AvaraLevelViewer resources

hsnd2wav: $(BUILD_DIR)/hsnd2wav resources

frandom: $(BUILD_DIR)/frandom

fixed: $(BUILD_DIR)/fixed

macapp:
	xcodebuild -configuration Debug -scheme Avara \
        -IDEBuildOperationMaxNumberOfConcurrentCompileTasks=$(JOBS) \
        -derivedDataPath $(BUILD_DIR)/DerivedData \
        ONLY_ACTIVE_ARCH=NO \
        CONFIGURATION_BUILD_DIR=$(BUILD_DIR)

macdist: macapp
	cd $(BUILD_DIR) && zip $(ZIPFLAGS) MacAvara.zip Avara.app && cd ..

winapp: avara
	$(RMDIR) $(BUILD_DIR)/WinAvara
	$(MKDIR_P) $(BUILD_DIR)/WinAvara
	if [ -f $(BUILD_DIR)/Avara ]; then mv $(BUILD_DIR)/Avara $(BUILD_DIR)/Avara.exe; fi
	cp -r $(BUILD_DIR)/{Avara.exe,levels,rsrc} $(BUILD_DIR)/WinAvara
	cp /mingw64/bin/{libstdc++-6,libwinpthread-1,libgcc_s_seh-1,SDL2}.dll $(BUILD_DIR)/WinAvara

windist: winapp
	cd $(BUILD_DIR) && zip $(ZIPFLAGS) WinAvara.zip WinAvara && cd ..

# Avara
$(BUILD_DIR)/Avara: $(OBJS) $(BUILD_DIR)/src/Avara.cpp.o
	$(PRE_PROCESS)
	$(CXX) $(OBJS) $(EXTRA_OBJS) $(BUILD_DIR)/src/Avara.cpp.o -o $@ $(LDFLAGS)
	$(POST_PROCESS) $@

# Tests
$(BUILD_DIR)/tests: $(OBJS) $(BUILD_DIR)/src/tests.cpp.o
	$(CXX) $(OBJS) $(BUILD_DIR)/src/tests.cpp.o -o $@ $(LDFLAGS) -lgtest
	$(POST_PROCESS) $@

# BSPViewer
$(BUILD_DIR)/BSPViewer: $(OBJS) $(BUILD_DIR)/src/BSPViewer.cpp.o
	$(CC) $(OBJS) $(BUILD_DIR)/src/BSPViewer.cpp.o -o $@ $(LDFLAGS)
	$(POST_PROCESS) $@

# hsnd2wav
$(BUILD_DIR)/hsnd2wav: $(OBJS) $(BUILD_DIR)/src/hsnd2wav.cpp.o
	$(CXX) $(OBJS) $(BUILD_DIR)/src/hsnd2wav.cpp.o -o $@ $(LDFLAGS)
	$(POST_PROCESS) $@

# frandom
$(BUILD_DIR)/frandom: $(OBJS) $(BUILD_DIR)/src/frandom.cpp.o
	$(CXX) $(OBJS) $(BUILD_DIR)/src/frandom.cpp.o -o $@ $(LDFLAGS)
	$(POST_PROCESS) $@

# fixed
$(BUILD_DIR)/fixed: $(OBJS) $(BUILD_DIR)/src/fixed.cpp.o
	$(CXX) $(OBJS) $(BUILD_DIR)/src/fixed.cpp.o -o $@ $(LDFLAGS)
	$(POST_PROCESS) $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# obj-c++ source
$(BUILD_DIR)/%.mm.o: %.mm
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

set-version:
	grep -q $(GIT_HASH) src/util/GitVersion.h || (echo "#define GIT_VERSION \"$(GIT_HASH)\"" > src/util/GitVersion.h)

clean:
	$(RM) src/util/GitVersion.h
	$(RMDIR) $(BUILD_DIR)

clean-levels:
	$(RM) levels/*/alf/*.alf
	$(RM) levels/*/default.avarascript
	$(RM) levels/*/set.json
	$(RM) levels/*/ogg/*.ogg
	$(RM) levels/*/wav/*.wav

resources:
	# python3 bin/pict2svg.py
	# cp -r bsps levels rsrc shaders $(BUILD_DIR)
	rsync -av levels rsrc $(BUILD_DIR)

-include $(DEPS)
