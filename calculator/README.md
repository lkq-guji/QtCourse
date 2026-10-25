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
| 百分比 | % | % |
| 倒数 | 1/x | R |
| 平方 | x² | Q |
| 平方根 | √x | S |

按标准计算器逐步运算，`2 + 3 × 4 = 20`。连续操作符替换待执行的操作符，连续等号重复上一轮运算。`5 + =` 等待第二操作数。结果后输入数字开始新计算；结果后输入操作符继续使用结果。除零显示错误，输入数字、C或CE可恢复。

数字输入最多15位，计算使用 double、结果显示15位有效数字。C清空全部状态；CE保留待执行运算并清除当前操作数。支持百分比、倒数、平方和平方根，不包含内存和三角函数。

## 新增功能规则

按键排列为 `% CE C ⌫`、`1/x x² √x ÷`、`7 8 9 ×`、`4 5 6 −`、`1 2 3 +`、`± 0 . =`。

- 百分比：单独使用时当前数除以100，例如 `50 % = 0.5`。加减时以左操作数为基数，`200 + 10 % = 220`、`200 − 10 % = 180`；乘除时转换右操作数，`200 × 10 % = 20`、`200 ÷ 10 % = 2000`。加减与乘除的区别参考 [Windows计算器实现](https://github.com/microsoft/calculator/blob/main/src/CalcManager/CEngine/scifunc.cpp)。
- 倒数、平方和平方根作用于当前显示的操作数，例如 `4 → 1/x → 0.25`、`12 → x² → 144`、`81 → √x → 9`。待处理的四则运算保留，例如 `5 + 9 √x = 8`。
- 一元运算后输入数字会替换当前项，按操作符则使用运算后的值继续计算；CE可清除右操作数。零的倒数、负数开平方和溢出会显示错误，数字或清除键可恢复。

## 代码学习顺序

1. `calculatorwindow.ui`：QVBoxLayout组织显示区和提示，QGridLayout按参考图排列6行4列共24个按钮。
2. `calculatorwindow.cpp`：连接所有 clicked，统一调用 `dispatch`。
3. `calculatorengine.h/.cpp`：Entering、WaitingOperand、Result、Error四种状态，集中处理命令。
4. `eventFilter`：只处理本窗口及子控件，ShortcutOverride只接管事件，KeyPress才执行命令。
5. `styles.qss`、`resources.qrc`：视觉样式及内嵌资源。

鼠标和键盘共用 `dispatch → engine.input`。参考 [Qt官方计算器示例](https://doc.qt.io/qt-6/qtwidgets-widgets-calculator-example.html) 的布局及集中处理思路；本实验采用约定的逐步运算。

## 实际测试与迭代

在 Qt Creator 打开 `calculator/tests/tests.pro`，构建并运行，覆盖状态机与真实窗口交互。

测试覆盖四则运算、小数输入、重复小数点、连续操作符、退格、清除、除零恢复、结果后继续输入、百分比、倒数、平方、平方根、数值溢出、键鼠混合操作及窗口缩小时的按键布局。当前测试结果为61项通过、0项失败。

## Git 分支

本次分支为 [experiment01-calculator](https://github.com/lkq-guji/QtCourse/tree/experiment01-calculator)，逐步提交界面、引擎、测试、键盘、修复和样式。

本机上传目录为 `QT/worktrees/experiment01-calculator`，在这里修改后用 `git add`、`git commit`、`git push` 更新本实验。[main](https://github.com/lkq-guji/QtCourse/tree/main) 维护导航。
