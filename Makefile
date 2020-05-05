# Taken from https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

CC = clang
CXX = clang++

BUILD_DIR ?= build
SRC_DIRS ?= $(shell find src -type d -not -path src) vendor/glad vendor/nanovg vendor/nanogui vendor

UNAME := $(shell uname)
SRCS := $(shell find $(SRC_DIRS) -maxdepth 1 -name '*.cpp' -or -name '*.c')

INCFLAGS := $(addprefix -I, $(SRC_DIRS)) -Ivendor -Ivendor/gtest/include
CPPFLAGS := ${CPPFLAGS}
CPPFLAGS += $(INCFLAGS) -MMD -MP -g -Wno-multichar -DNANOGUI_GLAD
CXXFLAGS := ${CXXFLAGS}
CXXFLAGS += -std=c++1y
LDFLAGS := ${LDFLAGS}

ifeq ($(UNAME), Darwin)
	# MacOS
	SRCS += $(shell find $(SRC_DIRS) -maxdepth 1 -name '*.mm')
	CPPFLAGS += -F/Library/Frameworks
	LDFLAGS += -F/Library/Frameworks -lstdc++ -lm -lpthread -framework SDL2 -framework SDL2_net -framework OpenGL -framework AppKit
	POST_PROCESS ?= dsymutil
else ifneq (,$(findstring NT-10.0,$(UNAME)))
	# Windows - should match for MSYS2 on Win10
	LDFLAGS += -lstdc++ -lm -lpthread -lmingw32 -lSDL2main -lSDL2 -lSDL2_net -lglu32 -lopengl32 -lws2_32 -lcomdlg32
	POST_PROCESS ?= ls -lh
else
	# Linux
	PKG_CONFIG ?= pkg-config
	LDFLAGS += -lstdc++ -lm -lpthread -ldl
	LDFLAGS += $(shell ${PKG_CONFIG} --libs-only-l SDL2_net)
	LDFLAGS += $(shell ${PKG_CONFIG} --libs-only-l glu)
	CPPFLAGS += $(shell ${PKG_CONFIG} --cflags-only-I directfb)
	CPPFLAGS += -fPIC
	POST_PROCESS ?= ls -lh
endif

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# This is mine. "make macapp SIGNING_ID=yourid" if you want to use your own.
# Alternatively set this to "NONE" for no code signing.
SIGNING_ID := Y56DGU8P8X


avara: $(BUILD_DIR)/Avara resources

tests: $(BUILD_DIR)/tests resources

bspviewer: $(BUILD_DIR)/BSPViewer resources

levelviewer: $(BUILD_DIR)/AvaraLevelViewer resources

hsnd2wav: $(BUILD_DIR)/hsnd2wav resources

macapp: avara
	rm -rf $(BUILD_DIR)/Avara.app
	$(MKDIR_P) $(BUILD_DIR)/Avara.app/Contents/{Frameworks,MacOS,Resources}
	cp platform/macos/Info.plist $(BUILD_DIR)/Avara.app/Contents
	cp $(BUILD_DIR)/Avara $(BUILD_DIR)/Avara.app/Contents/MacOS
	cp -r $(BUILD_DIR)/{bsps,levels,rsrc,shaders} $(BUILD_DIR)/Avara.app/Contents/Resources
	cp platform/macos/Avara.icns $(BUILD_DIR)/Avara.app/Contents/Resources
	cp -a /Library/Frameworks/{SDL2,SDL2_net}.framework $(BUILD_DIR)/Avara.app/Contents/Frameworks
	install_name_tool -change @rpath/SDL2.framework/Versions/A/SDL2 @executable_path/../Frameworks/SDL2.framework/Versions/A/SDL2 $(BUILD_DIR)/Avara.app/Contents/MacOS/Avara
	install_name_tool -change @rpath/SDL2_net.framework/Versions/A/SDL2_net @executable_path/../Frameworks/SDL2_net.framework/Versions/A/SDL2_net $(BUILD_DIR)/Avara.app/Contents/MacOS/Avara
	if [ $(SIGNING_ID) = "NONE" ]; then echo "Not signing app bundle."; else codesign -vvv --no-strict --deep --force -s $(SIGNING_ID) $(BUILD_DIR)/Avara.app; fi
	cd $(BUILD_DIR) && zip -r MacAvara.zip Avara.app && cd ..

winapp: avara
	rm -rf $(BUILD_DIR)/WinAvara
	$(MKDIR_P) $(BUILD_DIR)/WinAvara
	if [ -f $(BUILD_DIR)/Avara ]; then mv $(BUILD_DIR)/Avara $(BUILD_DIR)/Avara.exe; fi
	cp -r $(BUILD_DIR)/{Avara.exe,bsps,levels,rsrc,shaders,vendor,src} $(BUILD_DIR)/WinAvara
	# cp platform/windows/*.dll $(BUILD_DIR)/WinAvara
	cp /mingw64/bin/{libstdc++-6,libwinpthread-1,libgcc_s_seh-1,SDL2,SDL2_net}.dll $(BUILD_DIR)/WinAvara
	cd $(BUILD_DIR) && zip -r WinAvara.zip WinAvara && cd ..

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

# LevelViewer
$(BUILD_DIR)/AvaraLevelViewer: $(OBJS) $(BUILD_DIR)/src/AvaraLevelViewer.cpp.o
	$(CXX) $(OBJS) $(BUILD_DIR)/src/AvaraLevelViewer.cpp.o -o $@ $(LDFLAGS)
	$(POST_PROCESS) $@

# hsnd2wav
$(BUILD_DIR)/hsnd2wav: $(OBJS) $(BUILD_DIR)/src/hsnd2wav.cpp.o
	$(CXX) $(OBJS) $(BUILD_DIR)/src/hsnd2wav.cpp.o -o $@ $(LDFLAGS)
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

clean:
	$(RM) -r $(BUILD_DIR)

publish:
	scp build/Avara-*.zip avaraline.net:/srv/http/avaraline/dev/builds/

resources:
	# python3 bin/pict2svg.py
	cp -r bsps levels rsrc shaders $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
