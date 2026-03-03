#!/bin/bash
# Glint installer for Raspberry Pi
# Usage: ./install.sh [install_prefix]
# Example: ./install.sh /usr/local
#          ./install.sh ~/glint

set -e

REPO="NorthwoodHS-ProjectGlint/glint"
ARTIFACT_NAME="glint-dist"
PREFIX="${1:-/usr/local}"
TMPDIR=$(mktemp -d)

echo "==> Glint Installer"
echo "    Repo:   $REPO"
echo "    Prefix: $PREFIX"
echo ""

# Check for gh CLI
if ! command -v gh &>/dev/null; then
  echo "Installing GitHub CLI..."
  curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg \
    | sudo dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg
  echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] \
    https://cli.github.com/packages stable main" \
    | sudo tee /etc/apt/sources.list.d/github-cli.list > /dev/null
  sudo apt-get update && sudo apt-get install -y gh
fi

# Authenticate if needed
if ! gh auth status &>/dev/null; then
  echo "Please authenticate with GitHub:"
  gh auth login
fi

echo "==> Downloading latest '$ARTIFACT_NAME' artifact..."
gh run download \
  --repo "$REPO" \
  --name "$ARTIFACT_NAME" \
  --dir "$TMPDIR"

ARCHIVE=$(find "$TMPDIR" -name "*.tar.gz" | head -1)
if [ -z "$ARCHIVE" ]; then
  echo "ERROR: No .tar.gz found in downloaded artifact"
  exit 1
fi

echo "==> Extracting to $PREFIX ..."
sudo mkdir -p "$PREFIX"
sudo tar -xzf "$ARCHIVE" -C "$PREFIX"

echo "==> Updating shared library cache..."
sudo ldconfig

echo "==> Setting PKG_CONFIG_PATH..."
PKGCONF_LINE="export PKG_CONFIG_PATH=\"$PREFIX/lib/pkgconfig:\$PKG_CONFIG_PATH\""
if ! grep -qF "$PKGCONF_LINE" ~/.bashrc; then
  echo "$PKGCONF_LINE" >> ~/.bashrc
  echo "    Added to ~/.bashrc"
fi

# Install runtime deps if missing
echo "==> Checking runtime dependencies..."
sudo apt-get install -y --no-install-recommends libglfw3 libcjson1 2>/dev/null || true

echo ""
echo "Done! Glint installed to $PREFIX"
echo ""
echo "Installed files:"
echo "  $PREFIX/bin/bootloader"
echo "  $PREFIX/bin/glt_execcreate"
echo "  $PREFIX/lib/libglint.so"
echo "  $PREFIX/lib/pkgconfig/glint.pc"
echo "  $PREFIX/include/glint/"
echo "  $PREFIX/titles/*.glt"
echo ""
echo "Run 'source ~/.bashrc' or open a new terminal to apply PKG_CONFIG_PATH."

rm -rf "$TMPDIR"
