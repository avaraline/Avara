cmake_minimum_required(VERSION 3.12)
project(Avara VERSION 0.7.6)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(VERBOSE True)
set(OpenGL_GL_PREFERENCE GLVND)

set(EXEC_NAME Avara)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "-MP -g -Wno-multichar -DNANOGUI_GLAD")

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
# glad
find_package(glad CONFIG REQUIRED)
# gtest
find_package(GTest CONFIG REQUIRED)
# sqlite3
find_package(SQLite3 REQUIRED)

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

if(NOT WIN32)
   find_package(OpenGL REQUIRED)
endif()


file(GLOB_RECURSE CORE_SOURCE RELATIVE ${CMAKE_SOURCE_DIR} 
    "src/assets/*.cpp"
    "src/audio/*.cpp" 
    "src/base/*.cpp" 
    "src/bsp/*.cpp" 
    "src/compat/*.cpp"
    "src/game/*.cpp"
    "src/gui/*.cpp" 
    "src/level/*.cpp"
    "src/net/*.cpp"
    "src/render/*.cpp"
    "src/tui/*.cpp"
    "src/util/*.cpp"
    "src/util/huffman/*.cpp"
    "vendor/glad/glad.c"
    "vendor/glm/detail/glm.cpp"
    "vendor/nanogui/*.cpp"
    "vendor/nanovg/nanovg.c"
    "vendor/pugixml/pugixml.cpp"
    "vendor/csscolorparser.cpp"
    "vendor/stb_vorbis.c"
)

file(GLOB_RECURSE ALL_RES_FILES 
    "rsrc/*"
    "levels/*"
)
list(FILTER ALL_RES_FILES EXCLUDE REGEX ".*\.r$")

if(WIN32 OR MSVC)
    set(CMAKE_CXX_FLAGS "/EHsc /std:c++17 /DNANOGUI_GLAD")
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/platform/windows/appicon.rc")
    list(APPEND EXTRA_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 shell32 opengl32 glu32 wsock32 ws2_32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
    add_compile_definitions(NOMINMAX)
endif()

add_library(AvaraCore OBJECT ${CORE_SOURCE})

set(CORE_INCLUDES
    "${SDL2_INCLUDE_DIR}"
    ${SDL2main_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR} 
    ${CMAKE_BINARY_DIR}
    src
    src/assets
    src/audio
    src/base
    src/bsp
    src/compat
    src/game
    src/gui
    src/tui
    src/level
    src/net
    src/render
    src/tui
    src/util
    src/util/huffman
    vendor 
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

add_executable(${EXEC_NAME}
    src/Avara.cpp
    $<TARGET_OBJECTS:AvaraCore>
)

if (APPLE)
    set(AssetLib ${CMAKE_CURRENT_SOURCE_DIR}/platform/macos/Assets.xcassets)
    target_sources(${EXEC_NAME} PUBLIC ${AssetLib})
    set_source_files_properties(${AssetLib} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    target_sources(${EXEC_NAME} PUBLIC ${ALL_RES_FILES})
    foreach(FILE ${ALL_RES_FILES})
        file(RELATIVE_PATH NEW_FILE "${CMAKE_CURRENT_SOURCE_DIR}" ${FILE})
        get_filename_component(NEW_FILE_PATH ${NEW_FILE} DIRECTORY)
        set_property(SOURCE ${FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${NEW_FILE_PATH}")
        source_group("Resources/${NEW_FILE_PATH}" FILES "${FILE}")
    endforeach()
    set_target_properties(${EXEC_NAME} PROPERTIES 
        MACOSX_BUNDLE TRUE
        BUILD_WITH_INSTALL_RPATH YES 
        XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME NO
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platform/macos/Info.plist"
        MACOSX_BUNDLE_GUI_IDENTIFIER com.avaraline.avara
        MACOSX_BUNDLE_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
        MACOSX_BUNDLE_ICON_FILE "AppIcon"
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
    )
endif()

add_executable(tests
    src/tests.cpp
    $<TARGET_OBJECTS:AvaraCore>
)

target_compile_options(${EXEC_NAME} PRIVATE ${AVARA_COMPILE_OPTIONS})
target_compile_options(tests PRIVATE ${AVARA_COMPILE_OPTIONS})

target_include_directories(AvaraCore PUBLIC ${CORE_INCLUDES})
target_include_directories(${EXEC_NAME} PUBLIC ${CORE_INCLUDES})
target_include_directories(tests PUBLIC ${CORE_INCLUDES} )

set(AVARA_LIBS SDL2::SDL2 SDL2::SDL2main ${OPENGL_LIBRARIES} ${SQLite3_LIBRARIES} dl pthread)

if(WIN32)
set(AVARA_LIBS SDL2::SDL2 SDL2::SDL2main glad::glad ${OPENGL_LIBRARIES} ${SQLite3_LIBRARIES})
add_custom_command(TARGET ${EXEC_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${EXEC_NAME}> $<TARGET_FILE_DIR:${EXEC_NAME}>
  COMMAND_EXPAND_LISTS
  )
endif()

target_link_libraries(${EXEC_NAME} ${AVARA_LIBS})

target_link_libraries(tests ${AVARA_LIBS} ${EXTRA_LIBS} GTest::gtest GTest::gtest_main)

#target_link_libraries(hsnd2wav ${AVARA_LIBS} ${EXTRA_LIBS})


if(WIN32)
    target_link_libraries(${EXEC_NAME} wsock32 ws2_32)
endif()

include(InstallRequiredSystemLibraries)


if(WIN32)
install(FILES $<TARGET_RUNTIME_DLLS:Avara> DESTINATION "${PROJECT_BINARY_DIR}")
endif()


set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Avara - a 3d networked multiplayer shooter")
set(CPACK_PACKAGE_VENDOR "Avaraline")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "7")
set(CPACK_PACKAGE_VERSION_PATCH "6")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Avara")
if(WIN32 AND NOT UNIX)
	set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\platform\\\\windows\\\\avara.ico")
	set(CPACK_NSIS_INSTALLED_ICON_NAME "Avara.exe")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\platform\\\\windows\\\\install.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\platform\\\\windows\\\\uninstall.ico")
    set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_CURRENT_SOURCE_DIR}\\\\platform\\\\windows\\\\installsplash.bmp")
    set(CPACK_NSIS_MUI_HEADERIMAGE "${CMAKE_CURRENT_SOURCE_DIR}\\\\platform\\\\windows\\\\installheader.bmp")
    set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
    set(CPACK_NSIS_BRANDING_TEXT " ")
	set(CPACK_NSIS_DISPLAY_NAME "Avara")
    set(CPACK_NSIS_PACKAGE_NAME "Avara")
	set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.github.com\\\\avaraline\\\\avara")
	set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\avara.gg")
	set(CPACK_NSIS_CONTACT "#avaraline@avaraline.net")
	set(CPACK_NSIS_MODIFY_PATH OFF)
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "Avara.exe")
else()
	set(CPACK_STRIP_FILES "Avara")
	# set(CPACK_SOURCE_STRIP_FILES "")
endif()
set(CPACK_PACKAGE_EXECUTABLES "Avara" "Avara")
include(CPack)
