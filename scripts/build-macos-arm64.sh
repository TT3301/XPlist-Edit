#!/bin/bash
set -euo pipefail
PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
QT_BIN_DIR="${QT_BIN_DIR:-$(brew --prefix qtbase)/bin}"
BUILD_DIR="$PROJECT_DIR/build/arm64"
DIST_DIR="$PROJECT_DIR/dist"
if [[ "$(uname -m)" != arm64 ]]; then
  echo "Build this edition on an Apple Silicon Mac." >&2
  exit 1
fi
mkdir -p "$BUILD_DIR" "$DIST_DIR"
"$QT_BIN_DIR/qmake" -v
(
  cd "$BUILD_DIR"
  "$QT_BIN_DIR/qmake" "$PROJECT_DIR/Xplist.pro" CONFIG+=release CONFIG+=arm64_self_use "DESTDIR=$BUILD_DIR/out"
  make -j"$(sysctl -n hw.ncpu)"
)
BUNDLE="$DIST_DIR/Xplist.app"
# Replace only the generated bundle from the previous build.
if [[ -e "$BUNDLE" ]]; then rm -rf "$BUNDLE"; fi
/usr/bin/ditto "$BUILD_DIR/out/Xplist.app" "$BUNDLE"
/usr/libexec/PlistBuddy -c 'Add :XplistEdition string 1.2.47-arm64.1' "$BUNDLE/Contents/Info.plist"
cp "$PROJECT_DIR/LICENSE" "$BUNDLE/Contents/Resources/LICENSE-Xplist.txt"
"$QT_BIN_DIR/macdeployqt" "$BUNDLE" -verbose=2
python3 "$PROJECT_DIR/scripts/prepare-macos-bundle.py" "$BUNDLE"
python3 "$PROJECT_DIR/scripts/verify-macos-bundle.py" "$BUNDLE"
STAGING_DIR="$(mktemp -d "$BUILD_DIR/dmg.XXXXXX")"
trap 'rm -rf "$STAGING_DIR"' EXIT
/usr/bin/ditto "$BUNDLE" "$STAGING_DIR/Xplist.app"
ln -s /Applications "$STAGING_DIR/Applications"
hdiutil create -volname 'Xplist ARM64' -srcfolder "$STAGING_DIR" -ov -format UDZO "$DIST_DIR/Xplist-arm64.dmg"
(cd "$DIST_DIR" && shasum -a 256 Xplist-arm64.dmg > SHA256SUMS.txt)
echo "Built $DIST_DIR/Xplist-arm64.dmg"
