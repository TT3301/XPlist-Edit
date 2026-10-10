#!/usr/bin/env python3
"""Fail on non-ARM dependencies or development-machine library paths."""
import pathlib, plistlib, subprocess, sys
bundle = pathlib.Path(sys.argv[1]).resolve()
checked = []
minimum = []
for path in bundle.rglob('*'):
    if not path.is_file() or path.is_symlink():
        continue
    description = subprocess.check_output(['file', '-b', str(path)], text=True)
    if 'Mach-O' not in description:
        continue
    architectures = subprocess.check_output(['lipo', '-archs', str(path)], text=True).split()
    assert 'arm64' in architectures, (path, architectures)
    dependencies = '\n'.join(subprocess.check_output(['otool', '-L', str(path)], text=True).splitlines()[1:])
    commands = '\n'.join(subprocess.check_output(['otool', '-l', str(path)], text=True).splitlines()[1:])
    for forbidden in ['/opt/homebrew', '/usr/local', '/Volumes/T7/Github', '/Users/']:
        assert forbidden not in dependencies, (path, dependencies)
        assert forbidden not in commands, (path, 'development path in load commands')
    for line in commands.splitlines():
        if line.strip().startswith('minos '):
            minimum.append(tuple(map(int, line.strip().split()[1].split('.'))))
    checked.append(str(path.relative_to(bundle)))
assert checked, 'No Mach-O files found'
info = plistlib.loads((bundle / 'Contents/Info.plist').read_bytes())
assert tuple(map(int, info['LSMinimumSystemVersion'].split('.'))) >= max(minimum)
assert info['CFBundleVersion'] == '1.2.47.1'
assert info['XplistEdition'] == '1.2.47-arm64.1'
subprocess.run(['codesign', '--verify', '--deep', '--strict', '--verbose=2', str(bundle)], check=True)
print(f'PASS: {len(checked)} ARM64 Mach-O files; no development paths; signature valid; minimum macOS {info["LSMinimumSystemVersion"]}')
