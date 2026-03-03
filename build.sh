#!/bin/bash
set -e

if [ "$(id -u)" -eq 0 ]; then
	echo "This script should be run as your normal user, not with sudo."
	echo "Running with sudo can break DISPLAY/XAUTH under VNC and cause OpenGL startup crashes."
	exit 1
fi

# Stage 1: Build and install the core library and system apps (without examples)
echo "Building and installing Glint core library..."
mkdir -p _build && cd _build
cmake .. -DGLINT_BUILD_EXAMPLES=OFF
sudo make -j$(nproc)
sudo make install
cd ../

# Stage 2: Build the examples (after library is installed)
echo "Building example applications..."
mkdir -p _examples_build && cd _examples_build
cmake .. -DGLINT_BUILD_EXAMPLES=ON
make -j$(nproc)
cd ../

# Stage 3: Copy examples to device and run bootloader
echo "Setting up device filesystem..."
mkdir -p _device/titles
mkdir -p _device/sys/apps

# Copy built .glt files
cp system/apps/homescreen/*.glt _device/sys/apps/ 2>/dev/null || echo "Note: homescreen .glt not found"
cp examples/graphics/cube/*.glt _device/titles/ 2>/dev/null || echo "Note: cube example .glt not found"

# Run bootloader
echo "Launching bootloader..."
cd _device
../_build/system/bootloader/bootloader