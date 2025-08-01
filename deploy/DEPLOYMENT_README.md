# OpenAuto Deployment Package

**Version**: 4.1.0+20241124  
**Build Date**: August 1, 2025  
**Architecture**: x86_64 Linux  
**Refactoring Status**: ✅ **Phase 5 Complete - CustomButtonController Implementation**

## What's New in This Build

### 🎯 **Major Refactoring Completed**
This deployment package includes the complete 5-phase MainWindow refactoring with:

1. **SimpleMediaController** - Media playback and playlist management
2. **SystemController** - Brightness, volume, and theme control
3. **CustomButtonController** - Configuration-driven custom button handling
4. **Cleaned MainWindow** - Reduced from 2,325 to 2,201 lines
5. **Modern Architecture** - Controller-based design with clean separation of concerns

### 🚀 **Key Improvements**
- **300+ lines removed** from MainWindow while adding functionality
- **Configuration-driven design** for maximum flexibility
- **Thread-safe operations** with proper Qt5 signal/slot integration
- **Secure command execution** via QProcess and SafeSystemExecutor
- **Enhanced maintainability** with modular controller architecture

## Package Contents

```
deploy/
├── bin/
│   ├── autoapp          # Main OpenAuto application (52MB)
│   └── btservice        # Bluetooth service component
├── assets/              # UI assets (icons, fonts, resources)
├── docs/                # Complete refactoring documentation
├── README.md            # Original project documentation
├── LICENSE              # GPL v3 License
├── RELEASE.txt          # Release notes
└── DEPLOYMENT_README.md # This file
```

## System Requirements

### Required Libraries
- Qt5 (Core, Widgets, Multimedia, MultimediaWidgets, Bluetooth, Network)
- Boost (Log, System) v1.83+
- TagLib (Audio metadata)
- RtAudio (Audio I/O)
- GPS support libraries
- Protocol Buffers
- AAP and AASDK libraries
- USB 1.0 support

### Platform Support
- **Primary**: x86_64 Linux (Ubuntu 24.04+ recommended)
- **Target**: Raspberry Pi with Crankshaft integration
- **Compatible**: Most modern Linux distributions

## Installation Instructions

### 1. Install Dependencies
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install \
    qt5-default libqt5multimedia5-dev libqt5multimediawidgets5 \
    libqt5bluetooth5-dev libboost-all-dev \
    libtag1-dev librtaudio-dev libgps-dev \
    libprotobuf-dev libusb-1.0-0-dev

# Install AAP/AASDK libraries (platform-specific)
```

### 2. Deploy Binaries
```bash
# Copy binaries to system location
sudo cp bin/autoapp /usr/local/bin/
sudo cp bin/btservice /usr/local/bin/
sudo chmod +x /usr/local/bin/autoapp /usr/local/bin/btservice
```

### 3. Install Assets
```bash
# Copy assets to appropriate location
sudo mkdir -p /opt/crankshaft/assets
sudo cp -r assets/* /opt/crankshaft/assets/
```

### 4. Configure Custom Buttons (Optional)
```bash
# Create custom button configuration directory
sudo mkdir -p /boot/crankshaft

# Example custom button configuration
echo "Volume Up#amixer set Master 5%+#100,200,100" | sudo tee /boot/crankshaft/custom_button_1
echo "Volume Down#amixer set Master 5%-#200,100,100" | sudo tee /boot/crankshaft/custom_button_2

# Enable custom buttons by creating trigger files
touch /tmp/custom_button_1
touch /tmp/custom_button_2
```

## Custom Button Configuration

The refactored CustomButtonController supports configuration-driven button setup:

### Configuration Format
```
Button_Label#shell_command#rgb_color_values
```

### Example Configurations
```bash
# Media controls
echo "Next Track#playerctl next#100,150,255" > /boot/crankshaft/custom_button_1
echo "Prev Track#playerctl previous#100,150,255" > /boot/crankshaft/custom_button_2

# System controls  
echo "Reboot#sudo reboot#255,100,100" > /boot/crankshaft/custom_button_3
echo "Screenshot#scrot /tmp/screenshot.png#100,255,100" > /boot/crankshaft/custom_button_4

# Custom scripts
echo "Night Mode#/opt/crankshaft/service_daynight.sh app night#150,150,150" > /boot/crankshaft/custom_button_5
echo "Day Mode#/opt/crankshaft/service_daynight.sh app day#255,255,100" > /boot/crankshaft/custom_button_6
```

### Enable Buttons
```bash
# Create trigger files to enable configured buttons
touch /tmp/custom_button_1  # Enables button 1
touch /tmp/custom_button_2  # Enables button 2
# ... etc for buttons 3-6
```

## Running the Application

### Direct Execution
```bash
# Run with default configuration
/usr/local/bin/autoapp

# Run with debug output
QT_LOGGING_RULES="*=true" /usr/local/bin/autoapp

# Run Bluetooth service (separate process)
/usr/local/bin/btservice
```

### Systemd Service (Recommended)
```bash
# Create service file
sudo tee /etc/systemd/system/openauto.service << EOF
[Unit]
Description=OpenAuto Android Auto Implementation
After=network.target sound.target

[Service]
Type=simple
User=pi
Environment=DISPLAY=:0
ExecStart=/usr/local/bin/autoapp
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

# Enable and start service
sudo systemctl enable openauto
sudo systemctl start openauto
```

## Architecture Overview

### Controller-Based Design
```
MainWindow (UI Coordinator)
├── SimpleMediaController    # Media playback, playlists, metadata
├── SystemController        # Brightness, volume, themes
└── CustomButtonController  # User-configurable actions
```

### Key Benefits
- **Modular Design**: Each controller has single responsibility
- **Configuration-Driven**: Buttons, commands, and settings via files
- **Thread-Safe**: Proper Qt5 signal/slot integration
- **Maintainable**: Clean separation enables easy testing and updates
- **Extensible**: Established patterns for future controller additions

## Troubleshooting

### Common Issues

**1. Missing Dependencies**
```bash
# Check for missing libraries
ldd /usr/local/bin/autoapp | grep "not found"

# Install missing packages
sudo apt install <missing-package>
```

**2. Qt5 Plugin Issues**
```bash
# Set Qt plugin path
export QT_PLUGIN_PATH=/usr/lib/x86_64-linux-gnu/qt5/plugins
```

**3. Audio Issues**
```bash
# Check audio system
pactl info
aplay -l

# Test audio
speaker-test -c 2 -t sine
```

**4. Custom Button Issues**
```bash
# Check configuration files
ls -la /boot/crankshaft/custom_button_*
cat /boot/crankshaft/custom_button_1

# Check trigger files
ls -la /tmp/custom_button_*

# Test command manually
/bin/bash -c "command_from_config_file"
```

### Debug Information
```bash
# Generate debug logs
QT_LOGGING_RULES="*=true" /usr/local/bin/autoapp 2>&1 | tee autoapp.log

# Check system resources
htop
journalctl -u openauto -f
```

## Support and Documentation

### Complete Documentation
See the `docs/` directory for comprehensive documentation:
- `DEVELOPMENT_SESSION_2025-08-01.md` - Latest refactoring summary
- `MAINWINDOW_REFACTORING_SUMMARY.md` - Complete architecture overview
- `TECHNICAL_DEBT_ANALYSIS_2025.md` - Code quality improvements

### Build Information
- **Compiled**: August 1, 2025
- **Compiler**: GCC with C++17 support
- **Build Type**: Release with debug symbols
- **CMake Version**: 3.5.1+
- **Qt Version**: 5.15+

### Contact and Issues
- **Project Repository**: OpenAuto GitHub
- **Issues**: Report via GitHub Issues
- **Documentation**: See included `docs/` directory

---

**Deployment Package Status**: ✅ **Ready for Production**  
**Refactoring Status**: ✅ **Phase 5 Complete**  
**Next Development**: Phase 6 - Bluetooth Controller (Planned)
