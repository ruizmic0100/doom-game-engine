#!/bin/bash

# FIXME MinGW makes a whole new context when running this script 
# Which causes a mess.

set -e

# Detecting which OS the host is using.
detect_os() {
    uname -a

    echo "Linux Operating System Detected: $OS"
    echo "Installing OpenGL Libraries"
    sudo apt-get install freeglut3 freeglut3-dev libglew1.5 libglew1.5-dev libglu1-mesa libglu1-mesa-dev libgl1-mesa-glx libgl1-mesa-dev

    build_engine_linux

}

#TODO add dependency injection function for winddows and linux.

build_engine_linux() {
        g++ -Wall gl-game-engine.c -lGL -lGLU -lglut -o app.o
        run_engine_linux
}

run_engine_linux() {
    ./app.o
}

main() {
    detect_os
}

main