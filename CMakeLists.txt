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

find_package(GTest CONFIG REQUIRED)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}$<0:>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}$<0:>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}$<0:>")

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

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/rsrc ${PROJECT_BINARY_DIR}/rsrc
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/levels ${PROJECT_BINARY_DIR}/levels
)


if(NOT WIN32)
   find_package(OpenGL REQUIRED)
endif()

if(WIN32 OR MSVC)
    set(CMAKE_CXX_FLAGS "/EHsc /std:c++17 /DNANOGUI_GLAD")
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/platform/win/appicon.rc")
    list(APPEND EXTRA_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 shell32 opengl32 glu32 wsock32 ws2_32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
    add_compile_definitions(NOMINMAX)
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
)

add_dependencies(Avara copy_assets)
add_dependencies(tests copy_assets)

target_compile_options(Avara PRIVATE ${AVARA_COMPILE_OPTIONS})
target_compile_options(tests PRIVATE ${AVARA_COMPILE_OPTIONS})

target_include_directories(AvaraCore PUBLIC ${CORE_INCLUDES})
target_include_directories(Avara PUBLIC ${CORE_INCLUDES})
target_include_directories(tests PUBLIC 
    ${CORE_INCLUDES}
)

set(AVARA_LIBS SDL2::SDL2 SDL2::SDL2main ${MINIUPNPC_LIBRARIES} {OPENGL_LIBRARIES} dl pthread)

if(WIN32)
set(AVARA_LIBS SDL2::SDL2 SDL2::SDL2main glad::glad ${MINIUPNPC_LIBRARIES} ${OPENGL_LIBRARIES})
add_custom_command(TARGET Avara POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Avara> $<TARGET_FILE_DIR:Avara>
  COMMAND_EXPAND_LISTS
  )
endif()

target_link_libraries(Avara ${AVARA_LIBS})

target_link_libraries(tests ${AVARA_LIBS} ${EXTRA_LIBS} GTest::gtest GTest::gtest_main)

#target_link_libraries(hsnd2wav ${AVARA_LIBS} ${EXTRA_LIBS})


if(WIN32)
    target_link_libraries(Avara wsock32 ws2_32)
endif()


install(TARGETS Avara
    RUNTIME DESTINATION "."
    LIBRARY DESTINATION "."
    ARCHIVE DESTINATION "."
)

install(DIRECTORY rsrc/ DESTINATION rsrc)
install(DIRECTORY levels/ DESTINATION levels)
if(WIN32)
install(FILES $<TARGET_RUNTIME_DLLS:Avara> DESTINATION ".")
install(FILES ${PROJECT_BINARY_DIR}/miniupnpc.dll DESTINATION ".")
endif()


set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Avara - a 3d networked multiplayer shooter")
set(CPACK_PACKAGE_VENDOR "Avaraline")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Avara")
if(WIN32 AND NOT UNIX)
	set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\platform\\\\win\\\\avara.ico")
	set(CPACK_NSIS_INSTALLED_ICON_NAME "Avara.exe")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\platform\\\\win\\\\install.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\platform\\\\win\\\\uninstall.ico")
    set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_CURRENT_SOURCE_DIR}\\\\platform\\\\win\\\\installsplash.bmp")
    set(CPACK_NSIS_MUI_HEADERIMAGE "${CMAKE_CURRENT_SOURCE_DIR}\\\\platform\\\\win\\\\installheader.bmp")
    set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
    set(CPACK_NSIS_BRANDING_TEXT " ")
	set(CPACK_NSIS_DISPLAY_NAME "Avara")
    set(CPACK_NSIS_PACKAGE_NAME "Avara")
	set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.github.com\\\\avaraline\\\\avara")
	set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\avara.io")
	set(CPACK_NSIS_CONTACT "#avaraline@avaraline.net")
	set(CPACK_NSIS_MODIFY_PATH OFF)
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "Avara.exe")
else()
	set(CPACK_STRIP_FILES "Avara")
	set(CPACK_SOURCE_STRIP_FILES "")
endif()
set(CPACK_PACKAGE_EXECUTABLES "Avara" "Avara")
include(CPack)
