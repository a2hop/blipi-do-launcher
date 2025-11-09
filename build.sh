#!/bin/bash

# Build script for BliPi Do C++ application

echo "Building BliPi Do..."

# Create build directory if it doesn't exist
mkdir -p build

# Compile the resource file
echo "Compiling resources..."
glib-compile-resources --target=build/resources.c --generate-source --sourcedir=src src/resources.xml

if [ $? -ne 0 ]; then
    echo "Resource compilation failed!"
    exit 1
fi

# Compile the application
echo "Compiling application..."
g++ src/main.cpp src/com/window.cpp src/com/config_parser.cpp build/resources.c -o build/blipi-do \
    `pkg-config --cflags --libs gtk+-3.0` \
    `pkg-config --cflags --libs libxml-2.0` \
    -std=c++11

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Build successful! Executable: build/blipi-do"
    
    # Copy actions.xml to build directory
    cp actions.xml build/
    echo "Copied actions.xml to build directory"
    
    exit 0
else
    echo "Build failed!"
    exit 1
fi
