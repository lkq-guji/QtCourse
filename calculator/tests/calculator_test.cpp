#include "calculatorengine.h"
#include "calculatorwindow.h"
#include <QtTest>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDir>
class CalculatorTest : public QObject
{
    Q_OBJECT
private slots:
    void operations_data()
    {
        QTest::addColumn<QStringList>("commands");
        QTest::addColumn<QString>("expected");
        const auto add = [](const char *name, const QStringList &commands, const QString &result) {
            QTest::newRow(name) << commands << result;
        };
        add("addition", {"1","2","+","3","="}, "15");
        add("subtract", {"7","-","9","="}, "-2");
        add("multiply", {"6","*","7","="}, "42");
        add("divide", {"8","/","4","="}, "2");
        add("decimal", {"0",".","1","+","0",".","2","="}, "0.3");
        add("duplicate-point", {"2",".",".","3"}, "2.3");
        add("replace-operator", {"8","+","*","2","="}, "16");
        add("sequential", {"2","+","3","*","4","="}, "20");
        add("repeat-equals", {"2","+","3","=","="}, "8");
        add("new-input-after-result", {"2","+","3","=","7"}, "7");
        add("continue-after-result", {"2","+","3","=","*","4","="}, "20");
        add("backspace", {"1","2","3","BS"}, "12");
        add("backspace-empty", {"SIGN","2","BS"}, "0");
        add("backspace-waiting", {"1","2","+","BS","3","="}, "15");
        add("clear-entry", {"8","+","9","CE","2","="}, "10");
        add("clear-all", {"8","+","9","C","2","*","3","="}, "6");
        add("divide-zero", {"8","/","0","="}, "不能除以零");
        add("recover-from-error", {"8","/","0","=","2","+","3","="}, "5");
        add("negative-right", {"5","*","SIGN","2","="}, "-10");
        add("missing-right", {"5","+","="}, "5");
    }
    void operations()
    {
        QFETCH(QStringList, commands);
        QFETCH(QString, expected);
        CalculatorEngine engine;
        for (const auto &command : commands) engine.input(command);
        QCOMPARE(engine.display(), expected);
    }
    void keyboardDecimal()
    {
        CalculatorWindow window;
        window.show();
        auto *display = window.findChild<QLineEdit *>("displayEdit");
        QTest::keyClicks(display, "2..3+1");
        QTest::keyClick(display, Qt::Key_Return);
        QCOMPARE(display->text(), QString("3.3"));
    }
    void keyboardBackspaceAndClear()
    {
        CalculatorWindow window;
        window.show();
        auto *display = window.findChild<QLineEdit *>("displayEdit");
        QTest::keyClicks(display, "123");
        QTest::keyClick(display, Qt::Key_Backspace);
        QCOMPARE(display->text(), QString("12"));
        QTest::keyClick(display, Qt::Key_Escape);
        QCOMPARE(display->text(), QString("0"));
    }
    void mixedInput()
    {
        CalculatorWindow window;
        window.show();
        auto *display = window.findChild<QLineEdit *>("displayEdit");
        QTest::mouseClick(window.findChild<QPushButton *>("btn8"), Qt::LeftButton);
        QTest::mouseClick(window.findChild<QPushButton *>("btnDivide"), Qt::LeftButton);
        QTest::keyClicks(display, "2");
        QTest::keyClick(display, Qt::Key_Return);
        QCOMPARE(display->text(), QString("4"));
    }
    void signedResultExpression()
    {
        CalculatorEngine engine;
        for (const auto &command : QStringList{"2","+","3","=","SIGN"}) engine.input(command);
        QCOMPARE(engine.display(), QString("-5"));
        QCOMPARE(engine.expression(), QString("−(5)"));
    }
};
QTEST_MAIN(CalculatorTest)
#include "calculator_test.moc"
