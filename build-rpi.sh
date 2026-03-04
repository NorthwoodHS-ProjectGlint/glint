#!/bin/bash
# Local build script for Glint on Raspberry Pi 4B (aarch64)
# Mirrors the GitHub Actions workflow: cross-compiles on x86_64 for aarch64
#
# Usage:
#   ./build-rpi.sh              # Build (requires dependencies pre-installed)
#   ./build-rpi.sh --install    # Auto-install dependencies and build
#   ./build-rpi.sh -i           # Short form of --install

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

# Auto-install dependencies if --install flag is passed
if [ "$1" = "--install" ] || [ "$1" = "-i" ]; then
    echo -e "${YELLOW}Installing dependencies...${NC}"
    echo ""
    
    echo "Updating package lists..."
    sudo apt-get update
    echo ""
    
    echo "Installing native build tools..."
    sudo apt-get install -y \
        cmake \
        build-essential \
        pkg-config \
        wget \
        unzip \
        zip
    echo -e "${GREEN}✓ Native build tools installed${NC}"
    echo ""
    
    echo "Installing aarch64 cross-compilation toolchain..."
    sudo apt-get install -y \
        gcc-aarch64-linux-gnu \
        g++-aarch64-linux-gnu \
        binutils-aarch64-linux-gnu
    echo -e "${GREEN}✓ Cross-compilation toolchain installed${NC}"
    echo ""
    
    echo "Enabling arm64 multiarch support..."
    sudo dpkg --add-architecture arm64
    echo ""
    
    echo "Updating package lists for arm64 architecture..."
    sudo sed -i 's/^Types: deb$/Types: deb/' /etc/apt/sources.list.d/ubuntu.sources 2>/dev/null || true
    sudo sed -i '/^Architectures:/d' /etc/apt/sources.list.d/ubuntu.sources 2>/dev/null || true
    sudo sed -i '/^Types: deb/a Architectures: amd64' /etc/apt/sources.list.d/ubuntu.sources 2>/dev/null || true
    sudo sed -i 's/^deb http/deb [arch=amd64] http/' /etc/apt/sources.list 2>/dev/null || true
    
    sudo tee /etc/apt/sources.list.d/arm64-ports.list > /dev/null <<'EOF'
deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble main restricted universe multiverse
deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble-updates main restricted universe multiverse
deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports noble-security main restricted universe multiverse
EOF
    
    sudo apt-get update
    echo -e "${GREEN}✓ Multiarch configured${NC}"
    echo ""
    
    echo "Installing target libraries (arm64)..."
    sudo apt-get install -y \
        libglfw3-dev:arm64 \
        libcjson-dev:arm64 \
        libstb-dev:arm64
    echo -e "${GREEN}✓ Target libraries installed${NC}"
    echo ""

    echo "Installing native x86_64 libraries (for glt_execcreate)..."
    sudo apt-get install -y \
        libglfw3-dev \
        libcjson-dev \
        libstb-dev
    echo -e "${GREEN}✓ Native x86_64 libraries installed${NC}"
    echo ""
    
    echo -e "${GREEN}All dependencies installed successfully!${NC}"
    echo ""
fi

echo -e "${YELLOW}Checking for required tools...${NC}"
for tool in cmake g++ gcc pkg-config aarch64-linux-gnu-gcc aarch64-linux-gnu-g++; do
    if ! command -v "$tool" &> /dev/null; then
        echo -e "${RED}ERROR: Required tool not found: $tool${NC}"
        echo ""
        echo "Install all dependencies with:"
        echo "  $0 --install"
        echo ""
        echo "Or manually with:"
        echo "  sudo apt-get install -y cmake build-essential pkg-config gcc-aarch64-linux-gnu g++-aarch64-linux-gnu binutils-aarch64-linux-gnu"
        exit 1
    fi
done
echo -e "${GREEN}✓ All build tools found${NC}"
echo ""

# Check for required target libraries (arm64 architecture)
echo -e "${YELLOW}Checking for target libraries (arm64)...${NC}"
MISSING_LIBS=""

# Set pkg-config path for arm64
export PKG_CONFIG_PATH="/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig:$PKG_CONFIG_PATH"

for lib in glfw3 libcjson stb; do
    if ! pkg-config --exists $lib 2>/dev/null; then
        MISSING_LIBS="$MISSING_LIBS $lib"
    fi
done

if [ -n "$MISSING_LIBS" ]; then
    echo -e "${RED}ERROR: Missing target libraries:$MISSING_LIBS${NC}"
    echo ""
    echo "Install all dependencies with:"
    echo "  $0 --install"
    echo ""
    echo "Or manually install target libraries with:"
    echo "  sudo dpkg --add-architecture arm64"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install -y libglfw3-dev:arm64 libcjson-dev:arm64 libstb-dev:arm64"
    exit 1
fi
echo -e "${GREEN}✓ Target libraries found${NC}"
echo ""

# Stage 0: Build native glt_execcreate tool
echo -e "${YELLOW}[Stage 0] Building native glt_execcreate tool...${NC}"
cd "$WORKSPACE"
echo "Configuring native build..."
cmake -B _build-native -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DGLINT_BUILD_EXAMPLES=OFF
echo ""
echo "Building glt_execcreate..."
if cmake --build _build-native --target glt_execcreate --parallel "$(nproc)"; then
    echo -e "${GREEN}✓ Native glt_execcreate built${NC}"
    if [ -f "_build-native/tools/glt_execcreate/glt_execcreate" ]; then
        export PATH="$(pwd)/_build-native/tools/glt_execcreate:$PATH"
        echo "Added to PATH: $PATH"
    fi
else
    echo -e "${YELLOW}⚠ glt_execcreate build had issues, but continuing...${NC}"
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

echo "Environment:"
echo "  PKG_CONFIG_PATH=$PKG_CONFIG_PATH"
echo "  PKG_CONFIG_LIBDIR=$PKG_CONFIG_LIBDIR"
echo ""
echo "Configuring for aarch64..."
cmake --preset rpi4-aarch64 \
    -DGLINT_BUILD_EXAMPLES=OFF \
    -DPKG_CONFIG_EXECUTABLE=/usr/bin/pkg-config \
    -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu
echo ""
echo "Building core library (this may take a few minutes)..."
cmake --build --preset rpi4-aarch64 --parallel "$(nproc)"

echo -e "${GREEN}✓ Core library built${NC}"

# Install to staging area
echo ""
echo "Installing to staging area: $DIST_DIR"
cmake --install _build-rpi4 --prefix "$DIST_DIR"
echo -e "${GREEN}✓ Core library installed${NC}"
echo ""

# Stage 2: Build examples
echo -e "${YELLOW}[Stage 2] Building examples for aarch64...${NC}"
export PKG_CONFIG_PATH="/usr/lib/aarch64-linux-gnu/pkgconfig:$DIST_DIR/lib/pkgconfig"
echo "Environment:"
echo "  PKG_CONFIG_PATH=$PKG_CONFIG_PATH"
echo ""
echo "Reconfiguring with examples enabled..."
cmake --preset rpi4-aarch64 \
    -DGLINT_BUILD_EXAMPLES=ON \
    -DCMAKE_PREFIX_PATH="$DIST_DIR:/usr/lib/aarch64-linux-gnu" \
    -DPKG_CONFIG_EXECUTABLE=/usr/bin/pkg-config
echo ""
echo "Building examples (this may take a few minutes)..."
cmake --build --preset rpi4-aarch64 --parallel "$(nproc)"
echo -e "${GREEN}✓ Examples built${NC}"
echo ""

# Copy bootloader
echo -e "${YELLOW}Collecting artifacts...${NC}"
mkdir -p "$DIST_DIR/bin"
BOOTLOADER="_build-rpi4/system/bootloader/bootloader"
if [ -f "$BOOTLOADER" ]; then
    echo "Copying bootloader from: $BOOTLOADER"
    cp "$BOOTLOADER" "$DIST_DIR/bin/"
    ls -lh "$DIST_DIR/bin/bootloader"
    echo -e "${GREEN}✓ Bootloader copied${NC}"
else
    echo -e "${RED}✗ Bootloader not found at $BOOTLOADER${NC}"
    exit 1
fi
echo ""

# Collect .glt files
echo "Collecting .glt application files..."
mkdir -p "$DIST_DIR/titles"
find . -name "*.glt" \
    ! -path "./_build-rpi4/*" \
    ! -path "./dist/*" \
    ! -path "./_build-native/*" \
    -exec cp {} "$DIST_DIR/titles/" \; 2>/dev/null || true
find _build-rpi4 -name "*.glt" -exec cp {} "$DIST_DIR/titles/" \; 2>/dev/null || true

if [ -d "$DIST_DIR/titles" ] && [ "$(ls -A $DIST_DIR/titles)" ]; then
    echo "Found application files:"
    ls -lh "$DIST_DIR/titles/"
fi

GLT_COUNT=$(find "$DIST_DIR/titles" -name "*.glt" 2>/dev/null | wc -l)
echo -e "${GREEN}✓ Collected $GLT_COUNT .glt files${NC}"
echo ""

# Add installation files
echo -e "${YELLOW}Adding installation files...${NC}"
if [ -f "install-rpi.sh" ]; then
    echo "Copying install-rpi.sh"
    cp install-rpi.sh "$DIST_DIR/"
    chmod +x "$DIST_DIR/install-rpi.sh"
    ls -lh "$DIST_DIR/install-rpi.sh"
fi
if [ -f "README-RPI.md" ]; then
    echo "Copying README-RPI.md"
    cp README-RPI.md "$DIST_DIR/README.md"
    ls -lh "$DIST_DIR/README.md"
fi
echo -e "${GREEN}✓ Installation files added${NC}"
echo ""

# Create zip archive
echo -e "${YELLOW}Creating distribution archive...${NC}"
echo "Listing distribution directory structure:"
du -sh "$DIST_DIR"
find "$DIST_DIR" -type f -exec ls -lh {} \; | awk '{print "  " $9 " (" $5 ")"}'
echo ""
cd "$DIST_DIR"
echo "Creating zip file: $WORKSPACE/$OUTPUT_ZIP"
zip -r "$WORKSPACE/$OUTPUT_ZIP" .
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
echo "Archive contents:"
unzip -l "$OUTPUT_ZIP" | head -30
if [ $(unzip -l "$OUTPUT_ZIP" | wc -l) -gt 32 ]; then
    echo "  ... and more files"
fi
echo ""
echo -e "${GREEN}Next steps:${NC}"
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
