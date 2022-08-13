#!/bin/bash

echo "Removing old artifacts..."
rm app.o

echo "Building new binaries..."
g++ -Wall gl-game-engine.c -lGL -lGLU -lglut -o app.o

echo "Finished!"