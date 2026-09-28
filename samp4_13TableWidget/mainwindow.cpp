#include "mainwindow.h"
#include "ui_mainwindow.h"

#include    <QDate>
#include    <QTableWidgetItem>
#include    <QRandomGenerator>
#include    <QFile>
#include    <QJsonDocument>
#include    <QJsonArray>
#include    <QJsonObject>
#include    <QMessageBox>
#include    <QSignalBlocker>


//姓名 item 同时保存学号和籍贯；学号使用 QString，避免十三位学号溢出。
void MainWindow::createRosterRow(int row, const QStringList &values, const QString &birthplace)
{
    for (int col = 0; col < values.size(); ++col) {
        auto *item = new QTableWidgetItem(values.at(col), col == 1 ? int(ctName) : int(QTableWidgetItem::Type));
        item->setTextAlignment(Qt::AlignCenter);
        if (col == 1) {
            item->setData(StudentIdRole, values.at(0));
            item->setData(BirthplaceRole, birthplace);
        }
        if (col < 2 && values.at(0) == "2024414290221") {
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            item->setForeground(QBrush(Qt::red));
        }
        ui->tableInfo->setItem(row, col, item);
    }
}

void MainWindow::on_actSetStudents_triggered()
{
    QFile file(":/data/students.json");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("读取名单失败"), file.errorString());
        return;
    }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    const QJsonArray students = document.object().value("students").toArray();
    const QStringList keys = {"studentId", "name", "gender", "className", "department", "major", "studyType"};
    bool valid = error.error == QJsonParseError::NoError && students.size() == 5;
    for (const auto &value : students) {
        const auto student = value.toObject();
        for (const auto &key : keys) valid = valid && student.value(key).isString();
        valid = valid && student.value("birthplace").isString();
    }
    if (!valid) {
        QMessageBox::warning(this, tr("读取名单失败"), tr("名单应包含五名学生，且所有字段必须为文本。"));
        return;
    }

    { //批量重建时暂时阻断信号，避免访问尚未填好的姓名单元格。
        const QSignalBlocker blocker(ui->tableInfo);
        rosterMode = true;
        ui->tableInfo->setSortingEnabled(false);
        ui->tableInfo->clear();
        setHeaders({tr("学号"), tr("姓名"), tr("性别"), tr("行政班级"),
                    tr("院系"), tr("专业"), tr("修读性质")});
        ui->tableInfo->setRowCount(students.size());
        for (int row = 0; row < students.size(); ++row) {
            const auto student = students.at(row).toObject();
            QStringList values;
            for (const auto &key : keys) values << student.value(key).toString();
            createRosterRow(row, values, student.value("birthplace").toString());
        }
        ui->spinRowCount->setValue(students.size());
        ui->tableInfo->resizeColumnsToContents();
        ui->tableInfo->resizeRowsToContents();
        ui->tableInfo->horizontalHeader()->setStretchLastSection(true);
        ui->tableInfo->clearSelection();
        ui->tableInfo->setCurrentCell(-1, -1);
    }
    ui->textEdit->clear();
    refreshCurrentStudent();
}

void MainWindow::refreshCurrentStudent()
{
    const int row = ui->tableInfo->currentRow();
    const int col = ui->tableInfo->currentColumn();
    const auto *item = ui->tableInfo->item(row, col);
    labCellIndex->setText(row < 0 ? tr("当前单元格坐标：未选择")
                                : tr("当前单元格坐标：%1 行，%2 列").arg(row + 1).arg(col + 1));
    labCellType->setText(item ? tr("当前单元格类型：%1").arg(item->type()) : tr("当前单元格类型：无"));
    const auto *nameItem = ui->tableInfo->item(row, rosterMode ? 1 : colName);
    if (!nameItem || ui->tableInfo->selectedItems().isEmpty()) {
        labStudID->setText(tr("学生ID：未选择"));
        labBirthplace->setText(tr("籍贯：未选择学生"));
        return;
    }
    //点击这一行的任意列，都从姓名 item 中取出籍贯。
    const QString id = rosterMode && ui->tableInfo->item(row, 0)
        ? ui->tableInfo->item(row, 0)->text() : nameItem->data(StudentIdRole).toString();
    labStudID->setText(tr("学生ID：%1").arg(id));
    QString birthplace = nameItem->data(BirthplaceRole).toString();
    if (birthplace.isEmpty()) birthplace = tr("待补充");
    labBirthplace->setText(tr("籍贯：%1（%2）").arg(birthplace, nameItem->text()));
}

//为一行的单元格创建 Items
void MainWindow::createItemsARow(int rowNo,QString name,QString sex,QDate birth,QString nation,bool isPM,int score)
{
    uint studID=202105000;  //学号基数
    //姓名
    QTableWidgetItem *item=new  QTableWidgetItem(name, MainWindow::ctName);   //数据项类型为MainWindow::ctName
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    studID  +=rowNo;        //学号 =基数 + 行号
    item->setData(Qt::UserRole,QVariant(studID));           //设置studID为用户数据
    ui->tableInfo->setItem(rowNo,MainWindow::colName,item);

    //性别
    QIcon   icon;
    if (sex=="男")
        icon.addFile(":/images/icons/boy.ico");
    else
        icon.addFile(":/images/icons/girl.ico");

    item=new  QTableWidgetItem(sex,MainWindow::ctSex);      //type为MainWindow::ctSex
    item->setIcon(icon);
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    Qt::ItemFlags flags=Qt::ItemIsSelectable |Qt::ItemIsEnabled;    //不允许编辑
    item->setFlags(flags);
    ui->tableInfo->setItem(rowNo,MainWindow::colSex,item);  //为单元格设置Item

    //出生日期
    QString str=birth.toString("yyyy-MM-dd");   //日期转换为字符串
    item=new  QTableWidgetItem(str,MainWindow::ctBirth);        //type为MainWindow::ctBirth
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);   //文本对齐格式
    ui->tableInfo->setItem(rowNo,MainWindow::colBirth,item);

    //民族
    item=new  QTableWidgetItem(nation,MainWindow::ctNation);        //type为MainWindow::ctNation
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableInfo->setItem(rowNo,MainWindow::colNation,item);

    //是否党员
    item=new  QTableWidgetItem("党员",MainWindow::ctPartyM);      //type为 MainWindow::ctPartyM
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    flags= Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |Qt::ItemIsEnabled;   //不允许编辑，但可以更改复选状态
    item->setFlags(flags);
    if (isPM)
        item->setCheckState(Qt::Checked);
    else
        item->setCheckState(Qt::Unchecked);
    item->setBackground(QBrush(Qt::yellow));   //设置背景颜色
    ui->tableInfo->setItem(rowNo,MainWindow::colPartyM,item);

    //分数
    str.setNum(score);
    item=new  QTableWidgetItem(str,MainWindow::ctScore);    //type为MainWindow::ctPartyM
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableInfo->setItem(rowNo,MainWindow::colScore,item);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //    setCentralWidget(ui->splitterMain);

    //状态栏初始化创建
    labCellIndex = new QLabel("当前单元格坐标：",this);
    labCellIndex->setMinimumWidth(220);

    labCellType=new QLabel("当前单元格类型：",this);
    labCellType->setMinimumWidth(150);

    labStudID=new QLabel("学生ID：",this);
    labStudID->setMinimumWidth(230);

    labBirthplace = new QLabel(tr("籍贯：未选择学生"), this);
    labBirthplace->setObjectName("labBirthplace");
    labStudID->setObjectName("labStudID");
    labBirthplace->setMinimumWidth(260);

    ui->statusBar->addWidget(labCellIndex); //添加到状态栏
    ui->statusBar->addWidget(labCellType);
    ui->statusBar->addWidget(labStudID);
    ui->statusBar->addWidget(labBirthplace, 1);
    on_btnSetHeader_clicked();
    ui->tableInfo->setRowCount(0);
    ui->tableInfo->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ui->tableInfo, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::refreshCurrentStudent);
    connect(ui->tableInfo, &QTableWidget::itemChanged,
            this, &MainWindow::refreshCurrentStudent);
}

MainWindow::~MainWindow()
{
    delete ui;
}


//设置水平表头
void MainWindow::on_btnSetHeader_clicked()
{
    QSignalBlocker blocker(ui->tableInfo);
    rosterMode = false;
    ui->tableInfo->clear(); //切回原示例时清除旧的七列表格，避免列含义混用
    QStringList headerText;
    headerText<<"姓名"<<"性别"<<"出生日期"<<"民族"<<"分数"<<"是否党员";
    setHeaders(headerText);
    refreshCurrentStudent();
}

void MainWindow::setHeaders(const QStringList &headerText)
{
    //    ui->tableInfo->setHorizontalHeaderLabels(headerText); //只设置标题
    ui->tableInfo->setColumnCount(headerText.size());      //设置表格列数
    for (int i=0;i<ui->tableInfo->columnCount();i++)
    {
        QTableWidgetItem *headerItem=new QTableWidgetItem(headerText.at(i));
        QFont font=headerItem->font();   //获取原有字体设置
        font.setBold(true);              //设置为粗体
        font.setPointSize(11);           //字体大小
        headerItem->setForeground(QBrush(Qt::red));  //设置文字颜色
        headerItem->setFont(font);       //设置字体
        ui->tableInfo->setHorizontalHeaderItem(i,headerItem);    //设置表头单元格的item
    }
}

//设置行数,设置的行数为数据区的行数，不含表头
void MainWindow::on_btnSetRows_clicked()
{
    ui->tableInfo->setRowCount(ui->spinRowCount->value());//设置数据区行数
    ui->tableInfo->setAlternatingRowColors(ui->chkBoxRowColor->isChecked()); //设置交替行背景颜色
    refreshCurrentStudent();
}


//初始化表格数据
void MainWindow::on_btnIniData_clicked()
{
    if (rosterMode) {
        on_actSetStudents_triggered();
        return;
    }
    QDate   birth(2001,4,6);        //初始化一个日期
    ui->tableInfo->clearContents(); //只清除工作区，不清除表头
    for (int i=0; i<ui->tableInfo->rowCount(); i++)
    {
        QString strName=QString("学生%1").arg(i);
        QString strSex= ((i % 2)==0)? "男":"女";
        bool isParty= ((i % 2)==0)? false:true;
        int score= QRandomGenerator::global()->bounded(60,100);   //随机数[60,100)
        createItemsARow(i, strName, strSex, birth,"汉族",isParty,score);  //为某一行创建items
        birth=birth.addDays(20);    //日期加20天
    }
}

void MainWindow::on_chkBoxTabEditable_clicked(bool checked)
{ //设置表格是否可编辑，以及进入编辑模式的方式
    if (checked)
        //双击或获取焦点后单击，进入编辑状态
        ui->tableInfo->setEditTriggers(QAbstractItemView::DoubleClicked
                                       | QAbstractItemView::SelectedClicked);
    else
        ui->tableInfo->setEditTriggers(QAbstractItemView::NoEditTriggers); //不允许编辑
}

void MainWindow::on_chkBoxHeaderH_clicked(bool checked)
{//是否显示水平表头
    ui->tableInfo->horizontalHeader()->setVisible(checked);
}

void MainWindow::on_chkBoxHeaderV_clicked(bool checked)
{//是否显示垂直表头
    ui->tableInfo->verticalHeader()->setVisible(checked);
}

void MainWindow::on_chkBoxRowColor_clicked(bool checked)
{ //行的底色交替采用不同颜色
    ui->tableInfo->setAlternatingRowColors(checked);
}

void MainWindow::on_rBtnSelectItem_clicked()
{//选择方式：单元格选择
    ui->tableInfo->setSelectionBehavior(QAbstractItemView::SelectItems);
}

void MainWindow::on_rBtnSelectRow_clicked()
{//选择方式：行选择
    ui->tableInfo->setSelectionBehavior(QAbstractItemView::SelectRows);
}


//将 QTableWidget的所有行的内容提取字符串，显示在QPlainTextEdit里
void MainWindow::on_btnReadToEdit_clicked()
{
    ui->textEdit->clear();
    for (int row = 0; row < ui->tableInfo->rowCount(); ++row) {
        QStringList cells;
        for (int col = 0; col < ui->tableInfo->columnCount(); ++col) {
            const auto *item = ui->tableInfo->item(row, col);
            if (!item) cells << QString(); //增加空行后也可以安全读取
            else if (!rosterMode && col == colPartyM)
                cells << (item->checkState() == Qt::Checked ? tr("党员") : tr("群众"));
            else cells << item->text();
        }
        ui->textEdit->appendPlainText(tr("第 %1 行： ").arg(row + 1) + cells.join("   "));
    }
}

//currentCellChanged()信号的槽函数，当前单元格发生变化时的响应
void MainWindow::on_tableInfo_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(previousRow);
    Q_UNUSED(previousColumn);

    Q_UNUSED(currentRow);
    Q_UNUSED(currentColumn);
    refreshCurrentStudent();
}

//插入一行
void MainWindow::on_btnInsertRow_clicked()
{
    int curRow=ui->tableInfo->currentRow();     //当前行号
    if (curRow < 0) curRow = ui->tableInfo->rowCount();
    ui->tableInfo->insertRow(curRow);           //插入一行，但不会自动为单元格创建item
    if (rosterMode) {
        createRosterRow(curRow, {"", tr("新学生"), "", "", "", "", ""}, tr("待补充"));
        ui->tableInfo->setCurrentCell(curRow, 1);
        return;
    }
    createItemsARow(curRow, "新学生", "男",
                    QDate::fromString("2002-10-1","yyyy-M-d"),"苗族",true,80 ); //为某一行创建items
}

//添加一行
void MainWindow::on_btnAppendRow_clicked()
{
    int curRow=ui->tableInfo->rowCount();       //当前行号
    ui->tableInfo->insertRow(curRow);           //在表格尾部添加一行
    if (rosterMode) {
        createRosterRow(curRow, {"", tr("新学生"), "", "", "", "", ""}, tr("待补充"));
        ui->tableInfo->setCurrentCell(curRow, 1);
        return;
    }
    createItemsARow(curRow, "新生", "女",
                    QDate::fromString("2002-6-5","yyyy-M-d"),"满族",false,76 ); //为某一行创建items
}

//删除当前行及其items
void MainWindow::on_btnDelCurRow_clicked()
{
    int curRow=ui->tableInfo->currentRow();     //当前行号
    ui->tableInfo->removeRow(curRow);           //删除当前行及其items
    refreshCurrentStudent();
}

void MainWindow::on_btnAutoHeght_clicked()
{
    ui->tableInfo->resizeRowsToContents();
}

void MainWindow::on_btnAutoWidth_clicked()
{
    ui->tableInfo->resizeColumnsToContents();
}
