# Glint

Glint is a library and operating system framework for handheld gaming devices. It provides a complete system with a bootloader, filesystem management, graphics, and HID support.

## Prerequisites

### Required Libraries

Before building Glint, ensure you have the following dependencies installed:

- **CMake** (>= 3.16)
- **pkg-config**
- **GLFW3** (`glfw3`) - Required for glint library, bootloader, and homescreen
- **STB** (`stb`) - Required for glint library
- **libcjson** (`libcjson`) - Required for system tools (glt_execcreate)
- C++ compiler with C++17 support

#### Component-Specific Dependencies

- **Glint Library** (core): GLFW3, STB
- **Bootloader**: glint, GLFW3
- **Homescreen**: glint, GLFW3
- **System Tools** (glt_execcreate): libcjson

#### Installing Dependencies

On Debian/Ubuntu:
```bash
sudo apt-get install cmake pkg-config libglfw3-dev libstb-dev libcjson-dev
```

On Fedora:
```bash
sudo dnf install cmake pkgconfig glfw-devel stb-devel cjson-devel
```

On Arch Linux:
```bash
sudo pacman -S cmake pkgconfig glfw stb cjson
```

## Building

Glint can be built and installed using CMake or the provided build script.

### Quick Build (Recommended)

The easiest way to build and run Glint:

```bash
sudo sh ./build.sh
```

This script will:
- Build and install the Glint library
- Build the bootloader and homescreen
- Build the example application from `examples/app-template`
- Copy the example to `_device/titles/`
- Launch the bootloader

### Manual Build

From within the `glint/` directory:

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
sudo make install
cd ..
```

This will:
- Build the core `libglint.so` library
- Build the bootloader executable
- Build the homescreen application
- Build the `glt_execcreate` tool
- Install headers to `/usr/local/include/glint/`
- Install the library to `/usr/local/lib/`
- Install `glint.pc` pkg-config file
- Install system tools to `/usr/local/bin/`

The bootloader executable will be located at `build/system/bootloader/bootloader`.

## Running

The **bootloader** is the main executable for Glint. It initializes the system and launches the home screen.

### Running the Bootloader

From within the `glint/` directory, after building:

```bash
cd build/system/bootloader
./bootloader
```

The bootloader should be run from a directory that will act as the device's virtual filesystem root. For a clean environment, you can create a dedicated directory:

```bash
mkdir -p device_root && cd device_root
../build/system/bootloader/bootloader
```

This directory will store system apps and user data.

## Quick Start

To create a new Glint application, use the provided template:

```bash
# Copy the template
cp -r examples/app-template my_game
cd my_game

# Edit the project details
# - Update .titleconfig with your game's info
# - Edit src/main.cpp with your game code
# - Add resources to res/

# Build your game
mkdir -p _build && cd _build
cmake ..
make
```

Your `.glt` file will be created in the project directory.

## Project Structure

- `src/` - Core library source code (IO, filesystem, graphics, HID)
- `include/` - Public header files
- `system/bootloader/` - Bootloader application
- `system/apps/` - System applications (homescreen, etc.)
- `tools/` - Build tools (executable creator, resource packer)
- `examples/` - Example applications and templates
  - `app-template/` - Ready-to-use application template

## Development

The Glint library (`libglint`) provides APIs for:
- **Filesystem management** (`fs.cpp`)
- **Executable loading** (`exec.cpp`)
- **OpenGL/Graphics** (`gl.cpp`, `glad.c`)
- **Input/HID** (`hid.cpp`)
- **Title/Game management** (`title.cpp`)
- **I/O operations** (`io.cpp`)

## Documentation

Comprehensive documentation is available in the following files:

- **[examples/app-template/README.md](examples/app-template/README.md)** - Quick start guide for the application template
  
- **[BUILDING.md](BUILDING.md)** - Complete build system documentation
  - Build script usage
  - Two-stage build process
  - `glt_execcreate` tool usage
  - Title configuration format
  - Resource packaging
  - CMake integration
  - Application template guide
  - Troubleshooting

- **[APPLICATION_FRAMEWORK.md](APPLICATION_FRAMEWORK.md)** - Application development guide
  - Getting started with the template
  - Required functions (`glattach`, `app_setup`, `app_cycle`, `app_present`, `app_shutdown`)
  - Application lifecycle
  - Pause/resume system
  - Complete examples
  - Limitations and constraints
  - Best practices

- **[API_REFERENCE.md](API_REFERENCE.md)** - Complete API reference
  - I/O module
  - Filesystem module (with resource mounting)
  - Graphics module (textures, shaders, rendering)
  - HID module (input)
  - Title/Executable module
  - Resource system (mount points)
  - Usage examples

