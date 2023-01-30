cmake_minimum_required(VERSION 3.12)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
project(Avara VERSION 0.7.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(VERBOSE True)
set(OpenGL_GL_PREFERENCE GLVND)

set(CMAKE_CXX_FLAGS "-MMD -MP -g -Wno-multichar -DNANOGUI_GLAD")

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

find_package(SDL2 CONFIG REQUIRED)

find_path(MINIUPNPC_INCLUDE_DIRS NAMES miniupnpc.h PATH_SUFFIXES miniupnpc)
find_library(MINIUPNPC_LIBRARIES miniupnpc)

find_package(glad CONFIG REQUIRED)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

execute_process (
    COMMAND git describe --always --dirty
    OUTPUT_VARIABLE GIT_HASH
)
string(STRIP ${GIT_HASH} GIT_HASH)

execute_process (
    COMMAND echo "#define GIT_VERSION \"${GIT_HASH}\""
    OUTPUT_FILE ${CMAKE_SOURCE_DIR}/src/util/GitVersion.h
)

add_compile_definitions(GIT_VERSION="${GIT_HASH}")

# Modules for copying file, finding OpenGL and SDL libraries
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

include(TargetCopyFiles)

if(NOT WIN32)
   find_package(OpenGL REQUIRED)
endif()

if(WIN32 OR MSVC)
    set(CMAKE_CXX_FLAGS "/EHsc /std:c++17 /DNANOGUI_GLAD")
    list(APPEND EXTRA_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 shell32 opengl32 glu32 wsock32 ws2_32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
    add_compile_definitions(NOMINMAX)
endif()


add_custom_target(datafiles ALL)
add_copy_directory(datafiles shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)
add_copy_directory(datafiles rsrc DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/rsrc)
add_copy_directory(datafiles bsps DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bsps)
add_copy_directory(datafiles levels DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/levels)


file(GLOB_RECURSE CORE_SOURCE RELATIVE ${CMAKE_SOURCE_DIR} 
    "src/audio/*.cpp" 
    "src/base/*.cpp" 
    "src/bsp/*.cpp" 
    "src/compat/*.cpp"
    "src/game/*.cpp"
    "src/level/*.cpp"
    "src/net/*.cpp"
    "src/util/*.cpp"
    "src/gui/*.cpp" 
    "src/tui/*.cpp"
    "vendor/nanogui/*.cpp"
    "vendor/glm/detail/glm.cpp"
    "vendor/nanovg/nanovg.c"
    "vendor/pugixml/pugixml.cpp"
    "vendor/csscolorparser.cpp"
    "vendor/stb_vorbis.c"
)

if(APPLE)
	list(APPEND CORE_SOURCE "vendor/nanogui/darwin.mm")
	set(MACOSX_BUNDLE_GUI_IDENTIFIER com.avaraline.avara)
	set(MACOSX_BUNDLE_BUNDLE_NAME Avara)
	set(MACOSX_BUNDLE_INFO_PLIST "platform/macos/Info.plist")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.9")
endif()

add_library(AvaraCore OBJECT ${CORE_SOURCE})

set(CORE_INCLUDES
    "${SDL2_INCLUDE_DIR}"
    ${SDL2main_INCLUDE_DIRS} 
    ${MINIUPNPC_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR} 
    ${CMAKE_BINARY_DIR}
    src
    src/audio
    src/base
    src/bsp
    src/compat
    src/game
    src/gui
    src/tui
    src/level
    src/net
    src/util
    src/util/huffman
    vendor 
    vendor/glad
    vendor/glm
    vendor/glm/detail
    vendor/glm/ext
    vendor/glm/gtc
    vendor/glm/gtx
    vendor/glm/simd
    vendor/nanogui
    vendor/nanovg
    vendor/utf8
    vendor/pugixml
)

if (NOT APPLE AND NOT MSVC AND NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++ -lm -lpthread")
endif()

add_executable(Avara 
    src/Avara.cpp
    $<TARGET_OBJECTS:AvaraCore>
)

add_executable(tests
    src/tests.cpp
    $<TARGET_OBJECTS:AvaraCore>
    vendor/gtest/src/gtest-all.cc
)

add_executable(hsnd2wav 
    src/hsnd2wav.cpp
    $<TARGET_OBJECTS:AvaraCore>
)

add_dependencies(Avara datafiles)
add_dependencies(tests datafiles)
add_dependencies(hsnd2wav datafiles)

target_compile_options(Avara PRIVATE ${AVARA_COMPILE_OPTIONS})
target_compile_options(tests PRIVATE ${AVARA_COMPILE_OPTIONS})
target_compile_options(hsnd2wav PRIVATE ${AVARA_COMPILE_OPTIONS})

target_include_directories(AvaraCore PUBLIC ${CORE_INCLUDES})
target_include_directories(Avara PUBLIC ${CORE_INCLUDES})
target_include_directories(tests PUBLIC 
    ${CORE_INCLUDES}
    vendor/gtest
    vendor/gtest/include
)
target_include_directories(hsnd2wav PUBLIC ${CORE_INCLUDES})

set(AVARA_LIBS SDL2::SDL2 SDL2::SDL2main ${MINIUPNPC_LIBRARIES} {OPENGL_LIBRARIES} dl pthread)

if(WIN32)
set(AVARA_LIBS SDL2::SDL2 SDL2::SDL2main glad::glad ${MINIUPNPC_LIBRARIES} ${OPENGL_LIBRARIES})
endif()

target_link_libraries(Avara ${AVARA_LIBS})
target_link_libraries(tests ${AVARA_LIBS} ${EXTRA_LIBS})
target_link_libraries(hsnd2wav ${AVARA_LIBS} ${EXTRA_LIBS})


if(WIN32)
    target_link_libraries(Avara wsock32 ws2_32)
endif()


install(TARGETS Avara
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
install(DIRECTORY rsrc/ DESTINATION bin/rsrc/)
install(DIRECTORY shaders/ DESTINATION bin/shaders/)
install(DIRECTORY bsps/ DESTINATION bin/bsps/)
install(DIRECTORY levels/ DESTINATION bin/levels/)

if(WIN32)
    install(FILES platform/windows/SDL2-2.0.12/lib/x64/SDL2.dll DESTINATION bin/)
endif()

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Avara - a 3d networked multiplayer shooter")
set(CPACK_PACKAGE_VENDOR "Avaraline")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Avara 0.1.1")
if(WIN32 AND NOT UNIX)
	#set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/platform/windows\\\\avara.bmp")
	set(CPACK_NSIS_INSTALLED_ICON_NAME "build\\\\Avara.exe")
	set(CPACK_NSIS_DISPLAY_NAME "Avara")
	set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.github.com\\\\avaraline\\\\avara")
	set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\avara.io")
	set(CPACK_NSIS_CONTACT "#avaraline@avaraline.net")
	set(CPACK_NSIS_MODIFY_PATH ON)
else()
	set(CPACK_STRIP_FILES "build/Avara")
	set(CPACK_SOURCE_STRIP_FILES "")
endif()
set(CPACK_PACKAGE_EXECUTABLES "Avara" "Avara")
include(CPack)
