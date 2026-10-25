#include "calculatorengine.h"
#include "calculatorwindow.h"
#include <QtTest>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDir>
#include <QGridLayout>
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
        add("reciprocal", {"4","RECIP"}, "0.25");
        add("reciprocal-negative", {"4","SIGN","RECIP"}, "-0.25");
        add("reciprocal-zero", {"0","RECIP"}, "不能除以零");
        add("square", {"1","2","SQUARE"}, "144");
        add("square-negative", {"3","SIGN","SQUARE"}, "9");
        add("square-root", {"8","1","SQRT"}, "9");
        add("root-zero", {"0","SQRT"}, "0");
        add("root-negative", {"9","SIGN","SQRT"}, "负数不能开平方");
        add("percent-alone", {"5","0","%"}, "0.5");
        add("percent-add", {"2","0","0","+","1","0","%","="}, "220");
        add("percent-subtract", {"2","0","0","-","1","0","%","="}, "180");
        add("percent-multiply", {"2","0","0","*","1","0","%","="}, "20");
        add("percent-divide", {"2","0","0","/","1","0","%","="}, "2000");
        add("percent-repeat-equals", {"2","0","0","+","1","0","%","=","="}, "240");
        add("unary-right-operand", {"5","+","9","SQRT","="}, "8");
        add("unary-right-then-operator", {"5","+","3","SQUARE","*","2","="}, "28");
        add("unary-result-continue", {"2","+","3","=","SQUARE","+","1","="}, "26");
        add("unary-result-new-input", {"3","SQUARE","2"}, "2");
        add("unary-right-replace-input", {"5","+","9","SQRT","2","="}, "7");
        add("unary-right-clear-entry", {"5","+","9","SQRT","CE","2","="}, "7");
        add("unary-right-backspace", {"5","+","9","SQRT","BS","="}, "8");
        add("unary-after-waiting", {"5","+","SQUARE","="}, "30");
        add("unary-clears-repeat", {"2","+","3","=","SQUARE","="}, "25");
        add("chain-unary", {"4","SQUARE","SQRT","RECIP"}, "0.25");
        add("unary-error-recovery", {"9","SIGN","SQRT","2","+","3","="}, "5");
        add("reciprocal-error-ce", {"0","RECIP","CE","4","RECIP"}, "0.25");
        add("square-overflow", {"9","SQUARE","SQUARE","SQUARE","SQUARE","SQUARE",
                                 "SQUARE","SQUARE","SQUARE","SQUARE"}, "结果超出范围");
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
    void keyboardAfterMouseFocus()
    {
        CalculatorWindow window;
        window.show();
        auto *button = window.findChild<QPushButton *>("btn2");
        auto *display = window.findChild<QLineEdit *>("displayEdit");
        QTest::mouseClick(button, Qt::LeftButton);
        QTest::keyClicks(button, "+3");
        QTest::keyClick(button, Qt::Key_Return);
        QCOMPARE(display->text(), QString("5"));
        QTest::keyClick(button, Qt::Key_Enter, Qt::KeypadModifier);
        QCOMPARE(display->text(), QString("8"));
    }
    void extendedButtonsAndKeyboard_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("buttonName");
        QTest::addColumn<QString>("shortcut");
        QTest::addColumn<QString>("expected");
        QTest::newRow("percent") << "50" << "btnPercent" << "%" << "0.5";
        QTest::newRow("reciprocal") << "4" << "btnReciprocal" << "r" << "0.25";
        QTest::newRow("square") << "12" << "btnSquare" << "q" << "144";
        QTest::newRow("square-root") << "81" << "btnSqrt" << "s" << "9";
    }
    void extendedButtonsAndKeyboard()
    {
        QFETCH(QString, input);
        QFETCH(QString, buttonName);
        QFETCH(QString, shortcut);
        QFETCH(QString, expected);
        CalculatorWindow window;
        window.show();
        auto *display = window.findChild<QLineEdit *>("displayEdit");
        auto *button = window.findChild<QPushButton *>(buttonName);
        QVERIFY(button);
        QTest::keyClicks(display, input);
        QTest::mouseClick(button, Qt::LeftButton);
        QCOMPARE(display->text(), expected);
        QTest::keyClick(display, Qt::Key_Escape);
        QTest::keyClicks(display, input + shortcut);
        QCOMPARE(display->text(), expected);
    }
    void percentageWithMouseAndKeyboard()
    {
        CalculatorWindow window;
        window.show();
        auto *display = window.findChild<QLineEdit *>("displayEdit");
        QTest::keyClicks(display, "200+10");
        QTest::mouseClick(window.findChild<QPushButton *>("btnPercent"), Qt::LeftButton);
        QCOMPARE(display->text(), QString("20"));
        QTest::keyClick(display, Qt::Key_Return);
        QCOMPARE(display->text(), QString("220"));
        QTest::keyClick(display, Qt::Key_Escape);
        QTest::keyClicks(display, "9");
        QTest::mouseClick(window.findChild<QPushButton *>("btnSign"), Qt::LeftButton);
        QTest::mouseClick(window.findChild<QPushButton *>("btnSqrt"), Qt::LeftButton);
        QCOMPARE(display->text(), QString("负数不能开平方"));
        QVERIFY(display->property("error").toBool());
        QTest::keyClicks(display, "4r");
        QCOMPARE(display->text(), QString("0.25"));
        QVERIFY(!display->property("error").toBool());
    }
    void layoutMatchesReference()
    {
        CalculatorWindow window;
        window.resize(360, 600);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        auto *grid = window.findChild<QGridLayout *>("buttonGrid");
        QVERIFY(grid);
        QCOMPARE(grid->count(), 24);
        const QStringList expected = {
            "btnPercent", "btnClearEntry", "btnClearAll", "btnBackspace",
            "btnReciprocal", "btnSquare", "btnSqrt", "btnDivide",
            "btn7", "btn8", "btn9", "btnMultiply",
            "btn4", "btn5", "btn6", "btnSubtract",
            "btn1", "btn2", "btn3", "btnAdd",
            "btnSign", "btn0", "btnPoint", "btnEquals"};
        for (int i = 0; i < expected.size(); ++i) {
            auto *item = grid->itemAtPosition(i / 4, i % 4);
            QVERIFY(item && item->widget());
            QCOMPARE(item->widget()->objectName(), expected[i]);
            QVERIFY(window.rect().contains(item->widget()->geometry()));
            for (int j = 0; j < i; ++j)
                QVERIFY2(!item->widget()->geometry().intersects(
                    grid->itemAtPosition(j / 4, j % 4)->widget()->geometry()),
                    qPrintable(QString("%1 overlaps %2 at %3x%4")
                        .arg(expected[i], expected[j]).arg(window.width()).arg(window.height())));
        }
    }
    void captureScreenshots()
    {
        CalculatorWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        auto *display = window.findChild<QLineEdit *>("displayEdit");
        QTest::keyClicks(display, "12.5+3.75");
        QTest::keyClick(display, Qt::Key_Return);
        QTest::qWait(80);
        QVERIFY(window.grab().save("calculation.png"));
        const QString styled = window.styleSheet();
        window.setStyleSheet("");
        QTest::qWait(80);
        QVERIFY(window.grab().save("before-style.png"));
        window.setStyleSheet(styled);
        QTest::keyClick(display, Qt::Key_Escape);
        QTest::keyClicks(display, "8/0");
        QTest::keyClick(display, Qt::Key_Return);
        QTest::qWait(80);
        QVERIFY(window.grab().save("division-error.png"));
        QTest::keyClick(display, Qt::Key_Escape);
        QTest::keyClicks(display, "200+10%");
        QTest::keyClick(display, Qt::Key_Return);
        QTest::qWait(80);
        QVERIFY(window.grab().save("percentage.png"));
        QTest::keyClick(display, Qt::Key_Escape);
        QTest::keyClicks(display, "81s");
        QTest::qWait(80);
        QVERIFY(window.grab().save("square-root.png"));
        QTest::keyClick(display, Qt::Key_Escape);
        QTest::keyClicks(display, "9");
        QTest::keyClick(display, Qt::Key_F9);
        QTest::keyClicks(display, "s");
        QTest::qWait(80);
        QVERIFY(window.grab().save("negative-root.png"));
    }
};
QTEST_MAIN(CalculatorTest)
#include "calculator_test.moc"
