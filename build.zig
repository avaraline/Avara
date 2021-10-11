const std = @import("std");
const stdbuild = @import("build");
const sdk = @import("sdk.zig");

pub fn build(b: *std.build.Builder) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const avara = b.addExecutable("avara", null);
    avara.setTarget(target);
    avara.setBuildMode(mode);
    avara.linkLibC();
    avara.linkSystemLibrary("c++");
    avara.force_pic = true;
    avara.addIncludeDir("src/game");
    avara.addIncludeDir("src/level");
    avara.addIncludeDir("src/base");
    avara.addIncludeDir("src/util");
    avara.addIncludeDir("src/util/huffman");
    avara.addIncludeDir("src/audio");
    avara.addIncludeDir("src/gui");
    avara.addIncludeDir("src/bsp");
    avara.addIncludeDir("src/net");
    avara.addIncludeDir("src/compat");
    avara.addIncludeDir("vendor");
    avara.addIncludeDir("vendor/glm");
    avara.addIncludeDir("vendor/nanogui");
    avara.addIncludeDir("vendor/nanovg");
    avara.addIncludeDir("vendor/pugixml");
    avara.addIncludeDir("vendor/glad");
    avara.addIncludeDir("vendor/miniupnpc");
    avara.addIncludeDir("vendor/utf8");
    avara.addCSourceFiles(&.{
        "src/game/CGoody.cpp",
        "src/game/CLogicAnd.cpp",
        "src/game/CGlowActors.cpp",
        "src/game/CParasite.cpp",
        "src/game/CYonBox.cpp",
        "src/game/CPlayerActor.cpp",
        "src/game/CAreaActor.cpp",
        "src/game/CWalkerActor.cpp",
        "src/game/CAbstractActor.cpp",
        "src/game/CSwitchActor.cpp",
        "src/game/CPlayerMissile.cpp",
        "src/game/CHologramActor.cpp",
        "src/game/CBall.cpp",
        "src/game/CWeapon.cpp",
        "src/game/CAbstractPlayer.cpp",
        "src/game/CGrenade.cpp",
        "src/game/CTeleporter.cpp",
        "src/game/CRamp.cpp",
        "src/game/CShuriken.cpp",
        "src/game/CYonSphere.cpp",
        "src/game/CNetManager.cpp",
        "src/game/CPlayerManager.cpp",
        "src/game/CMarkerActor.cpp",
        "src/game/CFreeSolid.cpp",
        "src/game/CWallDoor.cpp",
        "src/game/CRealMovers.cpp",
        "src/game/CTextActor.cpp",
        "src/game/CLogicTimer.cpp",
        "src/game/CSmart.cpp",
        "src/game/CAvaraGame.cpp",
        "src/game/CDepot.cpp",
        "src/game/CMineActor.cpp",
        "src/game/CSoundActor.cpp",
        "src/game/CWorldShader.cpp",
        "src/game/CZombieActor.cpp",
        "src/game/CIncarnator.cpp",
        "src/game/CLogicDistributor.cpp",
        "src/game/CSkyColorAdjuster.cpp",
        "src/game/CScout.cpp",
        "src/game/CTriPyramidActor.cpp",
        "src/game/CWallActor.cpp",
        "src/game/CGroundColorAdjuster.cpp",
        "src/game/CSphereActor.cpp",
        "src/game/CWallSolid.cpp",
        "src/game/CLogicBase.cpp",
        "src/game/CLogicCounter.cpp",
        "src/game/CHUD.cpp",
        "src/game/CDoor2Actor.cpp",
        "src/game/CAbstractYon.cpp",
        "src/game/CScoreKeeper.cpp",
        "src/game/LinkLoose.cpp",
        "src/game/CAbstractMovers.cpp",
        "src/game/CRealShooters.cpp",
        "src/game/CAbstractMissile.cpp",
        "src/game/CPlacedActors.cpp",
        "src/game/CLogic.cpp",
        "src/game/CMissile.cpp",
        "src/game/CPill.cpp",
        "src/game/CUfo.cpp",
        "src/game/CDome.cpp",
        "src/game/CSolidActor.cpp",
        "src/game/CLogicDelay.cpp",
        "src/game/CGoal.cpp",
        "src/game/CAvaraApp.cpp",
        "src/game/CGuardActor.cpp",
        "src/game/CSliverPart.cpp",
        "src/game/CDoorActor.cpp",
        "src/game/CForceField.cpp",
        "src/level/Parser.cpp",
        "src/level/LevelLoader.cpp",
        "src/base/CTagBase.cpp",
        "src/base/CBaseObject.cpp",
        "src/base/CDirectObject.cpp",
        "src/audio/CBasicSound.cpp",
        "src/audio/DopplerPlug.cpp",
        "src/audio/CSoundMixer.cpp",
        "src/audio/CRateSound.cpp",
        "src/audio/CSoundHub.cpp",
        "src/util/CStringDictionary.cpp",
        "src/util/AvaraGL.cpp",
        "src/util/CRC.cpp",
        "src/util/RamFiles.cpp",
        "src/util/Beeper.cpp",
        "src/util/FastMat.cpp",
        "src/util/huffman/CHuffProcessor.cpp",
        "src/util/huffman/CAbstractHuffPipe.cpp",
        "src/util/huffman/CAbstractPipe.cpp",
        "src/util/huffman/CHandlePipe.cpp",
        "src/net/CUDPComm.cpp",
        "src/net/CUDPConnection.cpp",
        "src/net/CProtoControl.cpp",
        "src/net/CCommManager.cpp",
        "src/net/AvaraTCP.cpp",
        "src/gui/CWindow.cpp",
        "src/gui/CColorManager.cpp",
        "src/gui/CServerWindow.cpp",
        "src/gui/CApplication.cpp",
        "src/gui/CRosterWindow.cpp",
        "src/gui/CNetworkWindow.cpp",
        "src/gui/CPlayerWindow.cpp",
        "src/gui/CTrackerWindow.cpp",
        "src/gui/CLevelWindow.cpp",
        "src/bsp/CSmartPart.cpp",
        "src/bsp/CBSPWorld.cpp",
        "src/bsp/CBSPPart.cpp",
        "src/bsp/CSmartBox.cpp",
        "src/bsp/CViewParameters.cpp",
        "src/bsp/CScaledBSP.cpp",
        "src/compat/System.cpp",
        "src/compat/Resource.cpp",
        "src/compat/Memory.cpp",
        "src/compat/Files.cpp",
        "vendor/csscolorparser.cpp",
        "vendor/pugixml/pugixml.cpp",
        "vendor/nanogui/progressbar.cpp",
        "vendor/nanogui/slider.cpp",
        "vendor/nanogui/tabwidget.cpp",
        "vendor/nanogui/desccombobox.cpp",
        "vendor/nanogui/colorcombobox.cpp",
        "vendor/nanogui/vscrollpanel.cpp",
        "vendor/nanogui/label.cpp",
        "vendor/nanogui/nanogui_resources.cpp",
        "vendor/nanogui/messagedialog.cpp",
        "vendor/nanogui/checkbox.cpp",
        "vendor/nanogui/popupbutton.cpp",
        "vendor/nanogui/combobox.cpp",
        "vendor/nanogui/theme.cpp",
        "vendor/nanogui/glutil.cpp",
        "vendor/nanogui/stackedwidget.cpp",
        "vendor/nanogui/text.cpp",
        "vendor/nanogui/screen.cpp",
        "vendor/nanogui/glcanvas.cpp",
        "vendor/nanogui/widget.cpp",
        "vendor/nanogui/common.cpp",
        "vendor/nanogui/window.cpp",
        "vendor/nanogui/layout.cpp",
        "vendor/nanogui/popup.cpp",
        "vendor/nanogui/tabheader.cpp",
        "vendor/nanogui/button.cpp",
        "vendor/nanogui/textbox.cpp",
        "vendor/glm/detail/glm.cpp",
        "src/Avara.cpp",
    }, &.{
        "-std=c++17",
        "-MP",
        "-Wno-multichar",
        "-DNANOGUI_GLAD",
        "-fno-sanitize=undefined"
    });

    avara.addCSourceFiles(&.{
        "vendor/nanovg/nanovg.c",
        "vendor/glad/glad.c",
        "vendor/miniupnpc/minixml.c",
        "vendor/miniupnpc/upnpcommands.c",
        "vendor/miniupnpc/receivedata.c",
        "vendor/miniupnpc/miniwget.c",
        "vendor/miniupnpc/miniupnpc.c",
        "vendor/miniupnpc/upnpdev.c",
        "vendor/miniupnpc/upnpreplyparse.c",
        "vendor/miniupnpc/upnperrors.c",
        "vendor/miniupnpc/minissdpc.c",
        "vendor/miniupnpc/connecthostport.c",
        "vendor/miniupnpc/minisoap.c",
        "vendor/miniupnpc/igd_desc_parse.c",
        "vendor/miniupnpc/portlistingparse.c",
        "vendor/miniupnpc/addr_is_reserved.c",
        "vendor/stb_vorbis.c",
    }, &.{
        "-MP",
        "-Wno-multichar",
        "-DNANOGUI_GLAD",
    });

    avara.install();

    b.installDirectory(std.build.InstallDirectoryOptions{
        .source_dir = "rsrc",
        .install_dir = std.build.InstallDir{.bin = {}},
        .install_subdir = "rsrc",
    });
    b.installDirectory(std.build.InstallDirectoryOptions{
        .source_dir = "levels",
        .install_dir = std.build.InstallDir{.bin = {}},
        .install_subdir = "levels",
    });

    const sdl2loader = sdk.init(b);
    sdl2loader.link(avara, .dynamic);

    const run_cmd = avara.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
