
# Avara

A port of the [1996 game by Juri Munkki](https://en.wikipedia.org/wiki/Avara), published by Ambrosia Software, [released under MIT license](https://github.com/jmunkki/Avara).

Avara is a cult-classic three-dimensional networked action FPS where you play as a remote-controlled bipedal tank. It was originally developed for Apple Macintosh with support all the way back to [System 6](https://en.wikipedia.org/wiki/System_6), using the [THINK C programming language](https://en.wikipedia.org/wiki/THINK_C). This port uses C++, SDL2, and OpenGL to replace code that relied on Macintosh system APIs, and is intended to run on Windows, macOS, and Linux.

## Media
* [YouTube: The first successful 3-player network test game of our Avara port](https://www.youtube.com/watch?v=aOW6lSC0kQI)
* [YouTube: Testing Grimoire (and UI preview)](https://www.youtube.com/watch?v=OX4I18PXXOI&)
* [YouTube: Demo of live level editing with Inkscape in forthcoming SVG support](https://www.youtube.com/watch?v=ArM6k6w1ItM)

## Build Requirements
### All platforms
[SDL2](https://www.libsdl.org/download-2.0.php) and [SDL_net](https://www.libsdl.org/projects/SDL_net/), POSIX-like environment, clang

### Windows 10
Building on Windows 10 requires the [MSYS2](https://www.msys2.org/) environment. After installing MSYS2, you can get the dependencies for building by running the following in an MSYS2 Mingw64 terminal: 

`pacman --noconfirm --needed -Syuu zip mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_net mingw-w64-x86_64-clang mingw-w64-x86_64-glbinding mingw-w64-x86_64-glew make`

### Linux
On Debian based distributions of Linux, you can install the needed dependencies for building by running the following in a terminal:

`apt-get -yq install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libegl1-mesa-dev libgles2-mesa-dev libsdl2-net-dev`

### Apple macOS
Avara can be built using [Xcode](https://developer.apple.com/xcode/features/) development tools or [Homebrew](https://brew.sh/). You must install SDL and ensure the libraries are available in `/Library/Frameworks`.

## Build Instructions

Running `make` will produce a binary at `build/Avara`.

This Makefile has been tested on recent versions of macOS, Windows 10, and various flavors of Linux. If you encounter a problem building, feel free to submit an issue, or ideally a pull request!

To create a signed app bundle on macOS, you can use `make macapp SIGNING_ID=yourid`.

To create a portable executable archive (library dlls included) on Windows 10, use `make winapp`.

There are many intermediate compiled files and you can drastically speed up Avara's build times by offering a number of jobs to run simultaneously to the `make` command. On a quad-core machine, try `make -j5`. Typically you can safely use a number of jobs that is one more than the number of cores on your machine.

## How to play
To get into a game and look around, just press the "Load Level" button, then the "Start Game" button in the "Levels" window. This will drop you into the level called Alektra. You're now in control of a lethal bipedal machine called a Hostile Environment Combat and Tactical Operations Remote (HECTOR), equipped with homing missiles, high-explosive contact grenades, and reactive plasma cannons! The controls shown below are the defaults. To change them, see the Configuring your Controls section.
* The movement controls are `WSAD`, with `A` and `D` rotating you left/right and `W` and `S` to move forward and backward respectively. The head and legs are controlled independently, reminiscent of MechWarrior. 
* Use your mouse to rotate your HECTOR's head. Note the `V` shape at the top of the screen, this indicates the direction of your legs (and the direction you will move when you press forward).
* You can click to fire your HECTOR's plasma cannons, press `E` to fire grenades. 
* You can load a missile with `Q` and then press the mouse button to fire it. 
* The meters on the bottom of the screen show your current energy levels: 
	* Blue 🛡️ is the energy shield level--when this is gone, your HECTOR is toast!💥
	* Green ⚡ is the reactor's energy reserve--this source recharges both the energy shield and the plasma cannons. When it empty, you are very vulnerable.
	* Red 🔫 shows your HECTOR's plasma cannon energy level, which directly corresponds to the damage an enemy will take when they are hit by the plasma if it is fired at that moment.
* Press `Left Shift` to use a booster 🚀. This puts your reactor into hyperdrive and quickly recharges energy shields and cannons. The standard Light recon HECTOR carries only three of these reactor boosters, so use them wisely!
* Weight 🏋️ has a large effect on your speed 💨--Try moving from one spot to another before and after firing all your ordnance!
* `Tab` will deploy your "Scout" camera drone 🚁, enabling an external view of your HECTOR. Press an arrow key once to set a homing position for the Scout. It will always try to move and stay in this spot relative to the HECTOR. The Scout is a physical object within the game, can get stuck on level geometry, can be destroyed, and alerts enemies to your position--so be careful!
* Press and hold `Space` to crouch, release it to jump. To get full jump height, you must hold space until you stop crouching. Jump wisely, as you are very slow when crouching, and airbourne movement is quite predictable.
* To quit playing, press `Escape` once to self-destruct, then again quickly to abort the game.

To join a network game use the "Network" window and enter the server's address. You should be able to copy an IP from somewhere else and paste it in. Then you can hit connect to join the server.

You can host a game by clicking the "Start Hosting" button. Other users can connect to you after you see the button change to "Stop Hosting". 

Press `Enter` during gameplay to start text input mode and type messages to other players at the bottom of the screen. You can press `Enter` again to exit this mode.

_Please note_: Before you attempt to play online, be sure to reference the section below about properly configuring your network.


## Configuring your controls
Start by running the game at least once and closing it, so that it can create a preferences file, `prefs.json`. This file will be overwritten on every close to save network connection preferences, so be sure not to make edits until after you've closed Avara. It is located in your user profile folder, you can use any text editor to edit it.

* Windows: `%UserProfile%\AppData\Roaming\Avaraline\Avara\`
* Mac: `~/Library/Application Support/Avaraline/Avara/`
* Linux: `~/.local/share/Avaraline/Avara/`

On Windows you can browse to the directory by copying the above, pressing `Win + R`, pasting, and pressing enter. On macos, you can press `Cmd + Shift + G` to open the `Go...` window, paste in the above, and press enter.

Within the preferences file, under the `keyboard` section, each left hand value is a game action that may be taken, with the right hand being the key that you would like for that action. The names of the keys are in the leftmost column of [this page describing SDL_Keycode](https://wiki.libsdl.org/SDL_Keycode), under the "Key Name" heading. Note that the modifier buttons are specific to the side of the keyboard they are on, for example, you can map `Left Alt` but not `Alt`.

This file allows you to set more than one button to a specific function by setting an action name to a quoted, comma separated list of values surrounded by `[ ]`:

    "jump": [
        "Space",
        "Left Alt"
    ]

See the `jump` key or the `fire` key for an example.

`mouseSensitivity` is a value between 0 (less sensitive) and 2 (more sensitive). SDL mouse data differs from platform to platform, so you may need to experiment to get the correct sensitivity value.

Make sure you test your settings before joining a networked game! Nothing is worse than not having a booster key because you misspelled "Ctrl".

## Configuring your Network 
Important: ⚠️ **You must forward port `19567`** to the Avara machine on whatever router you use to connect to the internet **even if you are only playing as a client** and do not plan on hosting a server. ⚠️

This is because of Avara's P2P networking architecture, in which other clients will connect directly to you, forwarding information in a round-robin fashion.

The default port number of `19567` can be changed in the `prefs.json` file, but this is not recommended as all clients must also update this value if they wish to connect.  

**One player per internet connection**. Because of the way Avara is set up to directly connect to single machines, routers will not be provided with enough information to forward (or NAT) the packets to the correct machine if there is more than one machine connected on the same local network. 

This means that if you have two machines using the same internet connection, which is typical in most home networks, two players using that network cannot connect to the same external internet server and play together. 

A typical solution to this problem is to have the second player connect via a VPN, separate ISP connection, or proxy/tunnel.

## Known issues
* Custom BSP shapes within levels are not yet supported. Loading a level with custom shapes will crash the game. You are safe loading any of the `aa-normal` collection levels which are shown by default.
* There is a bug in networked multiplayer where specific types of lag spikes cause the game to lose track of timing and [play old frames at an advanced rate](https://www.youtube.com/watch?v=zxm1zzFzor0). Thankfully this is somewhat rare during normal play. 
* [#26](https://github.com/avaraline/Avara/issues/26) Some levels have graphical "z-fighting" issues where geometry is too close to other geometry and our current graphics pipeline does not reconcile this correctly causing moire-like patterns. 
* [#25](https://github.com/avaraline/Avara/issues/25) Avara's usage of SDL does not play well with PulseAudio on some Linux systems. If you get a segfault when launching, try telling SDL to use ALSA instead with `export SDL_AUDIODRIVER=alsa` 


## Challenges overcome while porting
Porting this game has been a heroic effort, mostly pulled off in the [span of four or five insanely productive weeks](https://github.com/avaraline/Avara/commits?after=1163cea5b338c17e406986b2926c1cdc626dccaa+34&author=dcwatson) by @dcwatson. 
- Avara's graphics engine was completely custom and unique. It was able to fill and shade complex polygons in screen-space, which is unheard-of in any modern 3D graphics engine. This was achieved through clever use of Motorola 68k/PowerPC  assembly. Though porting the original renderer proved possible, it has been replaced with an OpenGL context to take advantage of hardware acceleration.
- Because of the nature of Avara's 3D engine, all existing BSP shape data was not compatible with OpenGL, which accepts only triangles (the term "BSP" is used loosely here, as the models are technically binary space partition trees, but they are in a format specialized to Avara's rendering engine and are unique from the commonly used BSP format popularized by Quake and other games later in the 90s). Scripts were written to decompose all complex polygons into triangles and save the models into a JSON format that can be read easily.
- The sound engine includes a Doppler/positional audio simulation which was extremely advanced for its time. The custom Huffman encoder and sampling algorithms have been successfully ported. You can thank @jonathan-voss for allowing us to experience the expressiveness of Avara's sounds in the new millenium!
- Avara had its own memory management system that made extensive use of old Macintosh APIs, this was fixed with backward-compatible shims to minimize code changes.
- The game timer originally used interrupts available on early Macintosh systems--it is now based off of threads and system timing functions available in the standard library.
- The networking subsystem required significant refactoring to make use of modern APIs (via [SDL_net](https://www.libsdl.org/projects/SDL_net/)) and the expectation of broadband speeds. The original was (successfully!) designed to make the game playable over 14.4k modem internet connections and some expectations about latency needed adjustment.
- The original Avara stored assets in a resource fork, which is a special type of format used in classic Macintosh OSs. These resources were easily parsed with the standard library on the Macintosh, we were able to replace these system calls in a backwards-compatible manner so that the port can use exactly the same assets that the original did. We are in the process of updating the formats to modern and easily used equivalents, and our backwards-compatibility allows us to unit test those updates.
- Many of the algorithms in Avara relied on bit shifting to do math operations, including on pointers, to save processor cycles. Type widths were adjusted, and most of these operations worked without any adjustment, though some were removed to make use of the C++ standard library functions.
- Various mathematical operations in the Avara source were written in 68k assembly only. These have been replaced with equivalent code in C++.
- Countless conversions of Pascal strings to string classes and back, replacing arrays and pointer math with modern data structures when necessary, correcting endianness.
- The entire interface to the original was created with the Macintosh Toolbox libraries, which all had to be ripped out and replaced. We are currently testing our options using [nanogui](https://github.com/wjakob/nanogui) for the pre-game interface and [nanovg](https://github.com/memononen/nanovg) for in-game information. 
- Avara's level format was based around a Macintosh binary picture drawing format called PICT, also known as QuickDraw. Currently the port still reads these PICT resources, but work is underway to use [Inkscape](https://inkscape.org/) SVG drawings instead, and to create a method of converting all the previously created community level sets to this new format.

Other than these specific items, the core of the game logic was immediately cross-platform. The code is designed very cleanly and Juri's wizardry, especially for having done it all in 1996, cannot be overstated. Avara is a joy to work on and play, so we hope you decide to check it out! Visit us at irc://avaraline.net:6667/avaraline to chat and maybe even ✓✓ play some games.
