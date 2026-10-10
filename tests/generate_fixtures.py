import datetime, pathlib, plistlib, sys
root = pathlib.Path(sys.argv[1])
root.mkdir(parents=True, exist_ok=True)
value = {
    "标题": "中文 & <内容> 🧪",
    "空白": "   ", "空字符串": "",
    "最大整数": 2**63 - 1, "负整数": -(2**63),
    "实数": 1.2345678901234567,
    "日期": datetime.datetime(2026, 10, 10, 1, 2, 3),
    "Data": bytes(range(256)),
    "Item key": "必须保留键名", "plist": "普通字典键",
    "启用": True, "禁用": False,
    "数组": [True, False, "   ", b"\x00\xff", {"嵌套": [123, "测试"]}, [], {}],
    "空字典": {}, "空数组": [],
}
for name, item in [("类型样例", value), ("根数组", list(value.values()))]:
    for label, fmt in [("XML", plistlib.FMT_XML), ("BIN", plistlib.FMT_BINARY)]:
        (root / f"{name} {label}.plist").write_bytes(plistlib.dumps(item, fmt=fmt, sort_keys=False))
(root / "无效.plist").write_bytes(b"bplist00broken")
