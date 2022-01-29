# Taken from https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

CC = clang
CXX = clang++

GIT_HASH := $(shell git describe --always --dirty)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)

ifneq ($(GIT_BRANCH),)
    BUILD_DIR ?= build-$(GIT_BRANCH)
		# quiet zip on dev branches
		ZIPFLAGS := -rq
else
    BUILD_DIR ?= build
		ZIPFLAGS := -r
endif
SRC_DIRS ?= $(shell find src -type d -not -path src) vendor/glad vendor/nanovg vendor/pugixml vendor

UNAME := $(shell uname)
SRCS := $(shell find $(SRC_DIRS) -maxdepth 1 -name '*.cpp' -or -name '*.c')

INCFLAGS := $(addprefix -I, $(SRC_DIRS)) -Ivendor/gtest/include
CPPFLAGS := ${CPPFLAGS}
CPPFLAGS += $(INCFLAGS) -MMD -MP -g -Wno-multichar
CXXFLAGS := ${CXXFLAGS}
CXXFLAGS += -std=c++17
LDFLAGS := ${LDFLAGS}

ifeq ($(UNAME), Darwin)
	# MacOS
	SRCS += $(shell find $(SRC_DIRS) -maxdepth 1 -name '*.mm')
	SRCS += $(shell find vendor/miniupnpc -maxdepth 1 -name '*.c')
	INCFLAGS += -Ivendor/miniupnpc
	CXXFLAGS += -mmacosx-version-min=10.9
	CPPFLAGS += -mmacosx-version-min=10.9
	CFLAGS += -mmacosx-version-min=10.9
ifneq ("$(wildcard $(HOME)/Library/Frameworks/SDL2.framework)", "")
	FRAMEWORK_PATH = $(HOME)/Library/Frameworks
else
	FRAMEWORK_PATH = /Library/Frameworks
endif
	CPPFLAGS += -F$(FRAMEWORK_PATH)
	LDFLAGS += -F$(FRAMEWORK_PATH) -lstdc++ -lm -lpthread -framework SDL2 -framework OpenGL -framework AppKit
	POST_PROCESS ?= dsymutil
else ifneq (,$(findstring NT-10.0,$(UNAME)))
	# Windows - should match for MSYS2 on Win10
	LDFLAGS += -lstdc++ -lm -lpthread -lmingw32 -lSDL2main -lSDL2 -lglu32 -lopengl32 -lws2_32 -lcomdlg32 -lminiupnpc
	CFLAGS +=  -DMINIUPNP_EXPORTS -D_WIN32_WINNT=0x501
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
	SRCS += $(shell find vendor/miniupnpc -maxdepth 1 -name '*.c')
	INCFLAGS += -Ivendor/miniupnpc
endif

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Use the command "make macapp SIGNING_ID=yourid" if you want to use your signing id.
# Alternatively set this to "NONE" for no code signing.
SIGNING_ID := NONE

avara: set-version $(BUILD_DIR)/Avara resources build-link

tests: $(BUILD_DIR)/tests resources
	$(BUILD_DIR)/tests

bspviewer: $(BUILD_DIR)/BSPViewer resources

levelviewer: $(BUILD_DIR)/AvaraLevelViewer resources

hsnd2wav: $(BUILD_DIR)/hsnd2wav resources

frandom: $(BUILD_DIR)/frandom

fixed: $(BUILD_DIR)/fixed

macapp: avara
	rm -rf $(BUILD_DIR)/Avara.app
	$(MKDIR_P) $(BUILD_DIR)/Avara.app/Contents/{Frameworks,MacOS,Resources}
	cp platform/macos/Info.plist $(BUILD_DIR)/Avara.app/Contents
	cp $(BUILD_DIR)/Avara $(BUILD_DIR)/Avara.app/Contents/MacOS
	cp -r $(BUILD_DIR)/{levels,rsrc} $(BUILD_DIR)/Avara.app/Contents/Resources
	cp platform/macos/Avara.icns $(BUILD_DIR)/Avara.app/Contents/Resources
	cp -a $(FRAMEWORK_PATH)/SDL2.framework $(BUILD_DIR)/Avara.app/Contents/Frameworks
	install_name_tool -change @rpath/SDL2.framework/Versions/A/SDL2 @executable_path/../Frameworks/SDL2.framework/Versions/A/SDL2 $(BUILD_DIR)/Avara.app/Contents/MacOS/Avara
	if [ $(SIGNING_ID) = "NONE" ]; then echo "Not signing app bundle."; else codesign -vvv --no-strict --deep --force -s $(SIGNING_ID) $(BUILD_DIR)/Avara.app; fi
	cd $(BUILD_DIR) && zip $(ZIPFLAGS) MacAvara.zip Avara.app && cd ..

winapp: avara
	rm -rf $(BUILD_DIR)/WinAvara
	$(MKDIR_P) $(BUILD_DIR)/WinAvara
	if [ -f $(BUILD_DIR)/Avara ]; then mv $(BUILD_DIR)/Avara $(BUILD_DIR)/Avara.exe; fi
	cp -r $(BUILD_DIR)/{Avara.exe,levels,rsrc,vendor,src} $(BUILD_DIR)/WinAvara
	# cp platform/windows/*.dll $(BUILD_DIR)/WinAvara
	cp /mingw64/bin/{libstdc++-6,libwinpthread-1,libgcc_s_seh-1,SDL2,libminiupnpc}.dll $(BUILD_DIR)/WinAvara
	cd $(BUILD_DIR) && zip $(ZIPFLAGS) WinAvara.zip WinAvara && cd ..

# Avara
$(BUILD_DIR)/Avara: $(OBJS) $(BUILD_DIR)/src/Avara.cpp.o
	$(CXX) $(OBJS) $(BUILD_DIR)/src/Avara.cpp.o -o $@ $(LDFLAGS)
	$(POST_PROCESS) $@

# Tests
$(BUILD_DIR)/tests: $(OBJS) $(BUILD_DIR)/src/tests.cpp.o $(BUILD_DIR)/vendor/gtest-all.cc.o
	$(CXX) $(OBJS) $(BUILD_DIR)/vendor/gtest-all.cc.o $(BUILD_DIR)/src/tests.cpp.o -o $@ $(LDFLAGS)
	$(POST_PROCESS) $@

# Google test
$(BUILD_DIR)/vendor/gtest-all.cc.o:
	$(CXX) -isystem vendor/gtest/include/ -Ivendor/gtest/ -pthread -c vendor/gtest/src/gtest-all.cc -o $@
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

.PHONY: clean publish

set-version:
	grep -q $(GIT_HASH) src/util/GitVersion.h || (echo "#define GIT_VERSION \"$(GIT_HASH)\"" > src/util/GitVersion.h)

build-link: $(BUILD_DIR)/Avara
	@if [ ! -e build ] || [ -h build ]; then \
		echo "build -> $(BUILD_DIR)" ; \
		ln -fns $(BUILD_DIR) build ; \
	else \
		echo "build is not a link so not linking build -> $(BUILD_DIR)" ; \
	fi

clean:
	$(RM) -r $(BUILD_DIR)

clean-levels:
	$(RM) -r levels/*/alf/*.alf
	$(RM) -r levels/*/default.avarascript
	$(RM) -r levels/*/set.json
	$(RM) -r levels/*/ogg/*.ogg
	$(RM) -r levels/*/wav/*.wav

publish:
	scp $(BUILD_DIR)/Avara-*.zip avaraline.net:/srv/http/avaraline/dev/builds/

resources:
	# python3 bin/pict2svg.py
	# cp -r bsps levels rsrc shaders $(BUILD_DIR)
	rsync -av levels rsrc $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
