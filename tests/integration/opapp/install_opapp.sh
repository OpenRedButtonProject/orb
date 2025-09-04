#! /bin/bash

set -euxo pipefail

# Requires adb and a device connected
# Use `adb root` to get root access

# This is currently hardcoded in the source code but should be configurable.
# See content/browser/hbbtv_package_url_loader_factory.cc
INSTALL_DIR=/data/data/org.chromium.content_shell_apk/files/hbbtv_packages/1234/abcd

# Minimal OpApp
adb shell mkdir -p        $INSTALL_DIR/video
adb push index.html       $INSTALL_DIR/index.html
adb push app.js           $INSTALL_DIR/app.js
adb push app_styles.css   $INSTALL_DIR/app_styles.css
adb push video/video.html $INSTALL_DIR/video/video.html
adb push video/script.js  $INSTALL_DIR/video/script.js
adb push video/styles.css $INSTALL_DIR/video/styles.css
