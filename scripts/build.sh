#!/bin/bash

echo "Removing old artifacts..."
rm app.o && rm app.exe

echo "Building new binaries..."
gcc -g -Wall gl-game-engine.c -lglu32 -lglut32 -lopengl32 -o app.o
gcc -g -Wall gl-game-engine.c -lglu32 -lglut32 -lopengl32 -o app.exe

echo "Finished!"