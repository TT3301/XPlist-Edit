#!/usr/bin/env python3
"""Remove local install IDs/rpaths and set a truthful deployment minimum."""
import pathlib, plistlib, re, subprocess, sys
bundle = pathlib.Path(sys.argv[1]).resolve()
frameworks = bundle / 'Contents/Frameworks'
minimums = []
files = []
for path in bundle.rglob('*'):
    if not path.is_file() or path.is_symlink():
        continue
    if 'Mach-O' not in subprocess.check_output(['file', '-b', str(path)], text=True):
        continue
    commands = subprocess.check_output(['otool', '-l', str(path)], text=True)
    for block in commands.split('Load command '):
        if 'cmd LC_RPATH\n' in block:
            old = re.search(r'path (.*) \(offset', block).group(1)
            if old.startswith(('/opt/homebrew', '/usr/local', '/Users/', '/Volumes/')):
                subprocess.run(['install_name_tool', '-delete_rpath', old, str(path)], check=True)
        if 'cmd LC_ID_DYLIB\n' in block:
            old = re.search(r'name (.*) \(offset', block).group(1)
            if old.startswith(('/opt/homebrew', '/usr/local')):
                relative = path.relative_to(frameworks) if path.is_relative_to(frameworks) else pathlib.Path(path.name)
                subprocess.run(['install_name_tool', '-id', '@rpath/' + str(relative), str(path)], check=True)
        match = re.search(r'\bminos (\S+)', block)
        if match:
            minimums.append(tuple(map(int, match.group(1).split('.'))))
    files.append(path)
info_path = bundle / 'Contents/Info.plist'
info = plistlib.loads(info_path.read_bytes())
minimum = max(minimums)
info['LSMinimumSystemVersion'] = '.'.join(map(str, minimum))
info_path.write_bytes(plistlib.dumps(info, sort_keys=False))
# Sign standalone libraries/plugins, framework bundles, and the outer application in that order.
for path in files:
    if '.framework/' not in str(path) and path != bundle / 'Contents/MacOS/Xplist':
        subprocess.run(['codesign', '--force', '--sign', '-', '--timestamp=none', str(path)], check=True)
for framework in frameworks.glob('*.framework'):
    subprocess.run(['codesign', '--force', '--sign', '-', '--timestamp=none', str(framework)], check=True)
subprocess.run(['codesign', '--force', '--sign', '-', '--timestamp=none', str(bundle)], check=True)
print('Bundle minimum macOS:', info['LSMinimumSystemVersion'])
