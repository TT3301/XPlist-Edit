#!/bin/bash
set -euo pipefail
PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
QT_BIN_DIR="${QT_BIN_DIR:-$(brew --prefix qtbase)/bin}"
TEST_DIR="$PROJECT_DIR/build/tests"
FIXTURES_DIR="$PROJECT_DIR/build/test-fixtures"
mkdir -p "$TEST_DIR"
TEST_HOME_DIR="$(mktemp -d "$PROJECT_DIR/build/test-home.XXXXXX")"
trap 'rm -rf "$TEST_HOME_DIR"' EXIT
python3 "$PROJECT_DIR/tests/generate_fixtures.py" "$FIXTURES_DIR"
(
  cd "$TEST_DIR"
  "$QT_BIN_DIR/qmake" "$PROJECT_DIR/tests.pro" CONFIG+=release "DESTDIR=$TEST_DIR/out"
  make -j"$(sysctl -n hw.ncpu)"
)
XPLIST_TEST_HOME="$TEST_HOME_DIR" XPLIST_FIXTURES="$FIXTURES_DIR" QT_QPA_PLATFORM=offscreen \
  "$TEST_DIR/out/XplistTests" -o "$TEST_DIR/results.txt",txt
python3 "$PROJECT_DIR/tests/verify_roundtrips.py" "$FIXTURES_DIR"
python3 - "$FIXTURES_DIR" <<'PY'
import pathlib, subprocess, sys
for path in pathlib.Path(sys.argv[1]).glob('*-saved-*.plist'):
    subprocess.run(['/usr/bin/plutil', '-lint', str(path)], check=True)
PY
cat "$TEST_DIR/results.txt"
