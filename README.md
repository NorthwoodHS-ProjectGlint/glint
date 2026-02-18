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

Glint uses a two-stage build process:

### 1. Build and Install the Glint Library

```bash
mkdir -p _glintbuild && cd _glintbuild
cmake ../glint
make -j$(nproc)
sudo make install
cd ..
```

This builds the core `libglint.so` library and installs it to your system.

### 2. Build the Project (Bootloader and Tools)

```bash
mkdir -p _build && cd _build
cmake ..
make -j$(nproc)
cd ..
```

This builds the bootloader executable and associated system tools.

### Quick Build

You can also use the provided build script:
```bash
./makerun.sh
```

Note: This script builds, installs, and runs the bootloader automatically.

## Running

The **bootloader** is the main executable for Glint. It initializes the system and launches the home screen.

### Running the Bootloader

```bash
mkdir -p _device && cd _device
../_build/glint/system/bootloader/bootloader
```

The `_device` directory acts as the device's virtual filesystem where system apps and user data are stored.

## Project Structure

- `src/` - Core library source code (IO, filesystem, graphics, HID)
- `include/` - Public header files
- `system/bootloader/` - Bootloader application
- `system/apps/` - System applications (homescreen, etc.)
- `tools/` - Build tools (executable creator, resource packer)

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

- **[BUILDING.md](BUILDING.md)** - Complete build system documentation
  - Two-stage build process
  - `glt_execcreate` tool usage
  - Title configuration format
  - Resource packaging
  - CMake integration
  - Troubleshooting

- **[APPLICATION_FRAMEWORK.md](APPLICATION_FRAMEWORK.md)** - Application development guide
  - Required functions (`glattach`, `app_setup`, `app_cycle`, `app_present`, `app_shutdown`)
  - Application lifecycle
  - Pause/resume system
  - Complete examples
  - Limitations and constraints
  - Best practices

- **[API_REFERENCE.md](API_REFERENCE.md)** - Complete API reference
  - I/O module
  - Filesystem module
  - Graphics module
  - HID module (input)
  - Title/Executable module
  - Usage examples

