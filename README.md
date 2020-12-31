
# Avara

A port of the [1996 game by Juri Munkki](https://en.wikipedia.org/wiki/Avara), published by Ambrosia Software, [released under MIT license](https://github.com/jmunkki/Avara).

Avara is a cult-classic three-dimensional networked action FPS where you play as a remote-controlled bipedal tank. It was originally developed for Apple Macintosh with support all the way back to [System 6](https://en.wikipedia.org/wiki/System_6), using the [THINK C programming language](https://en.wikipedia.org/wiki/THINK_C). This port uses C++, SDL2, and OpenGL to replace code that relied on Macintosh system APIs, and is intended to run on Windows, macOS, and Linux.

## Media
* [YouTube: The first successful 3-player network test game of our Avara port](https://www.youtube.com/watch?v=aOW6lSC0kQI)
* [YouTube: Testing Grimoire (and UI preview)](https://www.youtube.com/watch?v=OX4I18PXXOI&)
* [YouTube: Demo of live level editing with Inkscape in forthcoming SVG support](https://www.youtube.com/watch?v=ArM6k6w1ItM)

## Play
[Download here!](https://github.com/avaraline/Avara/releases/tag/0.7.0-nightly.20201230) These are binaries built automatically by github after every update we make. Try this out first! 

These have been tested on Windows 10 and macOS Catalina (10.15). If you're running an older macOS, you will probably need to follow the build instructions below instead.

### Tracker
We now have a game tracker running full time! [Check here to see if any servers are up, without launching the game](http://avara.io/)! There's also a JSON feed available [here](http://avara.io/api/v1/games/)!

### Resources 
- **[How to play](https://github.com/avaraline/Avara/wiki/How-to-Play)**
- **[Configuring your Controls](https://github.com/avaraline/Avara/wiki/Configuring-your-Controls)**
- **[Configuring your Network](https://github.com/avaraline/Avara/wiki/Configuring-your-Network)** - important! Read this if you have trouble connecting online.
- [Known issues](https://github.com/avaraline/Avara/issues?q=is%3Aissue+is%3Aopen+label%3Abug)
- [Read more about the porting process](https://github.com/avaraline/Avara/wiki/Porting-challenges) and how specific [obstacles](https://en.wikipedia.org/wiki/Macintosh_Toolbox) were overcome
- IRC: `irc://avaraline.net:6667/avaraline`
- Matrix: [Avaraline](https://matrix.to/#/!aIYRNfvpKSzEIwbtqn:vastan.net?via=vastan.net&via=matrix.org)

## Build Instructions
### All platforms
The project currently requires a POSIX-like environment (which means MSYS2 on Windows), with GLEW, [SDL2](https://www.libsdl.org/download-2.0.php), [SDL2_net](https://www.libsdl.org/projects/SDL_net/), clang, git, rsync, and make. The Makefile is multiplatform, and produces a binary at `build-<branch name>/Avara`. You should be able to change directory into the Avara repo and `make -j`.

### Windows 10
Before you do this, try the [pre-built binaries](https://github.com/avaraline/Avara/releases/tag/0.7.0-nightly.20201230). These should work for recent versions of Windows 10. 

Avara does not yet support MSVC++, so the MSYS2 Mingw environment must be used.
1. Install the [MSYS2](https://www.msys2.org/) environment. 
2. From the Start menu, open "MSYS2 Mingw64 terminal".
3. Update MSYS2: `pacman -Syu` As part of this process, you will need to close the terminal window, and you may need to run the command again.
4. Once MSYS2 is up to date, run the following: `pacman --noconfirm --needed -Syuu mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_net mingw-w64-x86_64-clang mingw-w64-x86_64-glbinding mingw-w64-x86_64-glew make git zip rsync`
5. You should now be able to clone the repository: `git clone https://github.com/avaraline/Avara.git`
6. Enter the repository: `cd Avara`
7. Build: `make -j`
8. You can now start the game with: `build-master/Avara.exe`
9. (optional) Make a portable exe: `make winapp`

### Linux
On Debian based distributions of Linux, you can install the needed dependencies for building by running the following in a terminal:

`apt-get -yq install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libegl1-mesa-dev libgles2-mesa-dev libsdl2-net-dev clang libdirectfb-dev git make rsync`

`libdirectfb-dev` is an optional dependency for running headless tests.

`cd /path/to/avara/repo && make -j && build-master/Avara`

### Apple macOS
Avara can be built using [Xcode](https://developer.apple.com/xcode/features/) development tools and [Homebrew](https://brew.sh/).

You must also download SDL2.framework and SDL2_net.framework. To get these, go to the [SDL2 download page](https://www.libsdl.org/download-2.0.php) and the [SDL2_net download page](https://www.libsdl.org/projects/SDL_net/), and click the links for Macintosh under Developement Libraries. [Copy the files](https://github.com/avaraline/Avara/blob/master/.github/workflows/avara-ci.yml#L60-L65) into either `/Library/Frameworks` or `~/Library/Frameworks`.

After that, you should be able to build: `cd /path/to/avara/repo && make -j`. Use `make macapp` to create an app bundle. You can provide a `SIGNING_ID` environment variable to create a signed app bundle.

