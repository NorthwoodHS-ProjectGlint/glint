# Glint for Raspberry Pi 4B (ARM64)

This package contains a pre-built distribution of Glint for the Raspberry Pi 4B running a 64-bit operating system (aarch64).

## Prerequisites

- Raspberry Pi 4B (or compatible ARM64 device)
- Raspberry Pi OS 64-bit (or Ubuntu/Debian ARM64)
- OpenGL ES support (usually provided by the system)

## Quick Installation

1. Extract this archive:
   ```bash
   unzip glint-rpi4-aarch64.zip -d glint
   cd glint
   ```

2. Run the installation script:
   ```bash
   chmod +x install-rpi.sh
   ./install-rpi.sh
   ```

3. Launch Glint:
   ```bash
   glint
   ```

## Manual Installation

If you prefer to install manually:

1. Install dependencies:
   ```bash
   sudo apt-get update
   sudo apt-get install -y libglfw3 libcjson1 libstb0
   ```

2. Copy files to your preferred location:
   ```bash
   sudo cp -r lib include bin /opt/glint/
   ```

3. Add library path:
   ```bash
   echo "/opt/glint/lib" | sudo tee /etc/ld.so.conf.d/glint.conf
   sudo ldconfig
   ```

4. Run the bootloader:
   ```bash
   /opt/glint/bin/bootloader
   ```

## What's Included

- **lib/**: Glint shared library (libglint.so) and pkg-config files
- **include/**: Development headers for building Glint applications
- **bin/**: Bootloader and tools
- **titles/**: Example applications (.glt files)
- **install-rpi.sh**: Automated installation script

## Building Your Own Applications

After installation, you can build your own Glint applications using the installed library:

```bash
export PKG_CONFIG_PATH=/opt/glint/lib/pkgconfig:$PKG_CONFIG_PATH
cmake -B build -S .
cmake --build build
```

See the main Glint documentation for more details on application development.

## Troubleshooting

### Missing libraries error
If you get library errors, ensure dependencies are installed and library cache is updated:
```bash
sudo apt-get install -y libglfw3 libcjson1 libstb0
sudo ldconfig
```

### Display errors
Ensure you're running on a system with graphics support. For headless systems, you may need to configure a virtual display.

### Permission errors
The bootloader may need access to certain system resources. Run from a user account with appropriate permissions.

## Uninstallation

To remove Glint:

```bash
sudo rm -rf /opt/glint
sudo rm /etc/ld.so.conf.d/glint.conf
sudo rm /usr/local/bin/glint /usr/local/bin/glint-bootloader
sudo ldconfig
rm -rf ~/.local/share/glint ~/.config/glint
```

## Support

For issues, documentation, and source code, visit:
https://github.com/your-repo/glint
