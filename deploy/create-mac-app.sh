#!/usr/bin/env bash

INPUT_DIR=$1
OUTPUT_DIR=$2
VERSION=$(cat "$INPUT_DIR/sast-evento-version.txt")
SCRIPT=$(realpath "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

IFS='.' read -r major minor patch <<< "$VERSION"
IFS='-' read -r patch <<< "$patch"
if [ -z "$major" ] ; then
    major='0'
fi
if [ -z "$minor" ] ; then
    minor='0'
fi
if [ -z "$patch" ] ; then
    patch='0'
fi
MACOSX_VERSION="${major}.${minor}.${patch}"

mkdir -p "$OUTPUT_DIR/Contents/MacOS"
cp -r $INPUT_DIR/* "$OUTPUT_DIR/Contents/MacOS"

mkdir -p "$OUTPUT_DIR/Contents/Resources"
install -m644 "$SCRIPTPATH/../ui/assets/image/icon/app.icns" "$OUTPUT_DIR/Contents/Resources/sast-evento.icns"

cat > "$OUTPUT_DIR/Contents/Info.plist" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  	<key>CFBundleDevelopmentRegion</key>
  	<string>English</string>
  	<key>CFBundleExecutable</key>
  	<string>sast-evento</string>
  	<key>CFBundleIconFile</key>
  	<string>sast-evento.icns</string>
  	<key>CFBundleIdentifier</key>
  	<string>fun.sast.evento</string>
  	<key>CFBundleInfoDictionaryVersion</key>
  	<string>6.0</string>
  	<key>CFBundleName</key>
  	<string>SAST Evento</string>
  	<key>CFBundlePackageType</key>
  	<string>APPL</string>
  	<key>CFBundleShortVersionString</key>
  	<string>${MACOSX_VERSION}</string>
  	<key>CFBundleVersion</key>
  	<string>${MACOSX_VERSION}</string>
  	<key>CSResourcesFileMapped</key>
  	<true/>
  	<key>NSHumanReadableCopyright</key>
  	<string>MIT license Copyright (c) 2024 NJUPT-SAST</string>
  	<key>NSPrincipalClass</key>
  	<string>NSApplication</string>
  	<key>NSHighResolutionCapable</key>
  	<string>True</string>
  	<key>NSUserNotificationAlertStyle</key>
  	<string>alert</string>
</dict>
</plist>
EOF
