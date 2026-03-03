#!/bin/bash
# Local build script for Glint on Raspberry Pi 4B (aarch64)
# Mirrors the GitHub Actions workflow: cross-compiles on x86_64 for aarch64

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="${BUILD_DIR:-.}"
OUTPUT_ZIP="${OUTPUT_ZIP:-glint-rpi4-aarch64.zip}"
DIST_DIR="$BUILD_DIR/dist"
WORKSPACE="$(cd "$BUILD_DIR" && pwd)"

echo -e "${GREEN}=== Glint Cross-Compilation for Raspberry Pi 4B (aarch64) ===${NC}"
echo "Working directory: $WORKSPACE"
echo ""

# Check for required tools
echo -e "${YELLOW}Checking for required tools...${NC}"
for tool in cmake g++ gcc pkg-config aarch64-linux-gnu-gcc aarch64-linux-gnu-g++; do
    if ! command -v "$tool" &> /dev/null; then
        echo -e "${RED}ERROR: Required tool not found: $tool${NC}"
        echo "Install cross-compilation tools with:"
        echo "  sudo apt-get install -y cmake build-essential pkg-config gcc-aarch64-linux-gnu g++-aarch64-linux-gnu binutils-aarch64-linux-gnu"
        exit 1
    fi
done
echo -e "${GREEN}✓ All tools found${NC}"
echo ""

# Check for required libraries (arm64 architecture)
echo -e "${YELLOW}Checking for target libraries...${NC}"
if ! pkg-config --list-all | grep -q "glfw3"; then
    echo -e "${RED}ERROR: Development libraries not found. Install with:${NC}"
    echo "  sudo apt-get install -y libglfw3-dev libcjson-dev libstb-dev"
    exit 1
fi
echo -e "${GREEN}✓ Target libraries found${NC}"
echo ""

# Stage 0: Build native glt_execcreate tool
echo -e "${YELLOW}[Stage 0] Building native glt_execcreate tool...${NC}"
cd "$WORKSPACE"
cmake -B _build-native -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DGLINT_BUILD_EXAMPLES=OFF > /dev/null 2>&1 || true
cmake --build _build-native --target glt_execcreate --parallel "$(nproc)" 2>&1 | grep -E "Built target|Error" || true
if [ -f "_build-native/tools/glt_execcreate/glt_execcreate" ]; then
    echo -e "${GREEN}✓ Native glt_execcreate built${NC}"
    export PATH="$(pwd)/_build-native/tools/glt_execcreate:$PATH"
else
    echo -e "${YELLOW}⚠ glt_execcreate not found in native build (may have errors, but continuing)${NC}"
fi
echo ""

# Clean dist directory
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

# Stage 1: Build and install core library (without examples)
echo -e "${YELLOW}[Stage 1] Building Glint core library for aarch64...${NC}"
cd "$WORKSPACE"
export PKG_CONFIG_PATH="/usr/lib/aarch64-linux-gnu/pkgconfig"
export PKG_CONFIG_LIBDIR="/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig"
export PKG_CONFIG_SYSROOT_DIR="/"

cmake --preset rpi4-aarch64 \
    -DGLINT_BUILD_EXAMPLES=OFF \
    -DPKG_CONFIG_EXECUTABLE=/usr/bin/pkg-config \
    -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu > /dev/null 2>&1

echo "Building (this may take a few minutes)..."
cmake --build --preset rpi4-aarch64 --parallel "$(nproc)" 2>&1 | tail -20

echo -e "${GREEN}✓ Core library built${NC}"

# Install to staging area
echo "Installing to staging area..."
cmake --install _build-rpi4 --prefix "$DIST_DIR" > /dev/null 2>&1
echo -e "${GREEN}✓ Core library installed to $DIST_DIR${NC}"
echo ""

# Stage 2: Build examples
echo -e "${YELLOW}[Stage 2] Building examples for aarch64...${NC}"
export PKG_CONFIG_PATH="/usr/lib/aarch64-linux-gnu/pkgconfig:$DIST_DIR/lib/pkgconfig"

cmake --preset rpi4-aarch64 \
    -DGLINT_BUILD_EXAMPLES=ON \
    -DCMAKE_PREFIX_PATH="$DIST_DIR:/usr/lib/aarch64-linux-gnu" \
    -DPKG_CONFIG_EXECUTABLE=/usr/bin/pkg-config > /dev/null 2>&1

echo "Building examples (this may take a few minutes)..."
cmake --build --preset rpi4-aarch64 --parallel "$(nproc)" 2>&1 | tail -20
echo -e "${GREEN}✓ Examples built${NC}"
echo ""

# Copy bootloader
echo -e "${YELLOW}Collecting artifacts...${NC}"
mkdir -p "$DIST_DIR/bin"
BOOTLOADER="_build-rpi4/system/bootloader/bootloader"
if [ -f "$BOOTLOADER" ]; then
    cp "$BOOTLOADER" "$DIST_DIR/bin/"
    echo -e "${GREEN}✓ Bootloader copied${NC}"
else
    echo -e "${RED}✗ Bootloader not found at $BOOTLOADER${NC}"
    exit 1
fi

# Collect .glt files
mkdir -p "$DIST_DIR/titles"
find . -name "*.glt" \
    ! -path "./_build-rpi4/*" \
    ! -path "./dist/*" \
    ! -path "./_build-native/*" \
    -exec cp {} "$DIST_DIR/titles/" \; 2>/dev/null || true
find _build-rpi4 -name "*.glt" -exec cp {} "$DIST_DIR/titles/" \; 2>/dev/null || true

GLT_COUNT=$(find "$DIST_DIR/titles" -name "*.glt" 2>/dev/null | wc -l)
echo -e "${GREEN}✓ Collected $GLT_COUNT .glt files${NC}"

# Add installation files
echo -e "${YELLOW}Adding installation files...${NC}"
if [ -f "install-rpi.sh" ]; then
    cp install-rpi.sh "$DIST_DIR/"
    chmod +x "$DIST_DIR/install-rpi.sh"
fi
if [ -f "README-RPI.md" ]; then
    cp README-RPI.md "$DIST_DIR/README.md"
fi
echo -e "${GREEN}✓ Installation files added${NC}"
echo ""

# Create zip archive
echo -e "${YELLOW}Creating distribution archive...${NC}"
cd "$DIST_DIR"
zip -r -q "$WORKSPACE/$OUTPUT_ZIP" .
cd "$WORKSPACE"
ZIP_SIZE=$(du -h "$OUTPUT_ZIP" | cut -f1)
echo -e "${GREEN}✓ Created $OUTPUT_ZIP ($ZIP_SIZE)${NC}"
echo ""

# Summary
echo -e "${GREEN}=== Build Complete ===${NC}"
echo ""
echo "Distribution package: $OUTPUT_ZIP"
echo "Staged files in: $DIST_DIR"
echo ""
echo "Next steps:"
echo "  1. Transfer to Raspberry Pi: scp $OUTPUT_ZIP pi@192.168.1.xxx:~/"
echo "  2. On the Pi:"
echo "     unzip $OUTPUT_ZIP -d glint"
echo "     cd glint"
echo "     ./install-rpi.sh"
echo "     glint"
echo ""
echo "Or install manually on a different system:"
echo "  unzip $OUTPUT_ZIP"
echo "  cd glint"
echo "  ./install-rpi.sh"
echo ""
