const std = @import("std");
const sdl = @import("sdl");

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{});

    const target = b.standardTargetOptions(.{});

    const avaracflags = [_][]const u8{ "-std=c++17", "-MP", "-Wno-multichar", "-DNANOGUI_GLAD", "-fno-sanitize=undefined"};

    const avara = b.addExecutable(.{ 
        .name = "avara", 
        .target = target, 
        .optimize = optimize
    });

    avara.linkLibC();
    avara.linkSystemLibrary("c++");

    const sdl_dep = b.dependency("sdl", .{
        .optimize = .ReleaseFast,
        .target = target,
    });

    avara.linkLibrary(sdl_dep.artifact("SDL2"));
    avara.addIncludePath(b.path("src/game"));
    avara.addIncludePath(b.path("src/level"));
    avara.addIncludePath(b.path("src/base"));
    avara.addIncludePath(b.path("src/util"));
    avara.addIncludePath(b.path("src/util/huffman"));
    avara.addIncludePath(b.path("src/audio"));
    avara.addIncludePath(b.path("src/gui"));
    avara.addIncludePath(b.path("src/bsp"));
    avara.addIncludePath(b.path("src/net"));
    avara.addIncludePath(b.path("src/compat"));
    avara.addIncludePath(b.path("vendor"));
    avara.addIncludePath(b.path("vendor/glm"));
    avara.addIncludePath(b.path("vendor/nanogui"));
    avara.addIncludePath(b.path("vendor/nanovg"));
    avara.addIncludePath(b.path("vendor/pugixml"));
    avara.addIncludePath(b.path("vendor/glad"));
    avara.addIncludePath(b.path("vendor/miniupnpc"));
    avara.addIncludePath(b.path("vendor/utf8"));
    avara.addCSourceFiles(.{ .files = &[_][]const u8{ 
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
        "src/Avara.cpp" }, 
        .flags = &avaracflags });
    
    avara.addCSourceFiles(.{ .files = &[_][]const u8{
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
    }, .flags = &[_][]const u8{
        "-MP",
        "-Wno-multichar",
        "-DNANOGUI_GLAD",
    } });

    if (@import("builtin").target.os.tag == .macos) {
        avara.addCSourceFiles(.{.files = &[_][]const u8{ "vendor/nanogui/darwin.mm" },
        .flags = &avaracflags });
    }

    const install_rsrc = b.addInstallDirectory(.{
        .source_dir = b.path("rsrc"),
        .install_dir = .{ .custom = "" },
        .install_subdir = "bin/rsrc"
    });

    const install_levels = b.addInstallDirectory(.{
        .source_dir = b.path("levels"),
        .install_dir = .{ .custom = "" },
        .install_subdir = "bin/levels"
    });

    avara.step.dependOn(&install_rsrc.step);
    avara.step.dependOn(&install_levels.step);

    const run_cmd = b.addRunArtifact(avara);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    b.installArtifact(avara);
}
