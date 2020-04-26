cmake_minimum_required(VERSION 3.1)
project(Avara)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-DNANOGUI_GLAD")
if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()



# Find SDL2 and OpenGL
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

include(TargetCopyFiles)

set(SDL2_PATH "${CMAKE_SOURCE_DIR}/platform/windows/SDL2-2.0.12")
find_package(SDL2 REQUIRED COMPONENTS main)
set(SDL2_NET_PATH "${CMAKE_SOURCE_DIR}/platform/windows/SDL2_net-2.0.1")
find_package(SDL2_net REQUIRED)
if(NOT WIN32)
    find_package(OpenGL REQUIRED)
endif()

if(WIN32)
	list(APPEND EXTRA_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 shell32 opengl32 glu32 wsock32 ws2_32 comlg32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
endif()

add_executable(Avara 
src/audio/CBasicSound.cpp
src/audio/CRateSound.cpp
src/audio/CSoundHub.cpp
src/audio/CSoundMixer.cpp
src/audio/DopplerPlug.cpp
src/Avara.cpp
src/base/CBaseObject.cpp
src/base/CCompactTagBase.cpp
src/base/CDirectObject.cpp
src/base/CExitHandler.cpp
src/base/CTagBase.cpp
src/bsp/CBSPPart.cpp
src/bsp/CBSPWorld.cpp
src/bsp/CScaledBSP.cpp
src/bsp/CSmartBox.cpp
src/bsp/CSmartPart.cpp
src/bsp/CViewParameters.cpp
src/compat/Files.cpp
src/compat/Memory.cpp
src/compat/Resource.cpp
src/compat/System.cpp
src/game/CAbstractActor.cpp
src/game/CAbstractMissile.cpp
src/game/CAbstractMovers.cpp
src/game/CAbstractPlayer.cpp
src/game/CAbstractYon.cpp
src/game/CAreaActor.cpp
src/game/CAvaraApp.cpp
src/game/CAvaraGame.cpp
src/game/CBall.cpp
src/game/CDepot.cpp
src/game/CDome.cpp
src/game/CDoor2Actor.cpp
src/game/CDoorActor.cpp
src/game/CForceField.cpp
src/game/CFreeSolid.cpp
src/game/CGlowActors.cpp
src/game/CGoal.cpp
src/game/CGoody.cpp
src/game/CGrenade.cpp
src/game/CGroundColorAdjuster.cpp
src/game/CGuardActor.cpp
src/game/CHologramActor.cpp
src/game/CHUD.cpp
src/game/CIncarnator.cpp
src/game/CLevelViewerApp.cpp
src/game/CLogic.cpp
src/game/CLogicAnd.cpp
src/game/CLogicBase.cpp
src/game/CLogicCounter.cpp
src/game/CLogicDelay.cpp
src/game/CLogicDistributor.cpp
src/game/CLogicTimer.cpp
src/game/CMarkerActor.cpp
src/game/CMineActor.cpp
src/game/CMissile.cpp
src/game/CNetManager.cpp
src/game/CParasite.cpp
src/game/CPill.cpp
src/game/CPlacedActors.cpp
src/game/CPlayerActor.cpp
src/game/CPlayerManager.cpp
src/game/CPlayerMissile.cpp
src/game/CRamp.cpp
src/game/CRealMovers.cpp
src/game/CRealShooters.cpp
src/game/CScoreKeeper.cpp
src/game/CScout.cpp
src/game/CShuriken.cpp
src/game/CSkyColorAdjuster.cpp
src/game/CSliverPart.cpp
src/game/CSmart.cpp
src/game/CSolidActor.cpp
src/game/CSoundActor.cpp
src/game/CSphereActor.cpp
src/game/CSwitchActor.cpp
src/game/CTeleporter.cpp
src/game/CTextActor.cpp
src/game/CTriPyramidActor.cpp
src/game/CUfo.cpp
src/game/CWalkerActor.cpp
src/game/CWallActor.cpp
src/game/CWallDoor.cpp
src/game/CWallSolid.cpp
src/game/CWeapon.cpp
src/game/CWorldShader.cpp
src/game/CYonBox.cpp
src/game/CYonSphere.cpp
src/game/CZombieActor.cpp
src/game/LinkLoose.cpp
src/gui/CApplication.cpp
src/gui/CLevelWindow.cpp
src/gui/CNetworkWindow.cpp
src/gui/CPlayerWindow.cpp
src/gui/CRosterWindow.cpp
src/gui/CTrackerWindow.cpp
src/gui/CWindow.cpp
src/level/CLevelDescriptor.cpp
src/level/LevelLoader.cpp
src/level/Parser.cpp
src/level/PICTParser.cpp
src/net/AvaraTCP.cpp
src/net/CAbstractCommClient.cpp
src/net/CAbstractCommServer.cpp
src/net/CCommManager.cpp
src/net/CProtoControl.cpp
src/net/CUDPComm.cpp
src/net/CUDPConnection.cpp
src/util/AvaraGL.cpp
src/util/Beeper.cpp
src/util/CDataBuffer.cpp
src/util/CharWordLongPointer.cpp
src/util/CRC.cpp
src/util/CStringDictionary.cpp
src/util/FastMat.cpp
src/util/huffman/CAbstractHuffPipe.cpp
src/util/huffman/CAbstractPipe.cpp
src/util/huffman/CHandlePipe.cpp
src/util/huffman/CHuffDecode.cpp
src/util/huffman/CHuffEncode.cpp
src/util/huffman/CHuffmanHandler.cpp
src/util/huffman/CHuffProcessor.cpp
src/util/RamFiles.cpp
vendor/glad/glad.c
vendor/glm/detail/glm.cpp
vendor/nanogui/button.cpp
vendor/nanogui/checkbox.cpp
vendor/nanogui/colorcombobox.cpp
vendor/nanogui/combobox.cpp
vendor/nanogui/common.cpp
vendor/nanogui/desccombobox.cpp
vendor/nanogui/glcanvas.cpp
vendor/nanogui/glutil.cpp
vendor/nanogui/label.cpp
vendor/nanogui/layout.cpp
vendor/nanogui/messagedialog.cpp
vendor/nanogui/nanogui_resources.cpp
vendor/nanogui/popup.cpp
vendor/nanogui/popupbutton.cpp
vendor/nanogui/progressbar.cpp
vendor/nanogui/screen.cpp
vendor/nanogui/slider.cpp
vendor/nanogui/stackedwidget.cpp
vendor/nanogui/tabheader.cpp
vendor/nanogui/tabwidget.cpp
vendor/nanogui/text.cpp
vendor/nanogui/textbox.cpp
vendor/nanogui/theme.cpp
vendor/nanogui/vscrollpanel.cpp
vendor/nanogui/widget.cpp
vendor/nanogui/window.cpp
vendor/nanovg/nanovg.c
)

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
target_link_libraries(Avara SDL2::Main SDL2::Net ${OPENGL_LIBRARIES})

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

#add_custom_command(TARGET Avara POST_BUILD COMMAND
		#${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ $<TARGET_FILE_DIR:Avara>)

