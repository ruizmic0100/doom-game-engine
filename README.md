# gl-game-engine v1.0.0
Is an OpenGL game engine aiming to be similar to how the Doom(1993) engine behaved.

## How To Run Release Demo
If you have a release demo file such as a demo.exe go ahead and proceed to run that like you normally would any other game.

## Dependencies On Windows For Development
- GCC Compiler (MinGW, Cygwin, etc.)
    > Used for compiling the source code.
- WSL2 or Bash
    > This is necessary so that windows can access linux commands.
- Libraries (opengl dev packages)
    - https://chortle.ccsu.edu/bloodshed/howtogl.html

## Dependencies On Linux For Development
- GCC Compiler
    > Used for compiling the source code.
    MinGW, Cygwin, etc.
- Libraries (opengl dev packages)
    > Needed for opengl api calls.
    - mesa-utils
    - libglu1-mesa-dev
    - freeglu3-dev
    - mesa-common-dev

## Supported Platforms
- Windows
- Linux

## Quick Start (Windows)
Run `.\scripts\quick-start` to setup the host and start the application. 

## Quick Start (Linux)
Run `./scripts/quick-start` to setup the host and start the application. 

## DEMO

![Alt Text](https://github.com/ruizmic0100/game-engine/tree/main/public/doom-opengl-engine-demo.gif "Engine Demo")

## Controls

[Movement]
- [w] - fowards
- [s] - backwards
- [d] - look right
- [a] - look left
- [<] - strafe left
- [>] - strafe right
- [m] + [w] - translate up
- [m] + [s] - translate down
- [m] + [d] - look down
- [m] + [a] - look up

## Useful Links
https://www.khronos.org/opengl/wiki/Getting_Started
