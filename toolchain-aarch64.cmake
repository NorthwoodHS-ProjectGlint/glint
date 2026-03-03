# Cross-compilation toolchain for aarch64 (ARM64) - Raspberry Pi 4B, etc.
# Usage: cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=toolchain-aarch64.cmake

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify cross-compiler
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc CACHE STRING "C compiler")
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++ CACHE STRING "C++ compiler")
set(CMAKE_AR aarch64-linux-gnu-ar CACHE STRING "Archiver")
set(CMAKE_RANLIB aarch64-linux-gnu-ranlib CACHE STRING "Ranlib")

# Flags for cross-compilation
set(CMAKE_C_FLAGS "-mcpu=cortex-a72 -mtune=cortex-a72" CACHE STRING "C flags")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-a72 -mtune=cortex-a72" CACHE STRING "C++ flags")

# Search for programs in the build host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# For libraries and headers in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

# Set library architecture for pkg-config
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)
