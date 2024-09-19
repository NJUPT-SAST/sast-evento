#!/usr/bin/env bash
if ! command -v dpkg-deb &> /dev/null
then
    echo "dpkg-deb could not be found"
    exit
fi

if [ -z "$1" ]
then
    echo "No input directory supplied"
    exit
fi
INPUT_DIR=$1
VERSION=$(cat "$INPUT_DIR/sast-evento-version.txt")
TMP_DIR=$(mktemp -d)
SCRIPT=$(realpath "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

mkdir -p "$TMP_DIR/opt/sast-evento"
cp -r $INPUT_DIR/* "$TMP_DIR/opt/sast-evento"

mkdir -p "$TMP_DIR/usr/bin"
ln -s "/opt/sast-evento/sast-evento" "$TMP_DIR/usr/bin/sast-evento"

mkdir -p "$TMP_DIR/DEBIAN"
cat > "$TMP_DIR/DEBIAN/control" << EOF
Package: sast-evento
Version: $VERSION
Architecture: x64
Priority: optional
Essential: no
Depends: libqt6network6, libqt6widgets6
Maintainer: NJUPT-SAST-C++
Description: An event management system developed and used by NJUPT SAST
EOF

mkdir -p "$TMP_DIR/usr/share/icons/hicolor/scalable/apps"
install -D -m644 "$SCRIPTPATH/../ui/assets/image/icon/evento.svg" "$TMP_DIR/usr/share/icons/hicolor/scalable/apps/sast-evento.svg"

mkdir -p "$TMP_DIR/usr/share/applications"
cat > "$TMP_DIR/usr/share/applications/sast-evento.desktop" << EOF
[Desktop Entry]
Name=SAST Evento
Version=$VERSION
Comment=An event management system developed and used by NJUPT SAST
Exec=sast-evento
Icon=sast-evento
Terminal=false
Type=Application
Categories=Education;
Terminal=false
EOF
chmod 644 "$TMP_DIR/usr/share/applications/sast-evento.desktop"

dpkg-deb --build "$TMP_DIR" "sast-evento.deb"
rm -rf "${TMP_DIR:?}"
