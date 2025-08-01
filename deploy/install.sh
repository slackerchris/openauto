#!/bin/bash

# OpenAuto Deployment Script
# Version: 4.1.0+20241124
# Date: August 1, 2025
# Refactoring: Phase 5 Complete - CustomButtonController

set -e

echo "===================================================================================="
echo "OpenAuto Deployment Script - Phase 5 Complete (CustomButtonController)"
echo "Version: 4.1.0+20241124"
echo "Date: $(date)"
echo "===================================================================================="

# Check if running as root for system installation
if [ "$EUID" -eq 0 ]; then
    INSTALL_PREFIX="/usr/local"
    ASSETS_DIR="/opt/crankshaft"
    echo "Installing system-wide to $INSTALL_PREFIX"
else
    INSTALL_PREFIX="$HOME/.local"
    ASSETS_DIR="$HOME/.config/crankshaft"
    echo "Installing user-local to $INSTALL_PREFIX"
fi

# Create directories
echo "Creating directories..."
mkdir -p "$INSTALL_PREFIX/bin"
mkdir -p "$ASSETS_DIR/assets"
mkdir -p "$ASSETS_DIR/docs"
mkdir -p "/boot/crankshaft" 2>/dev/null || mkdir -p "$HOME/.config/crankshaft/buttons"

# Install binaries
echo "Installing binaries..."
cp bin/autoapp "$INSTALL_PREFIX/bin/"
cp bin/btservice "$INSTALL_PREFIX/bin/"
chmod +x "$INSTALL_PREFIX/bin/autoapp"
chmod +x "$INSTALL_PREFIX/bin/btservice"

echo "✅ Binaries installed:"
echo "   - $INSTALL_PREFIX/bin/autoapp ($(du -h "$INSTALL_PREFIX/bin/autoapp" | cut -f1))"
echo "   - $INSTALL_PREFIX/bin/btservice ($(du -h "$INSTALL_PREFIX/bin/btservice" | cut -f1))"

# Install assets
echo "Installing assets..."
cp -r assets/* "$ASSETS_DIR/assets/"

echo "✅ Assets installed to $ASSETS_DIR/assets/"

# Install documentation
echo "Installing documentation..."
cp -r docs "$ASSETS_DIR/"
cp README.md LICENSE RELEASE.txt "$ASSETS_DIR/"

echo "✅ Documentation installed to $ASSETS_DIR/docs/"

# Create example custom button configurations
echo "Creating example custom button configurations..."

BUTTON_DIR="/boot/crankshaft"
if [ ! -w "/boot" ] 2>/dev/null; then
    BUTTON_DIR="$HOME/.config/crankshaft/buttons"
fi

# Example button configurations
cat > "$BUTTON_DIR/custom_button_1" << 'EOF'
Volume Up#amixer set Master 5%+#100,200,100
EOF

cat > "$BUTTON_DIR/custom_button_2" << 'EOF'
Volume Down#amixer set Master 5%-#200,100,100
EOF

cat > "$BUTTON_DIR/custom_button_3" << 'EOF'
Screenshot#scrot /tmp/screenshot.png#100,255,100
EOF

cat > "$BUTTON_DIR/custom_button_4" << 'EOF'
Night Mode#/opt/crankshaft/service_daynight.sh app night#150,150,150
EOF

echo "✅ Example custom button configurations created in $BUTTON_DIR/"

# Create systemd service file
if [ "$EUID" -eq 0 ]; then
    echo "Creating systemd service..."
    cat > /etc/systemd/system/openauto.service << EOF
[Unit]
Description=OpenAuto Android Auto Implementation (Phase 5 Refactored)
After=network.target sound.target
Documentation=file://$ASSETS_DIR/docs/

[Service]
Type=simple
User=pi
Group=pi
Environment=DISPLAY=:0
Environment=QT_QPA_PLATFORM=xcb
ExecStart=$INSTALL_PREFIX/bin/autoapp
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

    systemctl daemon-reload
    echo "✅ Systemd service created: openauto.service"
    echo "   Enable with: sudo systemctl enable openauto"
    echo "   Start with:  sudo systemctl start openauto"
fi

# Create desktop entry
if [ "$EUID" -ne 0 ]; then
    mkdir -p "$HOME/.local/share/applications"
    cat > "$HOME/.local/share/applications/openauto.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=OpenAuto
Comment=Android Auto Implementation (Phase 5 Refactored)
Exec=$INSTALL_PREFIX/bin/autoapp
Icon=$ASSETS_DIR/assets/coverlogo.png
Terminal=false
Categories=AudioVideo;Audio;
EOF
    echo "✅ Desktop entry created for current user"
fi

# Check dependencies
echo "Checking system dependencies..."
MISSING_DEPS=()

check_lib() {
    if ! ldconfig -p | grep -q "$1"; then
        MISSING_DEPS+=("$1")
    fi
}

check_lib "libQt5Core.so.5"
check_lib "libQt5Widgets.so.5"
check_lib "libQt5Multimedia.so.5"
check_lib "libQt5Bluetooth.so.5"
check_lib "libboost_log.so"
check_lib "libtag.so"
check_lib "librtaudio.so"

if [ ${#MISSING_DEPS[@]} -eq 0 ]; then
    echo "✅ All required dependencies found"
else
    echo "⚠️  Missing dependencies:"
    for dep in "${MISSING_DEPS[@]}"; do
        echo "   - $dep"
    done
    echo ""
    echo "Install missing dependencies with:"
    echo "   sudo apt update"
    echo "   sudo apt install qt5-default libqt5multimedia5-dev libqt5multimediawidgets5 \\"
    echo "                    libqt5bluetooth5-dev libboost-all-dev libtag1-dev \\"
    echo "                    librtaudio-dev libgps-dev libprotobuf-dev libusb-1.0-0-dev"
fi

# Test binary
echo "Testing binary..."
if "$INSTALL_PREFIX/bin/autoapp" --help &>/dev/null || "$INSTALL_PREFIX/bin/autoapp" --version &>/dev/null; then
    echo "✅ Binary test successful"
else
    echo "⚠️  Binary test failed - check dependencies"
fi

echo ""
echo "===================================================================================="
echo "🎉 OpenAuto Deployment Complete!"
echo "===================================================================================="
echo ""
echo "📋 Installation Summary:"
echo "   📦 Binaries:      $INSTALL_PREFIX/bin/"
echo "   🎨 Assets:        $ASSETS_DIR/assets/"
echo "   📚 Documentation: $ASSETS_DIR/docs/"
echo "   ⚙️  Config:       $BUTTON_DIR/"
echo ""
echo "🚀 Quick Start:"
echo "   1. Install dependencies (if missing)"
echo "   2. Run: $INSTALL_PREFIX/bin/autoapp"
echo "   3. For service: sudo systemctl enable openauto && sudo systemctl start openauto"
echo ""
echo "🎛️  Custom Buttons:"
echo "   - Edit configs in: $BUTTON_DIR/custom_button_*"
echo "   - Enable with: touch /tmp/custom_button_1 (etc.)"
echo "   - Format: Label#Command#R,G,B"
echo ""
echo "📖 Documentation:"
echo "   - Complete refactoring docs: $ASSETS_DIR/docs/"
echo "   - Phase 5 summary: $ASSETS_DIR/docs/DEVELOPMENT_SESSION_2025-08-01.md"
echo "   - Architecture overview: $ASSETS_DIR/docs/MAINWINDOW_REFACTORING_SUMMARY.md"
echo ""
echo "✅ Phase 5 CustomButtonController Implementation Complete!"
echo "✅ Ready for Production Deployment"
echo "===================================================================================="
