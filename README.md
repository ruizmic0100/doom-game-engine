# gl-game-engine v1.0.0
Is a OpenGL C game engine/framework akin to the original Doom engine.

## Dependencies
- MinGW
    > Used for compiling the source code.
- WSL2 or Bash
    > This is necessary so that windows can access linux commands.

## Supported Platforms
- Windows
- Linux

## Quick Start (Windows)
Run `.\scripts\quick-start` to setup the host and start the application. 

## Quick Start (Linux)
Run `wsl ./scripts/quick-start` to setup the host and start the application. 

## Useful Links
https://www.khronos.org/opengl/wiki/Getting_Started

## Notes
Linker flags necessary for compilation.
> -lglu32 -lglut32 -lopengl32