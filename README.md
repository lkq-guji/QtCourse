# 实验二 简单文本编辑器

Qt Widgets / C++17 / qmake。使用 Designer 和 Action Editor 设计界面，支持多个独立文档窗口。

## 打开和运行

在 Qt Creator 打开 `texteditor/texteditor.pro`，选择 **Desktop Qt 6.11.1 MinGW 64-bit**，按 Ctrl+R。
请保留整个源码目录，`.ui`、资源文件、图标、头文件和源文件缺一不可。

文件、编辑、格式、查看和帮助菜单按功能分组，工具按钮和菜单共用同一个 QAction。

| 操作 | 快捷键或入口 |
| --- | --- |
| 新建独立窗口 / 打开 | Ctrl+N / Ctrl+O |
| 保存 / 另存为 | Ctrl+S / Ctrl+Shift+S |
| 关闭当前窗口 / 退出全部 | Ctrl+W / Ctrl+Q |
| 撤销 / 重做 | Ctrl+Z / Ctrl+Shift+Z |
| 剪切 / 复制 / 粘贴 / 全选 | Ctrl+X / Ctrl+C / Ctrl+V / Ctrl+A |
| 查找 / 替换 | Ctrl+F / Ctrl+H |
| 向下 / 向上连续查找 | F3 / Shift+F3 |
| 显示或隐藏行号 | Ctrl+L |
| 字体、字号、文字颜色与底色、编辑区背景 | 格式菜单及字号工具控件 |
| 自动换行 / 工具栏 / 状态栏 / 自动备份 | 格式或查看菜单中的勾选项 |

## 文件与备份规则

- TXT 仅保存文本内容，字体和颜色是显示设置，不写入文件。
- 读取 UTF-8、UTF-8 BOM、UTF-16 LE/BE BOM，统一以 UTF-8 保存；读取时将换行规范为 LF。
- 无效编码、二进制控制字符、不存在的文件及超过20 MiB的文件会被拒绝，原窗口内容保留。
- 使用 QSaveFile 原子写入；保存失败不清除修改标记，关闭前可选保存、放弃或取消。
- 已有内容的窗口打开文件时会创建新窗口；同一个文件已经打开则激活已有窗口。
- 每60秒为修改中的文档写恢复备份，**不会覆盖原文件，也不会清除未保存标记**。
- 使用“文件 → 恢复自动备份”打开备份目录，恢复到新的未命名窗口后另存。备份存放在 Qt 的 AppLocalDataLocation/recovery，入口会直接定位该目录。

查找支持大小写、方向与循环。全部替换先收集原文匹配再倒序修改，替换内容包含查找词也能正常结束，整个操作可以一次撤销。

## 项目结构与学习顺序

1. `mainwindow.ui`：Action Editor、菜单、工具栏及零边距布局；CodeEditor 通过 Promote 提升自 QPlainTextEdit。
2. `mainwindow.cpp`：文件和窗口状态、Action 使能、状态栏、字体设置、60秒备份。
3. `documentio.cpp`：编码校验、文件读取、QSaveFile 原子保存。
4. `searchdialog.ui`、`searchdialog.cpp`：查找与替换共用布局，用模式控制替换控件的显示。
5. `codeeditor.h/.cpp`：行号区域绘制、滚动同步、当前行提示和文字底色。
6. `aboutdialog.ui`：姓名、学号、班级及实际 Qt 版本。

在 Qt Creator 打开 `texteditor/tests/tests.pro` 运行回归测试，覆盖文件异常、保存/另存为、关闭取消/保存失败、剪贴板、查找/替换、恢复备份、Unicode 行列和多窗口隔离。

## 录屏流程

建议录制3分钟左右，使用实际窗口连续操作。报告中的截图来自实际 Qt 程序；测试视频由学生自行录制。

1. 打开项目并运行，展示菜单、工具栏、行号及状态栏。
2. Ctrl+N 新建，输入两行文字，Ctrl+S 保存为 `录屏笔记.txt`，再加一行，展示标题星号。
3. Ctrl+W，展示未保存提示，选择“取消”；再次 Ctrl+S，星号消失。
4. Ctrl+O 打开 `texteditor/samples/lesson.txt`，展示中文、自动换行和独立窗口。
5. Ctrl+F 查找 `Qt`，连续查找两次；查找 `不存在的内容`，展示“未找到”。
6. Ctrl+H 将 `Qt` 全部替换为 `文本编辑`，展示替换计数；回到编辑器 Ctrl+Z 撤销，Ctrl+Shift+Z 重做。
7. 选中一段文字，展示复制/剪切变为可用，复制粘贴；取消选中，展示按钮状态变化。
8. Ctrl+L 切换行号，格式菜单切换自动换行，查看菜单隐藏/恢复工具栏或状态栏。
9. 如有时间，打开 `samples/invalid-utf8.txt` 展示错误提示和原文保留，打开关于窗口展示个人信息。
10. 最后展示 GitHub 本分支的 Commits 页面，或 Qt Creator 的 Git 日志。

可在开始录制后让未保存文档保持修改状态至少60秒，再用“恢复自动备份”展示恢复能力。不要在录制时使用真实的重要文件。

## 开发与参考

教学视频参考了主窗口/对话框、文件读写、剪贴板、配色、查找替换、状态栏、行号控件及隐藏行号部分的关键界面和代码片段。
实现同时参考 [Qt Code Editor 示例](https://doc.qt.io/qt-6/qtwidgets-widgets-codeeditor-example.html)、[QPlainTextEdit](https://doc.qt.io/qt-6/qplaintextedit.html)、[QSaveFile](https://doc.qt.io/qt-6/qsavefile.html) 与 [QStringConverter](https://doc.qt.io/qt-6/qstringconverter.html)。

源码分支：`codex/experiment02-texteditor`。实验报告、运行截图、日志和提交压缩包在本地整理，不上传到本分支。
