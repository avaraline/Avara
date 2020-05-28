cmake_minimum_required(VERSION 3.1)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
project(Avara)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_CXX_STANDARD 17)
set(OpenGL_GL_PREFERENCE GLVND)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-DNANOGUI_GLAD")
if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()


# Modules for copying files, finding OpenGL and SDL libraries
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

include(TargetCopyFiles)

if(WIN32)
    set(SDL2_PATH "${CMAKE_SOURCE_DIR}/platform/windows/SDL2-2.0.12")
    set(SDL2_NET_PATH "${CMAKE_SOURCE_DIR}/platform/windows/SDL2_net-2.0.1")
endif()

find_package(SDL2 REQUIRED COMPONENTS main)
find_package(SDL2_net REQUIRED)

if(NOT WIN32)
   find_package(OpenGL REQUIRED)
endif()

if(WIN32)
    list(APPEND EXTRA_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 shell32 opengl32 glu32 wsock32 ws2_32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
endif()

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
    "vendor/nanogui/*.cpp"
    "vendor/glad/glad.c"
    "vendor/glm/detail/glm.cpp"
    "vendor/nanovg/nanovg.c"
)

if(APPLE)
	list(APPEND CORE_SOURCE "vendor/nanogui/darwin.mm")
	set(MACOSX_BUNDLE_GUI_IDENTIFIER com.avaraline.avara)
	set(MACOSX_BUNDLE_BUNDLE_NAME Avara)
	set(MACOSX_BUNDLE_INFO_PLIST "platform/macos/Info.plist")
endif()

add_library(AvaraCore OBJECT ${CORE_SOURCE})

set(CORE_INCLUDES
    ${SDL2_INCLUDE_DIRS} 
    ${SDL2main_INCLUDE_DIRS} 
    ${SDL2_NET_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR} 
    ${CMAKE_BINARY_DIR}
    src
    src/audio
    src/base
    src/bsp
    src/compat
    src/game
    src/gui
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
    vednor/glm/simd
    vendor/nanogui
    vendor/nanovg
    vendor/utf8
)

set(AVARA_COMPILE_OPTIONS -lstdc++ -lm -lpthread -ldl)

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

set(AVARA_LIBS ${SDL2_LIBRARIES} ${SDL2_NET_LIBRARIES} ${OPENGL_LIBRARIES} dl pthread)

if(WIN32)
set(AVARA_LIBS SDL2::Main SDL2::Net ${OPENGL_LIBRARIES})
endif()

target_link_libraries(Avara ${AVARA_LIBS})
target_link_libraries(tests ${AVARA_LIBS} ${EXTRA_LIBS})
target_link_libraries(hsnd2wav ${AVARA_LIBS} ${EXTRA_LIBS})


if(WIN32)
    target_link_libraries(Avara wsock32 ws2_32)
    add_custom_command(TARGET Avara POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/platform/windows/SDL2_net-2.0.1/lib/x64/SDL2_net.dll $<TARGET_FILE_DIR:Avara>)
    add_custom_command(TARGET Avara POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/platform/windows/SDL2-2.0.12/lib/x64/SDL2.dll $<TARGET_FILE_DIR:Avara>)
endif()

add_custom_target(datafiles)
add_copy_directory(datafiles shaders
    DESTINATION  $<TARGET_FILE_DIR:Avara>}/shaders
    GLOB *.glsl)

add_copy_directory(datafiles rsrc
    DESTINATION  $<TARGET_FILE_DIR:Avara>/rsrc
    GLOB *.r *.wav)

add_copy_directory(datafiles bsps
    DESTINATION  $<TARGET_FILE_DIR:Avara>/bsps
    GLOB *.json)

add_copy_directory(datafiles levels
    DESTINATION  $<TARGET_FILE_DIR:Avara>/levels
    GLOB *.r)

install(TARGETS Avara)
install(DIRECTORY shaders/ DESTINATION shaders/)
install(DIRECTORY rsrc/ DESTINATION rsrc/)
install(DIRECTORY bsps/ DESTINATION bsps/)
install(DIRECTORY levels/ DESTINATION levels/)

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