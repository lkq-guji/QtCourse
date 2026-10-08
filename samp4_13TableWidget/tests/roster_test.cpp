#include "mainwindow.h"
#include <QtTest>
#include <QAction>
#include <QTableWidget>
#include <QToolBar>
#include <QToolButton>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QRadioButton>

class RosterTest : public QObject
{
    Q_OBJECT
private slots:
    void rosterAndOriginalControls()
    {
        MainWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        auto *table = window.findChild<QTableWidget *>("tableInfo");
        auto *action = window.findChild<QAction *>("actSetStudents");
        auto *toolbar = window.findChild<QToolBar *>("mainToolBar");
        auto *label = window.findChild<QLabel *>("labBirthplace");
        auto *idLabel = window.findChild<QLabel *>("labStudID");
        QVERIFY(table && action && toolbar && label && idLabel);
        auto *button = qobject_cast<QToolButton *>(toolbar->widgetForAction(action));
        QVERIFY(button);
        QVERIFY(!action->icon().pixmap(24,24).isNull());
        QCOMPARE(table->columnCount(), 6);
        QCOMPARE(table->rowCount(), 0);
        QVERIFY(window.grab().save("before.png"));
        QTest::mouseClick(button, Qt::LeftButton);
        QCOMPARE(table->rowCount(), 5);
        QCOMPARE(table->columnCount(), 7);
        const QStringList headers = {"学号", "姓名", "性别", "行政班级", "院系", "专业", "修读性质"};
        const QStringList ids = {"2024414290219", "2024414290220", "2024414290221", "2024414290223", "2024414290224"};
        const QStringList names = {"林泳恩", "刘楚彤", "刘楷钦", "刘泽", "龙智森"};
        for (int col = 0; col < 7; ++col)
            QCOMPARE(table->horizontalHeaderItem(col)->text(), headers.at(col));
        for (int row = 0; row < 5; ++row) {
            const QStringList values = {ids.at(row),names.at(row),"男","2024软件2班","计算机科学与技术学院","软件工程","初修"};
            for (int col = 0; col < 7; ++col) {
                QVERIFY(table->item(row,col));
                QCOMPARE(table->item(row,col)->text(), values.at(col));
                const bool highlight = row == 2 && col < 2;
                QCOMPARE(table->item(row,col)->font().bold(), highlight);
                QCOMPARE(table->item(row,col)->foreground().color() == QColor(Qt::red), highlight);
            }
            QCOMPARE(table->item(row,1)->data(Qt::UserRole + 1).toString(), QString("广东东莞"));
            //点击专业列，仍然显示该行姓名 item 关联的籍贯。
            QTest::mouseClick(table->viewport(), Qt::LeftButton, Qt::NoModifier,
                             table->visualItemRect(table->item(row,5)).center());
            QVERIFY(label->text().contains(names.at(row)));
            QVERIFY(label->text().contains("广东东莞"));
            QVERIFY(idLabel->text().contains(ids.at(row)));
        }
        //测试数据仅存在于内存，证明状态栏确实读取 UserRole + 1。
        table->item(0,1)->setData(Qt::UserRole + 1, "测试籍贯");
        table->setCurrentCell(0,4);
        QVERIFY(label->text().contains("测试籍贯"));
        action->trigger();
        QCOMPARE(table->rowCount(), 5); //重复点击替换名单，不会追加重复行。
        table->setCurrentCell(0,1);
        table->clearSelection(); //取消选中，截图能清晰看到红色字体。
        QVERIFY(window.grab().save("after.png"));
        window.findChild<QRadioButton *>("rBtnSelectRow")->click();
        table->setCurrentCell(2,2);
        QVERIFY(label->text().contains("刘楷钦"));
        QVERIFY(label->text().contains("广东东莞"));
        QVERIFY(window.grab().save("selected-row.png"));

        auto click = [&](const char *name) { window.findChild<QPushButton *>(name)->click(); };
        click("btnReadToEdit");
        const auto text = window.findChild<QPlainTextEdit *>("textEdit")->toPlainText();
        QVERIFY(text.contains("龙智森"));
        QVERIFY(text.contains("初修"));
        QVERIFY(!text.contains("群众"));
        click("btnAppendRow");
        QCOMPARE(table->rowCount(), 6);
        click("btnInsertRow");
        QCOMPARE(table->rowCount(), 7);
        click("btnDelCurRow");
        QCOMPARE(table->rowCount(), 6);
        window.findChild<QSpinBox *>("spinRowCount")->setValue(8);
        click("btnSetRows");
        click("btnReadToEdit"); //空单元格不崩溃。
        click("btnIniData");
        QCOMPARE(table->rowCount(), 5);
        click("btnSetHeader");
        QCOMPARE(table->columnCount(), 6);
        click("btnIniData");
        click("btnReadToEdit");
        QVERIFY(window.findChild<QPlainTextEdit *>("textEdit")->toPlainText().contains("群众"));
        action->trigger();
        for (int row = 4; row >= 0; --row) {
            table->setCurrentCell(row,1);
            click("btnDelCurRow");
        }
        QCOMPARE(table->rowCount(), 0);
        QVERIFY(label->text().contains("未选择"));
        action->trigger();
        QCOMPARE(table->rowCount(), 5);
    }
};
QTEST_MAIN(RosterTest)
#include "roster_test.moc"
