# OpenAuto Dockerfile - Phase 5 Refactored
# Version: 4.1.0+20241124
# Refactoring: CustomButtonController Implementation Complete

FROM ubuntu:24.04

LABEL maintainer="OpenAuto Project"
LABEL version="4.1.0+20241124"
LABEL description="OpenAuto Android Auto Implementation - Phase 5 Refactored"
LABEL refactoring.phase="5"
LABEL refactoring.status="CustomButtonController Complete"

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    qt5-default \
    libqt5multimedia5 \
    libqt5multimediawidgets5 \
    libqt5bluetooth5 \
    libboost-log1.83.0 \
    libtag1v5 \
    librtaudio6 \
    libgps30 \
    libprotobuf32 \
    libusb-1.0-0 \
    alsa-utils \
    pulseaudio \
    && rm -rf /var/lib/apt/lists/*

# Create user for running application
RUN useradd -m -s /bin/bash openauto

# Create required directories
RUN mkdir -p /opt/crankshaft/assets \
    && mkdir -p /boot/crankshaft \
    && mkdir -p /tmp \
    && chown -R openauto:openauto /opt/crankshaft \
    && chown -R openauto:openauto /boot/crankshaft

# Copy application files
COPY bin/autoapp /usr/local/bin/autoapp
COPY bin/btservice /usr/local/bin/btservice
COPY assets/ /opt/crankshaft/assets/
COPY docs/ /opt/crankshaft/docs/
COPY *.md LICENSE RELEASE.txt /opt/crankshaft/

# Set permissions
RUN chmod +x /usr/local/bin/autoapp /usr/local/bin/btservice

# Create example custom button configurations
RUN echo "Volume Up#amixer set Master 5%+#100,200,100" > /boot/crankshaft/custom_button_1 \
    && echo "Volume Down#amixer set Master 5%-#200,100,100" > /boot/crankshaft/custom_button_2 \
    && echo "Screenshot#scrot /tmp/screenshot.png#100,255,100" > /boot/crankshaft/custom_button_3 \
    && chown openauto:openauto /boot/crankshaft/custom_button_*

# Switch to non-root user
USER openauto
WORKDIR /home/openauto

# Environment variables
ENV QT_QPA_PLATFORM=xcb
ENV DISPLAY=:0
ENV PULSE_RUNTIME_PATH=/tmp/pulse-runtime

# Expose typical X11 and audio ports
EXPOSE 6000

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD pgrep autoapp || exit 1

# Default command
CMD ["/usr/local/bin/autoapp"]

# Build information
LABEL build.date="2025-08-01"
LABEL build.phase="5"
LABEL build.controllers="SimpleMediaController,SystemController,CustomButtonController"
LABEL build.lines_removed="300+"
LABEL build.architecture="modular-controller-based"
