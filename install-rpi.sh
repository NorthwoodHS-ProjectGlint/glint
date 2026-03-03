#!/bin/bash
# Installation script for Glint on Raspberry Pi 4B
set -e

INSTALL_PREFIX="${INSTALL_PREFIX:-/opt/glint}"
INSTALL_SYSTEMD="${INSTALL_SYSTEMD:-no}"

echo "=== Glint Installation for Raspberry Pi 4B ==="
echo "Install prefix: $INSTALL_PREFIX"
echo ""

# Check architecture
if [ "$(uname -m)" != "aarch64" ]; then
    echo "WARNING: This script is designed for aarch64 (ARM64) Raspberry Pi."
    echo "Current architecture: $(uname -m)"
    read -p "Continue anyway? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Check for required runtime dependencies
echo "Checking dependencies..."
MISSING_DEPS=""
for dep in libglfw3 libcjson1; do
    if ! dpkg -l | grep -q "^ii.*$dep"; then
        MISSING_DEPS="$MISSING_DEPS $dep"
    fi
done

if [ -n "$MISSING_DEPS" ]; then
    echo "Missing dependencies:$MISSING_DEPS"
    echo "Installing dependencies..."
    sudo apt-get update
    sudo apt-get install -y libglfw3 libcjson1 libstb0
fi

# Create installation directory
echo "Creating installation directory..."
sudo mkdir -p "$INSTALL_PREFIX"

# Copy files
echo "Installing Glint files..."
sudo cp -r lib include bin "$INSTALL_PREFIX/"
if [ -d "titles" ]; then
    sudo mkdir -p "$INSTALL_PREFIX/titles"
    sudo cp -r titles/* "$INSTALL_PREFIX/titles/" 2>/dev/null || true
fi

# Create symlinks for easy access
echo "Creating symlinks..."
sudo ln -sf "$INSTALL_PREFIX/bin/bootloader" /usr/local/bin/glint-bootloader 2>/dev/null || true

# Update library cache
echo "Updating library cache..."
echo "$INSTALL_PREFIX/lib" | sudo tee /etc/ld.so.conf.d/glint.conf > /dev/null
sudo ldconfig

# Create launch script
cat <<'SCRIPT' | sudo tee "$INSTALL_PREFIX/bin/glint" > /dev/null
#!/bin/bash
cd "$HOME/.local/share/glint" 2>/dev/null || mkdir -p "$HOME/.local/share/glint" && cd "$HOME/.local/share/glint"
exec /opt/glint/bin/bootloader "$@"
SCRIPT
sudo chmod +x "$INSTALL_PREFIX/bin/glint"
sudo ln -sf "$INSTALL_PREFIX/bin/glint" /usr/local/bin/glint

# Create user data directory structure
mkdir -p "$HOME/.local/share/glint/titles"
mkdir -p "$HOME/.config/glint"

# Copy example titles to user directory
if [ -d "$INSTALL_PREFIX/titles" ]; then
    cp "$INSTALL_PREFIX/titles"/*.glt "$HOME/.local/share/glint/titles/" 2>/dev/null || true
fi

echo ""
echo "=== Installation Complete ==="
echo ""
echo "Glint has been installed to: $INSTALL_PREFIX"
echo ""
echo "To launch Glint:"
echo "  glint"
echo ""
echo "Or directly:"
echo "  $INSTALL_PREFIX/bin/bootloader"
echo ""
echo "User data location: $HOME/.local/share/glint"
echo "Configuration: $HOME/.config/glint"
echo ""

if [ "$INSTALL_SYSTEMD" = "yes" ]; then
    echo "Creating systemd user service..."
    mkdir -p "$HOME/.config/systemd/user"
    cat <<'SERVICE' > "$HOME/.config/systemd/user/glint.service"
[Unit]
Description=Glint Handheld OS
After=graphical.target

[Service]
Type=simple
ExecStart=/opt/glint/bin/glint
Restart=on-failure
Environment=DISPLAY=:0

[Install]
WantedBy=default.target
SERVICE
    
    systemctl --user daemon-reload
    echo "Systemd service created. Enable with:"
    echo "  systemctl --user enable glint.service"
    echo "  systemctl --user start glint.service"
fi
