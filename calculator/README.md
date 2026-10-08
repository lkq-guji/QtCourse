# 实验一 带键盘事件的计算器

刘楷钦，2024414290221，软件工程2班。Windows 11 / Qt 6.11.1 / MinGW 13.1.0。

本分支包含 Designer 可编辑的计算器界面、计算状态机和 Qt Test 测试代码。

## 打开和运行

1. 在 Qt Creator 中打开 `calculator/calculator.pro`（进入源码目录后直接打开 `calculator.pro`）。
2. 选择 Desktop Qt 6.11.1 MinGW 64-bit Kit，构建并运行。
3. 双击 `calculatorwindow.ui` 查看和修改布局、控件名称、按钮的 command 属性。
4. 本机已完成构建，可双击仓库根目录的 `run-calculator.cmd` 启动。新克隆项目先在 Qt Creator 构建。

## 操作约定

| 功能 | 鼠标 | 键盘 |
| --- | --- | --- |
| 数字与四则运算 | 对应按钮 | 0–9、+、-、*、/ |
| 小数点 | . | . 或 , |
| 等号 | = | Enter、小键盘Enter、= |
| 退格 | ⌫ | Backspace |
| 全部清除 | C | Esc 或 C |
| 清除当前操作数 | CE | Delete |
| 正负号 | ± | F9 |

按标准计算器逐步运算，`2 + 3 × 4 = 20`。连续操作符替换待执行的操作符，连续等号重复上一轮运算。`5 + =` 等待第二操作数。结果后输入数字开始新计算；结果后输入操作符继续使用结果。除零显示错误，输入数字、C或CE可恢复。

数字输入最多15位，计算使用 double、结果显示15位有效数字。C清空全部状态；CE保留待执行运算并清除当前操作数。功能范围不包含百分号、科学计算和内存。

## 代码学习顺序

1. `calculatorwindow.ui`：QVBoxLayout组织显示区和提示，QGridLayout排列20个按钮。
2. `calculatorwindow.cpp`：连接所有 clicked，统一调用 `dispatch`。
3. `calculatorengine.h/.cpp`：Entering、WaitingOperand、Result、Error四种状态，集中处理命令。
4. `eventFilter`：只处理本窗口及子控件，ShortcutOverride只接管事件，KeyPress才执行命令。
5. `styles.qss`、`resources.qrc`：视觉样式及内嵌资源。

鼠标和键盘共用 `dispatch → engine.input`。参考 [Qt官方计算器示例](https://doc.qt.io/qt-6/qtwidgets-widgets-calculator-example.html) 的布局及集中处理思路；本实验采用约定的逐步运算。

## 实际测试与迭代

在 Qt Creator 打开 `calculator/tests/tests.pro`，构建并运行，覆盖状态机与真实窗口交互。

测试覆盖四则运算、小数输入、重复小数点、连续操作符、退格、清除、除零恢复、结果后继续输入以及鼠标与键盘混合操作。

## Git 分支

本次分支为 [experiment01-calculator](https://github.com/lkq-guji/QtCourse/tree/experiment01-calculator)，逐步提交界面、引擎、测试、键盘、修复和样式。

本机上传目录为 `QT/worktrees/experiment01-calculator`，在这里修改后用 `git add`、`git commit`、`git push` 更新本实验。[main](https://github.com/lkq-guji/QtCourse/tree/main) 维护导航。
