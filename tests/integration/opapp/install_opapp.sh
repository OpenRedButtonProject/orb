#! /bin/bash

set -euxo pipefail

# Requires adb and a device connected
# Use `adb root` to get root access

INSTALL_DIR=/data/data/org.chromium.content_shell_apk/files/opapp

# Minimal OpApp
adb push index.html $INSTALL_DIR/index.html
adb push app.js $INSTALL_DIR/app.js
adb push app_styles.css $INSTALL_DIR/app_styles.css
adb push video.html $INSTALL_DIR/video.html
adb push script.js $INSTALL_DIR/script.js
adb push styles.css $INSTALL_DIR/styles.css
