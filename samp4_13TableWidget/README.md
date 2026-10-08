# 第四章作业：QTableWidget 学生名单

本作业在提供的 `samp4_13TableWidget` 示例工程上修改，对应作业说明中的 samp4_9。保留左侧示例控件，新增工具栏“设置学生名单”按钮。

## 运行

在 Qt Creator 打开 `samp4_13TableWidget/samp4_13.pro`（如果已经进入项目目录，直接打开 `samp4_13.pro`），选择 Desktop Qt 6.11.1 MinGW 64-bit Kit，构建并运行。

1. 初始显示原示例的六列表头和空表格。
2. 点击工具栏“设置学生名单”，切换为五行七列。
3. 刘楷钦的学号、姓名单元格显示为红色粗体。
4. 点击任意一行的任意列，底部显示该学生的学号及关联籍贯。选择“行选择”后同样有效。
5. 再次点击工具按钮可恢复五人原始名单。增删行、设置行数、读取到文本仍可使用。
6. “设置水平表头”切回原六列示例；名单模式下“初始化表格数据”重新载入五人名单。

## 名单和来源

以学号 2024414290221（刘楷钦）定位点名册中的行，取前两行、本人和后两行，按原顺序显示五人。

| 学号 | 姓名 | 班级 | 原点名册 Excel 行号 |
| --- | --- | --- | --- |
| 2024414290219 | 林泳恩 | 2024软件2班 | 57 |
| 2024414290220 | 刘楚彤 | 2024软件2班 | 58 |
| 2024414290221 | 刘楷钦 | 2024软件2班 | 59 |
| 2024414290223 | 刘泽 | 2024软件2班 | 60 |
| 2024414290224 | 龙智森 | 2024软件2班 | 61 |

学号、姓名和班级来自本地 `周四点名册.xls` 的 `Sheet1`（B、C、D 列）。用户补充：五人性别均为男，院系为计算机科学与技术学院，修读性质为初修。专业按软件班填写为软件工程。五人的籍贯按最新要求统一为“广东东莞”，并关联在各自的姓名 item 中。

源码中的 `data/students.json` 只保存这五人的必要数据以及来源说明，整份点名册留在本地。JSON 通过 `res.qrc` 编译为程序资源，所以运行时无需安装 Excel，也无需连接原始点名册。修改 JSON 后重新构建即可更新。

## 关键实现，按这个顺序学习

### 1. Action Editor 和工具按钮

在 Qt Creator 双击 `mainwindow.ui`，进入设计模式。在 Action Editor 中可以找到本次添加的 `actSetStudents`：

- text：设置学生名单
- icon：`:/images/icons/boy.ico`，使用示例自带图标
- 工具栏：`mainToolBar`

自己练习时，在 Action Editor 新建 QAction，填写 objectName 和 text，然后拖到工具栏。QToolBar 会自动为 QAction 创建一个 QToolButton。这里将动作和工具栏声明保存在 `.ui` 中，Designer 可以继续编辑。

`mainwindow.h` 声明 `void on_actSetStudents_triggered();`。`ui->setupUi(this)` 根据 `on_对象名_信号名` 规则自动连接动作的 triggered 信号，所以点击工具按钮后执行这个槽函数。不要再重复手写 connect。

### 2. 槽函数设置七列和五行

`on_actSetStudents_triggered()` 先读取并验证资源中的五人名单，然后：

```cpp
ui->tableInfo->clear();
setHeaders({tr("学号"), tr("姓名"), tr("性别"), tr("行政班级"),
            tr("院系"), tr("专业"), tr("修读性质")});
ui->tableInfo->setRowCount(students.size());
```

`setHeaders()` 内部设置列数并创建红色粗体表头，随后逐行调用 `createRosterRow()`，用 `setItem(row, col, item)` 放入单元格。题目中的“行政班几”按示意图写为“行政班级”。

批量重建表格时使用 QSignalBlocker，避免旧行被清除、姓名项尚未创建时，状态栏槽函数访问不完整数据。

### 3. 仅本人学号和姓名标红加粗

`createRosterRow()` 中按学号判断，而不是写死第三行：

```cpp
if (col < 2 && values.at(0) == "2024414290221") {
    QFont font = item->font();
    font.setBold(true);
    item->setFont(font);
    item->setForeground(QBrush(Qt::red));
}
```

这样改变名单顺序时仍然标记正确的人。最终名单中刘楷钦位于第三行。

### 4. 姓名 item 关联籍贯

表格只显示七列，但 item 可以另存不直接显示的数据。姓名单元格使用两个独立角色：

```cpp
item->setData(Qt::UserRole, values.at(0));       //学号
item->setData(Qt::UserRole + 1, birthplace);   //籍贯
```

实际代码为这两个角色定义了 StudentIdRole、BirthplaceRole，便于阅读。学号存为 QString，不能沿用旧例子的 uint，否则十三位学号会溢出。

### 5. QLabel 状态栏显示

构造函数创建 `labBirthplace` 并加入 `statusBar`。当前单元格、选区或内容变化时调用 `refreshCurrentStudent()`。

```cpp
const auto *nameItem = ui->tableInfo->item(row, 1);
QString birthplace = nameItem->data(Qt::UserRole + 1).toString();
```

从当前行的姓名 item 取数据，所以点击学号、专业等任意列也能显示该学生的籍贯。没有有效选择时清除旧状态，避免删除行后仍显示上一位同学的信息。以后只需补充 JSON 中的 `birthplace` 并重新构建。

## 验证

环境：Windows 11、Qt 6.11.1、MinGW 13.1.0。主程序编译通过。

测试项目：`tests/roster_test.pro`。在 Qt Creator 打开后构建运行。Windows 平台下 Qt Test 结果为 3 passed、0 failed（包含初始化、功能测试和清理）。

功能测试覆盖工具按钮实际点击、图标加载、完整五人数据和七个表头、本人两格红色粗体、任意列和整行选择更新状态栏、十三位学号保留、重复加载、空单元格文本读取、增删行、清空所有行和原六列模式恢复。测试在内存中临时写入“测试籍贯”，验证状态栏确实读取姓名 item 的关联数据；生产名单的籍贯为“广东东莞”。

## Git 分支

- [主分支概览](https://github.com/lkq-guji/QtCourse/tree/main)
- [原 About 作业](https://github.com/lkq-guji/QtCourse/tree/chapter02-about)
- [本次表格作业](https://github.com/lkq-guji/QtCourse/tree/chapter04-tablewidget)

本地上传工作区为 `QT/worktrees/chapter04-tablewidget`。原 `QT/samp4_13TableWidget` 中修改后的项目和它自带的 `.git` 记录都保留。本次上传复制源文件进入独立工作区，不会把嵌套仓库误上传成子模块。

后续在上传工作区修改后，用 `git add`、`git commit`、`git push` 更新这个分支。如果修改的是外层原项目，需先把更改同步到上传工作区，再提交。
