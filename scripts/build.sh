#!/bin/bash

echo "Removing old artifacts..."
rm app.o

echo "Building new binaries..."
g++ -Wall ./src/gl-game-engine.c ./src/doom_level.c -lglu -lglut -lopengl -o app.o

echo "Finished!"