# Xplist Apple Silicon 自用版

基于 [ic005k/Xplist](https://github.com/ic005k/Xplist) 的本地二开，沿用 C++ / Qt Widgets / qmake 和原有编辑界面。

- 上游基准提交：`bf8129b25541603d53aa2c5227b0fe774bcb35e9`。
- 本地分支：`apple-silicon`。
- 二开版本：`1.2.47-arm64.1`；macOS 数字版本为 `1.2.47`，构建版本为 `1.2.47.1`。
- 本次安装包：Apple Silicon / ARM64，**macOS 27.0 或更高版本**。仅在本机 macOS 27.0（26A428）实测。

## 安装和使用

构建产物在项目的 `dist/` 目录：

- `Xplist-arm64.dmg`：打开后，将 `Xplist.app` 拖到 Applications。
- `Xplist.app`：已包含运行依赖，可复制到其他目录运行，无需安装 Qt 或 Homebrew。
- `SHA256SUMS.txt`：DMG 的 SHA-256 校验值。

版本采用 ad-hoc 签名，未做 Developer ID 公证，定位为本机自用。保留上游作者、项目链接和许可证；应用内检查更新与自动覆盖更新均禁用。

## 构建

本次工具链：ARM64 Homebrew Qt **6.11.2**、Apple clang **21.0.0**、Command Line Tools 的 macOS **27.0 SDK**。Qt 模块使用 `qtbase`、`qttools`、`qttranslations`；Homebrew 同时安装了它们所需的 `qtsvg` 等依赖。

```bash
brew install qtbase qttools qttranslations
git clone https://github.com/TT3301/XPlist-Edit.git
cd XPlist-Edit
./scripts/build-macos-arm64.sh
```

脚本使用独立的 `build/arm64/` 目录，并通过 `CONFIG+=arm64_self_use` 开启 ARM64 自用版配置。默认从 `brew --prefix qtbase` 定位工具；其他 Qt 6 安装可设置 `QT_BIN_DIR` 指向其 `bin` 目录。

打包使用 `macdeployqt`，随后清理本机库标识和搜索路径、逐层 ad-hoc 签名，并检查架构、依赖路径、部署下限及签名。安装包最低系统版本取所有内置 Mach-O 的最高要求。本次 Qt 主库的部署下限为 14.0，但 Homebrew 的部分依赖要求 27.0，因此最终包标注 27.0；未宣称兼容旧版 macOS。

Qt 6.11.2 对 macOS 27 SDK 会显示未测试配置提示。当前版本的保证以本机运行验证为准，编译日志保留在 `build/arm64/package.log`。

## 改动

- 替换 Qt 6 已移除的正则、文本流编码、布局边距、集合转换和比较辅助 API；UTF-8 编码使用 Qt 5 / 6 条件编译。
- macOS XML / 二进制读写统一使用 `/usr/bin/plutil`。读取转换使用独立临时目录；保存先在临时目录完成转换，再使用 `QSaveFile` 原子提交。无效数据、转换失败和只读目标均不覆盖原文件。
- 保存失败保留未保存状态，关闭标签时选择保存但保存失败会取消关闭；成功保存保留撤销历史。
- 修复数组布尔值读取、以 `Item` 开头及名为 `plist` 的字典键保存、纯空白字符串在 Qt 6 DOM 中的保留。
- 移除启动改写应用 `Info.plist` 的行为，版本信息在构建时写入。
- 会话恢复延后到事件循环启动，使用启动时的文件列表快照并避免重复打开；Finder 打开文件支持应用处于非活动状态。

未重写界面或解析器，未新增业务功能，未更新 Windows / Linux / 旧版 macOS 的发布流程。现有编辑器主要支持以字典或数组为根节点的 plist；其他根类型会提示无法打开。

## 验证

```bash
./scripts/test-macos.sh
python3 scripts/verify-macos-bundle.py dist/Xplist.app
```

测试使用独立临时用户配置目录，不改写真实用户的配置和文件。样例在 `build/test-fixtures/`，测试结果在 `build/tests/results.txt`。

测试覆盖 XML / 二进制保存与重新打开、中文与空格路径、全部常见数据类型、64 位整数边界、纯空白字符串、嵌套数组与字典、编辑和撤销重做、新增删除、复制粘贴、搜索、多标签、无效输入、只读保存失败、未保存关闭提示，以及延后会话恢复。独立 Python `plistlib` 校验保存前后的数据语义，`plutil -lint` 校验文件结构。

实际打包与运行检查见 `VALIDATION-arm64.md`。
