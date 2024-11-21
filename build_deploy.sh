#!/bin/bash

# TODO: check missing tools

# Create the build directory
mkdir -p build

# Configure the build
cmake -B build -S firmware

# Build the firmware
make -C build -j

# Deploy the firmware
picotool load -v -x build/nfc.uf2 -f

echo "Build and deployment completed."