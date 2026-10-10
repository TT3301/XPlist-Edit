import pathlib, plistlib, sys
root = pathlib.Path(sys.argv[1])
count = 0
for name in ["类型样例 XML", "类型样例 BIN", "根数组 XML", "根数组 BIN"]:
    source = root / (name + ".plist")
    original = plistlib.loads(source.read_bytes())
    for label in ["xml", "bin"]:
        saved = source.with_name(source.stem + f"-saved-{label}.plist")
        assert plistlib.loads(saved.read_bytes()) == original, saved
        assert saved.read_bytes().startswith(b"bplist00") == (label == "bin"), saved
        count += 1
edited = root / "类型样例 XML-edited.plist"
expected = plistlib.loads((root / "类型样例 XML.plist").read_bytes())
expected["标题"] = "已修改的中文"
assert plistlib.loads(edited.read_bytes()) == expected
print(f"PASS: {count} XML/binary application roundtrips and one edited document; all types match plistlib")
