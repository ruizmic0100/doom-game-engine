#!/bin/bash

# FIXME MinGW makes a whole new context when running this script 
# Which causes a mess.


set -e

# Detecting which OS the host is using.
detect_os() {
    uname -a

    # Try using windows or linux command.
    OS=$(systeminfo | findstr /B /C:"OS Name") || OS=`wsl uname`
    uname -a
    #FIX Clear the erros considering >2/dev/null doesn't work...

    if [[ $OS=='Linux' ]]; then
        echo "Linux Operating System Detected: $OS"
        build_engine_linux
    else
        echo "Windows Operating System Detected: $OS"
        build_engine_windows
    fi

}

#TODO add dependency injection function for winddows and linux.

build_engine_linux() {
        gcc -g -Wall ./gl-game-engine.c -lglu32 -lglut32 -lopengl32 -o app.o
        run_engine_linux
}

build_engine_windows() {
        gcc -g -Wall .\gl-game-engine.c -lglu32 -lglut32 -lopengl32 -o app.exe
        run_engine_windows
}

run_engine_linux() {
    ./app.o
}

run_engine_windows() {
    ./app.exe
}

main() {
    detect_os
}

main