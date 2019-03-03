# Avara

A port of the 1996 game by Juri Munkki, published by Ambrosia Software, using SDL2 and OpenGL.

Originally released under the MIT license here:

https://github.com/jmunkki/Avara


## Requirements

[SDL2](https://www.libsdl.org/download-2.0.php) and [SDL_net](https://www.libsdl.org/projects/SDL_net/)

On Debian based distributions of Linux, you can install the needed dependencies for building by running `apt-get -yq install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libegl1-mesa-dev libgles2-mesa-dev libsdl2-net-dev`

## Build Instructions

`make`

`build/Avara`

This port has been tested on recent versions of MacOS, Windows, and various flavors of Linux. If you encounter a
problem building, feel free to submit an issue, or ideally a pull request!

Note that to build on Windows, you must install [MSYS2](https://www.msys2.org/). You can then get the dependencies for building by running the following in an MSYS2 Mingw64 terminal: `pacman --noconfirm --needed -Syuu zip mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_net mingw-w64-x86_64-clang mingw-w64-x86_64-glbinding mingw-w64-x86_64-glew make`

To create a signed app bundle on macos, you can use `make macapp SIGNING_ID=yourid`.

To create a portable executable archive (library dlls included) on Windows, use `make winapp`.

## Media

* [YouTube: The first successful 3-player network test game of our Avara port](https://www.youtube.com/watch?v=aOW6lSC0kQI)
* [YouTube: Testing Grimoire (and UI preview)](https://www.youtube.com/watch?v=OX4I18PXXOI&)
* [YouTube: Demo of live level editing with Inkscape in forthcoming SVG support](https://www.youtube.com/watch?v=ArM6k6w1ItM)

## How to play

To get into a game and look around immediately, just press the "Load Level" button, then the "Start Game" button in the "Levels" window. This will drop you into the level called Alektra. 
* The movement controls are `WSAD`, with `S` and `D` rotating you right and left.
* Use your mouse to rotate your HECTOR's head. Note the arrow at the top of the screen, this indicates the direction of your legs. 
* You can click to fire plasma, press `E` to fire grenades. 
* You can load a missile with `Q` and then press the mouse button to fire it. 
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
