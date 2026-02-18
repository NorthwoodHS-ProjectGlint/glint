# Glint Build System Documentation

## Overview

The Glint build system uses CMake and custom tooling to create executable packages that can run on the Glint handheld operating system. The build process involves compiling shared libraries and packaging them with resources into `.glt` executable files.

## Build Architecture

### Two-Stage Build Process

The build system operates in two stages:

#### Stage 1: Core Library Build
```bash
mkdir -p _glintbuild && cd _glintbuild
cmake ../glint
make -j$(nproc)
sudo make install
```

This stage:
- Builds the shared `libglint.so` library
- Installs headers to `/usr/local/include/glint/`
- Installs the library to `/usr/local/lib/`
- Installs `glint.pc` pkg-config file
- Builds and installs the `glt_execcreate` tool

#### Stage 2: Application Build
```bash
mkdir -p _build && cd _build
cmake ..
make -j$(nproc)
```

This stage:
- Builds applications (bootloader, homescreen, games)
- Packages applications into `.glt` files using `glt_execcreate`
- Links against the installed `libglint.so`

## Custom Build Tool: `glt_execcreate`

### Purpose

`glt_execcreate` is a command-line tool that packages shared libraries into `.glt` executable files, including:
- The compiled shared library (`.so` file)
- Title metadata (name, ID, description, icon, tags)
- Resource files (images, sounds, data)

### Usage

```bash
glt_execcreate <input_binary> <title_config>
```

**Parameters:**
- `<input_binary>`: Path to the compiled shared library (`.so` file)
- `<title_config>`: Path to the `.titleconfig` JSON file

### Output

Creates a `.glt` file named after the title ID specified in the configuration, containing:
- **Header**: Magic number "GLTE", executable size, resource size
- **Title Info**: ID, name, description, icon data (128x128), tags
- **Executable Data**: The compiled shared library
- **Resource Pack**: Packaged resource files

## Title Configuration File

### Format

Create a `.titleconfig` JSON file for your application:

```json
{
  "title": {
    "id": "000400000000001",
    "name": "My Game",
    "description": "A fun game for the Glint handheld",
    "icon": "icon.png",
    "tags": ["game", "action", "multiplayer"],
    "resources": "res/"
  },
  "dbg": {
    "direct_copy": true,
    "copy_directory": "/path/to/_device/sys/apps"
  }
}
```

### Configuration Fields

#### `title` Section
- **`id`** (required): Unique 15-character identifier for your application
  - System apps: `000400020000XXX`
  - User titles: `000400000000XXX`
- **`name`** (required): Display name (max 32 characters)
- **`description`** (required): Short description (max 128 characters)
- **`icon`** (required): Path to icon image (will be resized to 128x128)
- **`tags`** (required): Array of up to 3 tags (max 16 chars each)
- **`resources`** (required): Path to resources directory (relative to `.titleconfig`)

#### `dbg` Section (Optional)
- **`direct_copy`**: If true, automatically copy the built `.glt` file
- **`copy_directory`**: Destination directory for automatic copy

## Resource Packaging

### Resource Directory Structure

Place all resources in a dedicated directory (e.g., `res/`):
```
res/
├── textures/
│   ├── sprite.png
│   └── background.png
├── sounds/
│   └── music.wav
└── data/
    └── levels.json
```

### Resource Pack Format

`glt_execcreate` creates a resource pack with:
- **Header**: Magic number "GLTR", resource count
- **Entries**: For each resource file:
  - Path length
  - Relative path (from resource directory)
  - Data size
  - File data

Resources are accessed at runtime using the relative paths.

## CMake Integration

### Application CMakeLists.txt Template

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_game)
set(CMAKE_CXX_STANDARD 17)

# Find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLINT REQUIRED glint)
pkg_check_modules(GLFW REQUIRED glfw3)

# Build the shared library
add_library(my_game SHARED
    src/main.cpp
    src/game.cpp
)

# Include directories
target_include_directories(my_game PRIVATE
    ${GLINT_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(my_game
    ${GLINT_LIBRARIES}
    ${GLFW_LIBRARIES}
)

# Package into .glt file
add_custom_target(run_glt ALL
    COMMAND glt_execcreate $<TARGET_FILE:my_game> ${CMAKE_CURRENT_SOURCE_DIR}/.titleconfig
    DEPENDS my_game
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
```

### Key Points

1. **Build as shared library** (`add_library(... SHARED ...)`)
2. **Link against glint and glfw3** via pkg-config
3. **Create custom target** to run `glt_execcreate` after build
4. **Use `$<TARGET_FILE:...>`** to get the built library path

## Build Output

### File Locations

After building:
- **Library build**: `_glintbuild/libglint.so`
- **Installed library**: `/usr/local/lib/libglint.so`
- **Application libraries**: `_build/<project>/lib<name>.so`
- **Packaged executables**: `<project>/<titleid>.glt`
- **Debug copies**: `_device/sys/apps/<titleid>.glt` (if configured)

### Executable File Structure

`.glt` files contain:
```
+------------------+
| Header (12 bytes)|
|  - Magic: "GLTE" |
|  - Exe size      |
|  - Res size      |
+------------------+
| Title Info       |
|  - ID (16 bytes) |
|  - Name (32 b)   |
|  - Desc (128 b)  |
|  - Icon (16384b) |
|  - Tags (48 b)   |
+------------------+
| Executable Data  |
| (.so file)       |
+------------------+
| Resource Pack    |
+------------------+
```

## Build Troubleshooting

### Common Issues

**Issue**: `glt_execcreate: command not found`
- **Cause**: Core library not installed
- **Solution**: Run Stage 1 build and `sudo make install`

**Issue**: `Package 'glint' not found`
- **Cause**: pkg-config can't find glint.pc
- **Solution**: Check `/usr/local/lib/pkgconfig` is in `PKG_CONFIG_PATH`

**Issue**: Resources not found at runtime
- **Cause**: Resources not in the directory specified in `.titleconfig`
- **Solution**: Verify `resources` path is correct relative to `.titleconfig`

**Issue**: Icon not loading
- **Cause**: Icon path incorrect or format unsupported
- **Solution**: Use PNG format, verify path in `.titleconfig`

## Best Practices

1. **Use pkg-config**: Always use `pkg_check_modules` for dependencies
2. **Relative paths**: Use relative paths in `.titleconfig` for portability
3. **Resource organization**: Structure resources in subdirectories by type
4. **Title IDs**: Use unique IDs, follow the numbering convention
5. **Clean builds**: Use separate build directories (`_build`, `_glintbuild`)
6. **Parallel builds**: Use `-j$(nproc)` for faster compilation
