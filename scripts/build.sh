#!/bin/bash

echo "Removing old artifacts..."
rm app.o

echo "Building new binaries..."
g++ -Wall ./src/gl-game-engine.c ./src/doom_level.c -lGL -lGLU -lglut -o app.o

echo "Finished!"