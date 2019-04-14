# Avara

A port of the 1996 game by Juri Munkki, published by Ambrosia Software, using SDL2 and OpenGL.

Avara is a three-dimensional action first-person shooter where you play as a remote-controlled bipedal tank. It was developed for Classic Macintoshes, even running on systems with no color, with support all the way back to System 6, using the THINK-C programming language and compilers published by Symantec. THINK-C is a "C with objects" precursor to C++. The port attempts to leave as much of the original code intact as possible, while allowing for updates and changes that enable compilation on modern systems, and user quality of life improvements that gaming enthusiasts have come to expect from modern shooters. The project targets C++11 with LLVM. The major challenges of porting included:

- Avara's graphics engine was completely custom and unique. It was able to fill and shade complex polygons in screen-space, which is unheard-of in pretty much any modern graphics engine. This was acheived through heavy use of Motorola 68k assembly code. The original renderer has been replaced with an OpenGL context.
- Because of the nature of Avara's 3d engine, all existing BSP shape data was not compatible with OpenGL, which expects only triangles. Scripts were written to convert the shape data into a modern BSP/JSON format and to decompose all complex polygons into triangles.
- The sound engine includes a doppler/positional audio simulation which was extremely advanced for its time. The custom huffman encoder and sampling algorithms have been sucessfully ported to C++.
- Memory handling was done internally and made extensive use of Macintosh syscalls, this fixed with backward-compatible shims to modern C++ memory management.
- The game timer was originally handled with interrupts on early Macs--it is now cross-platform and based off of system time and callbacks
- The original game stored assets in a resource fork, which is a special type of format used in classic Macintosh OSs. These resources were easily parsed with syscalls on the Macintosh, we were able to replace these syscalls in a backwards-compatible manner so that the port can use exactly the same assets that the original did.
- Many of the algorithms in Avara relied on bit shifting to do math operations (this saved processor cycles on early Macintosh hardware), even on pointer types. Type widths were adjusted to ensure that these operations still work.
- Various mathematical operations in the Avara source were written in 68k assembly only. These have been replaced with equivalent code in C++.
- Countless conversion of Pascal strings to string classes and back, replacing arrays and pointer math with modern data structures when necessary, correcting endianness.

Original source released under the MIT license here:

https://github.com/jmunkki/Avara


## Requirements

# All platforms

[SDL2](https://www.libsdl.org/download-2.0.php) and [SDL_net](https://www.libsdl.org/projects/SDL_net/)

# Windows

Avara targets Windows 10 and up. This is [partly because](https://github.com/avaraline/Avara/issues/30) we target OpenGL version 3.3, which some graphics cards and drivers on Windows 7 do not support. Building on Windows 10 requires the [MSYS2](https://www.msys2.org/) environment.

After installing MSYS2, you can get the dependencies for building by running the following in an MSYS2 Mingw64 terminal: `pacman --noconfirm --needed -Syuu zip mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_net mingw-w64-x86_64-clang mingw-w64-x86_64-glbinding mingw-w64-x86_64-glew make`

# Linux

On Debian based distributions of Linux, you can install the needed dependencies for building by running `apt-get -yq install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libegl1-mesa-dev libgles2-mesa-dev libsdl2-net-dev`

## Build Instructions

`make`

`build/Avara`

This port has been tested on recent versions of MacOS, Windows 10, and various flavors of Linux. If you encounter a problem building, feel free to submit an issue, or ideally a pull request!

To create a signed app bundle on macos, you can use `make macapp SIGNING_ID=yourid`.

To create a portable executable archive (library dlls included) on Windows, use `make winapp`.

## Media

* [YouTube: The first successful 3-player network test game of our Avara port](https://www.youtube.com/watch?v=aOW6lSC0kQI)
* [YouTube: Testing Grimoire (and UI preview)](https://www.youtube.com/watch?v=OX4I18PXXOI&)
* [YouTube: Demo of live level editing with Inkscape in forthcoming SVG support](https://www.youtube.com/watch?v=ArM6k6w1ItM)

## How to play

To get into a game and look around immediately, just press the "Load Level" button, then the "Start Game" button in the "Levels" window. This will drop you into the level called Alektra. 
* The movement controls are `WSAD`, with `A` and `D` rotating you left/right and `W` and `S` to move forward and backward.
* Use your mouse to rotate your HECTOR's head. Note the arrow at the top of the screen, this indicates the direction of your legs. 
* You can click to fire plasma, press `E` to fire grenades. 
* You can load a missile with `Q` and then press the mouse button to fire it. 
* Press `Left Shift` to use a booster and refill your shields/plasma energy levels.
* Weight has a large effect on your speed--Try moving from one spot to another before and after firing all your ordnance
* `Tab` will deploy your Scout camera, enabling an external view of your HECTOR. Use the arrow keys to position the Scout.
* Press and hold `Space` to crouch, release it to jump.
* To quit playing, press Escape once to self-destruct, then again to abort the game.

These keyboard controls can be configured, see below.

To join a network game use the "Network" window and replace the placeholder `avara.jp` with the IP address of your server. You should be able to copy an IP from somewhere else and paste it in. Then you can hit connect to join the server.

Otherwise you can host a game by simply clicking the "Start Hosting" button. Other users can connect to you after you see the button change to "Stop Hosting". 

_Please note_: Avara is an old game, and that means there are special considerations to playing over the internet as either a client or server, see Configuring your Network below.


## Configuring your controls

Start by running the game at least once so it can create a preferences file, `prefs.json`. This file will save on every close, so be sure not to make edits until after you've closed Avara. It is located in your user profile folder, you can use any text editor to edit it.

* Windows: `%UserProfile%\AppData\Roaming\Avaraline\Avara\`
* Mac: `~/Library/Application Support/Avaraline/Avara/`
* Linux: `~/.local/share/Avaraline/Avara/`

On Windows you can browse to the directory by copying the above, pressing `Win + R`, pasting, and pressing enter. On macos, you can press `Cmd + Shift + G` to open the `Go...` window, paste in the above, and press enter.

Within the preferences file, under the `keyboard` dictionary, each key is a game action that may be taken, with the value being the key that you would like for that action. The names of the keys are in the leftmost column of [this page describing SDL_Keycode](https://wiki.libsdl.org/SDL_Keycode), under the "Key Name" heading. Note that the modifier buttons are specific to the side of the keyboard they are on, for example, you can map `Left Alt` but not `Alt`.

This file allows you to set more than one button to a specific function within the JSON by simply setting an action name to an array of keys:

    "jump": [
        "Space",
        "Left Alt"
    ]

See the `jump` key or the `fire` key for an example.

`mouseSensitivity` is a value between 0 and 2. SDL operates a little differently from platform to platform, so you may need to experiment to get the correct sensitivity value.

Make sure you test your settings before rejoining a network game in progress! Nothing sucks worse than not having a booster key because you misspelled "Ctrl"

## Configuring your Network

Avara uses port `19567`, via UDP. The port number can be changed in the configuration file, but this is not recommended as all clients must also update this value if they wish to connect.  **You must forward port `19567`** to the Avara machine on whatever router you use to connect to the internet **even if you are only playing as a client** and do not plan on hosting a server. This is because of Avara's P2P networking architecture, in which other clients will connect directly to you, forwarding information in a round-robin fashion.

**One player per internet connection**: Because of the way Avara is set up to directly connect to single machines, routers will not be provided with enough information to forward (or NAT) the packets to the correct machine if there is more than one machine connected on the same local network. This means that if you have two machines using the same internet connection, which is typical in most home networks, two players on that network cannot connect to the same (external) internet server and play together. A typical solution to this problem is to have the second player connect via a VPN or separate ISP connection.

## Known issues

Being a mostly direct port of a game from more than twenty years ago, there are some limitations.

* The server tracker interface is still under development (nonfunctional).
* Custom BSP shapes within levels are not yet supported. Loading a level with custom shapes will crash the game. You are safe loading any of the aa-normal collection levels which are shown by default.
* There is a bug in networked multiplayer where specific types of lag spikes cause the game to lose track of timing and [play old frames at an advanced rate](https://www.youtube.com/watch?v=zxm1zzFzor0). Thankfully this is somewhat rare during normal play. 
* [#4](https://github.com/avaraline/Avara/issues/4) The horizon/sky shader does not account correctly for head rotation. This can be disorienting in open levels without walls. 
* [#7](https://github.com/avaraline/Avara/issues/7) Level-specific lighting is currently unsupported, which can make some color choices not look so great. 
* [#26](https://github.com/avaraline/Avara/issues/26) Some levels have graphical "z-fighting" issues where geometry is too close to other geometry and our current graphics pipeline does not reconcile this correctly causing moire-like patterns. 
* [#25](https://github.com/avaraline/Avara/issues/25) Avara's usage of SDL does not play well with PulseAudio. If you get a segfault when launching on Linux, tell SDL to use ALSA instead: `export SDL_AUDIODRIVER=alsa` 
