#include "qwmainwind.h"
#include <QtTest>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

class AboutTest : public QObject
{
    Q_OBJECT
private slots:
    void toolbarAndMenuOpenAbout()
    {
        QWMainWind window;
        window.show();
        auto *action = window.findChild<QAction *>("actAbout");
        auto *toolbar = window.findChild<QToolBar *>("mainToolBar");
        auto *menu = window.findChild<QMenu *>("menuHelp");
        auto *editor = window.findChild<QTextEdit *>("txtEdit");
        QVERIFY(action && toolbar && menu && editor);
        QVERIFY(menu->actions().contains(action));
        auto *button = qobject_cast<QToolButton *>(toolbar->widgetForAction(action));
        QVERIFY(button);
        QVERIFY(!action->icon().pixmap(24, 24).isNull());
        editor->setPlainText(QStringLiteral("保留编辑内容"));

        for (int entry = 0; entry < 2; ++entry) {
            bool dialogSeen = false;
            bool hasInfo = false;
            QTimer::singleShot(100, &window, [&] {
                auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
                if (!box) return;
                dialogSeen = true;
                hasInfo = box->windowTitle().contains("About")
                    && box->informativeText().contains(QStringLiteral("姓名："))
                    && box->informativeText().contains(QStringLiteral("学号："));
                if (entry == 0) {
                    window.grab().save("main-window.png");
                    box->grab().save("about-window.png");
                }
                QTest::keyClick(box, Qt::Key_Return);
            });
            // 防止对话框关闭失败时测试无限等待。
            QTimer watchdog;
            watchdog.setSingleShot(true);
            connect(&watchdog, &QTimer::timeout, &window, [&] {
                if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()))
                    box->reject();
            });
            watchdog.start(3000);
            if (entry == 0) button->click();
            else menu->actions().first()->trigger();
            QVERIFY(dialogSeen);
            QVERIFY(hasInfo);
            QCOMPARE(editor->toPlainText(), QStringLiteral("保留编辑内容"));
            QVERIFY(QApplication::activeModalWidget() == nullptr);
        }
    }
};

QTEST_MAIN(AboutTest)
#include "about_test.moc"
