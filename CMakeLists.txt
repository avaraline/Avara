cmake_minimum_required(VERSION 3.1)
project(Avara)


set(CMAKE_CXX_STANDARD 17)
set(OpenGL_GL_PREFERENCE GLVND)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-DNANOGUI_GLAD")
if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()



# Find SDL2 and OpenGL
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
    list(APPEND EXTRA_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 shell32 opengl32 glu32 wsock32 ws2_32 comlg32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
endif()

file(GLOB_RECURSE AUDIO RELATIVE ${CMAKE_SOURCE_DIR} "src/audio/*.cpp")
file(GLOB_RECURSE BASE RELATIVE ${CMAKE_SOURCE_DIR} "src/base/*.cpp")
file(GLOB_RECURSE BSP RELATIVE ${CMAKE_SOURCE_DIR} "src/bsp/*.cpp")
file(GLOB_RECURSE COMPAT RELATIVE ${CMAKE_SOURCE_DIR} "src/compat/*.cpp")
file(GLOB_RECURSE GAME RELATIVE ${CMAKE_SOURCE_DIR} "src/game/*.cpp")
file(GLOB_RECURSE GUI RELATIVE ${CMAKE_SOURCE_DIR} "src/gui/*.cpp")
file(GLOB_RECURSE LEVEL RELATIVE ${CMAKE_SOURCE_DIR} "src/level/*.cpp")
file(GLOB_RECURSE NET RELATIVE ${CMAKE_SOURCE_DIR} "src/net/*.cpp")
file(GLOB_RECURSE UTIL RELATIVE ${CMAKE_SOURCE_DIR} "src/util/*.cpp")
file(GLOB_RECURSE NANOGUI RELATIVE ${CMAKE_SOURCE_DIR} "vendor/nanogui/*.cpp")

add_executable(Avara 
src/Avara.cpp
${AUDIO}
${BASE}
${BSP}
${COMPAT}
${GAME}
${GUI}
${LEVEL}
${NET}
${UTIL}
vendor/glad/glad.c
vendor/glm/detail/glm.cpp
${NANOGUI}
vendor/nanovg/nanovg.c
)

target_compile_options(Avara PRIVATE -lstdc++ -lm -lpthread -ldl)

# Define executable target
target_include_directories(Avara PUBLIC
    ${SDL2_INCLUDE_DIRS} 
    ${SDL2main_INCLUDE_DIRS} 
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
    vendor/gtest
    vendor/gtest/include
    vendor/nanogui
    vendor/nanovg
    vendor/utf8
)
target_link_libraries(Avara SDL2::Main SDL2::Net ${OPENGL_LIBRARIES} dl pthread)

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


