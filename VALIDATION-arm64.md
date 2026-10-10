# ARM64 自用版验证记录

验证日期：2026-10-10（Asia/Shanghai）。上游基准 `bf8129b25541603d53aa2c5227b0fe774bcb35e9`，二开版本 `1.2.47-arm64.1`。

## 环境

- Apple Silicon / ARM64，macOS 27.0（26A428）。
- Qt / QtTest 6.11.2，Apple clang 21.0.0，macOS SDK 27.0。
- 仅本机实测；最终包的 `LSMinimumSystemVersion` 为 27.0，取全部打包依赖的最高要求。

## 自动化结果

- QtTest：**12 passed, 0 failed**。涵盖四组 XML / 二进制读写、编辑和历史、新增删除、复制粘贴、搜索、只读保存失败和失败后的关闭保护、多标签和无效输入、未保存关闭取消/放弃、资源及文件对话框对象，以及延后会话恢复。
- 独立 Python `plistlib`：**8 次完整保存再打开及 1 次修改保存**均通过语义校验。覆盖中文/emoji、纯空白/空字符串、正负 64 位整数边界、实数、UTC 日期、256 字节 Data、数组布尔值、嵌套容器、空容器，以及 `Item key` / `plist` 字典键。
- 所有 8 份保存结果均通过 `/usr/bin/plutil -lint`。
- 两份应用（`dist/` 原包及 DMG 安装副本）均通过完整架构、库路径和签名检查：**43 个 ARM64 Mach-O 文件**，未发现 Homebrew 或开发目录依赖路径。

## 实际安装运行结果

- 最终 DMG 已只读挂载，应用已从镜像复制到独立目录 `build/installed-final/Xplist.app`，随后卸载镜像。
- 独立副本正常启动，中文菜单和树形编辑界面正常显示。
- 在原生另存对话框保存 `界面保存 验证.plist` 成功；`plistlib` 与 `plutil -lint` 验证其全部数据与输入一致。
- 使用 macOS 文件打开事件打开二进制根数组成功，显示 `[BIN]` 标签、完整数据和多个标签页。
- 运行进程采样明确显示 `Code Type: ARM64`，实际加载库来自应用内部和系统路径，未加载 Homebrew 库。
- 应用运行、另存、Finder 文件打开和退出前后，**85 个文件/符号链接条目内容完全一致**；运行后完整签名检查仍通过。
- 验证结束后已关闭测试标签页，退出测试副本，保存的会话数量为 0。

初次独立安装测试发现会话恢复阶段的子进程等待超时；现已把会话恢复延后到事件循环启动后，并固定文件列表快照。新增回归测试及最终安装运行测试均通过。

## 产物及证据

- `dist/Xplist.app`、`dist/Xplist-arm64.dmg`、`dist/SHA256SUMS.txt`。
- 自动测试：`build/tests/results.txt` 和 `build/tests/test.log`。
- 构建与打包：`build/arm64/package.log`。
- 独立副本：`build/installed-final/bundle-check.txt`、`runtime-sample.txt`、`before.json`、`after.json`、`after-check.txt`。
- 此记录对应 DMG SHA-256：`39605df3ec27e52aed4d145b7c7bb62f9d607e141c9435d50bd877e7e364dd48`。

这些检查未验证其他 macOS 版本、Intel、Windows、Linux 或 Developer ID 公证。Qt 6.11.2 对 macOS 27 SDK 的未测试配置提示仍保留；本次交付按本机实测结果说明支持范围。
