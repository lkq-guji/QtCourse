#include "mainwindow.h"
#include "codeeditor.h"
#include "searchdialog.h"
#include "documentio.h"
#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QClipboard>
#include <QAction>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTextBlock>

class EditorTest : public QObject
{
    Q_OBJECT
    QAction *action(MainWindow &w, const char *name) { return w.findChild<QAction *>(QString("action") + name); }
    static bool setClipboard(const QString &text)
    {
        for (int attempt = 0; attempt < 6; ++attempt) {
            qApp->clipboard()->setText(text);QTest::qWait(30);
            if (qApp->clipboard()->text() == text) return true;
        }
        return false;
    }
    static void respond(QMessageBox::StandardButton answer)
    {
        QTimer::singleShot(40, [answer] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (box && box->button(answer)) box->button(answer)->click();
        });
    }
private slots:
    void initTestCase() { qApp->setStyle("Fusion");qApp->setFont(QFont("Microsoft YaHei UI",10));qApp->setQuitOnLastWindowClosed(false);QStandardPaths::setTestModeEnabled(true); }
    void cleanup()
    {
        for (QWidget *widget : qApp->topLevelWidgets())
            if (auto *w = qobject_cast<MainWindow *>(widget)) delete w;
    }
    void decoding_data()
    {
        QTest::addColumn<QByteArray>("bytes");QTest::addColumn<bool>("ok");QTest::addColumn<QString>("expected");
        QTest::newRow("utf8") << QString("中文 Qt\r\n第二行\r第三行").toUtf8() << true << QString("中文 Qt\n第二行\n第三行");
        QTest::newRow("bom-utf8") << QByteArray::fromHex("efbbbf") + QString("你好").toUtf8() << true << QString("你好");
        QTest::newRow("utf16le") << QByteArray::fromHex("fffe2d4e87650a00") << true << QString("中文\n");
        QTest::newRow("utf16be") << QByteArray::fromHex("feff4e2d6587") << true << QString("中文");
        QTest::newRow("empty") << QByteArray() << true << QString();
        QTest::newRow("invalid-utf8") << QByteArray::fromHex("ff80") << false << QString();
        QTest::newRow("truncated-utf8") << QByteArray::fromHex("e4b8") << false << QString();
        QTest::newRow("odd-utf16") << QByteArray::fromHex("fffe2d") << false << QString();
        QTest::newRow("binary") << QByteArray::fromHex("410042") << false << QString();
        QTest::newRow("utf32") << QByteArray::fromHex("fffe000041000000") << false << QString();
    }
    void decoding()
    {
        QFETCH(QByteArray, bytes);QFETCH(bool, ok);QFETCH(QString, expected);
        QString text = "sentinel", error;
        QCOMPARE(DocumentIO::decode(bytes, text, error), ok);
        if (ok) { QCOMPARE(text, expected);QVERIFY(error.isEmpty()); }
        else { QCOMPARE(text, QString("sentinel"));QVERIFY(!error.isEmpty()); }
    }
    void unicodeFileRoundTrip()
    {
        QTemporaryDir dir;QString error,text;const QString path = dir.filePath("中文 文件.txt");
        QVERIFY(DocumentIO::write(path, "你好\nQt 6\n", error));QVERIFY(DocumentIO::read(path, text, error));
        QCOMPARE(text, QString("你好\nQt 6\n"));QFile f(path);QVERIFY(f.open(QIODevice::ReadOnly));QCOMPARE(f.readAll(), text.toUtf8());
    }
    void failedWritePreservesExisting()
    {
        QTemporaryDir dir;QString error,text;const QString path = dir.filePath("original.txt");
        QVERIFY(DocumentIO::write(path, "original", error));
        QVERIFY(!DocumentIO::write(dir.path(), "changed", error));
        QVERIFY(DocumentIO::read(path, text, error));QCOMPARE(text, QString("original"));
    }
    void oversizedFileIsRejected()
    {
        QTemporaryDir dir;QFile f(dir.filePath("big.txt"));QVERIFY(f.open(QIODevice::WriteOnly));QVERIFY(f.resize(DocumentIO::MaxBytes + 1));f.close();
        QString text,error;QVERIFY(!DocumentIO::read(f.fileName(), text, error));QVERIFY(error.contains("20"));
    }
    void missingAndInvalidFileKeepDocument()
    {
        QTemporaryDir dir;MainWindow w(dir.path());QString error;
        const QString good = dir.filePath("good.txt"), bad = dir.filePath("bad.txt");
        QVERIFY(DocumentIO::write(good, "existing", error));QVERIFY(w.loadFile(good));
        QVERIFY(!w.loadFile(dir.filePath("missing.txt")));QCOMPARE(w.editor()->toPlainText(), QString("existing"));QCOMPARE(w.currentFile(), good);
        QFile f(bad);QVERIFY(f.open(QIODevice::WriteOnly));f.write(QByteArray::fromHex("ff00"));f.close();
        QVERIFY(!w.loadFile(bad));QCOMPARE(w.editor()->toPlainText(), QString("existing"));QVERIFY(!w.isWindowModified());
    }
    void saveAndSaveAsUpdateState()
    {
        QTemporaryDir dir;MainWindow w(dir.path());w.editor()->insertPlainText("draft");QVERIFY(w.isWindowModified());
        const QString one = dir.filePath("one.txt"), two = dir.filePath("two.txt");
        QVERIFY(w.saveTo(one));QVERIFY(!w.isWindowModified());QVERIFY(w.windowTitle().contains("one.txt"));
        w.editor()->insertPlainText("!");QVERIFY(w.saveTo(two));QCOMPARE(w.currentFile(), two);
        QString text,error;QVERIFY(DocumentIO::read(one, text, error));QCOMPARE(text, QString("draft"));
        QVERIFY(DocumentIO::read(two, text, error));QCOMPARE(text, QString("draft!"));
        w.editor()->insertPlainText("?");QVERIFY(!w.saveTo(dir.path()));QVERIFY(w.isWindowModified());QCOMPARE(w.currentFile(), two);
    }
    void closeCancelAndDiscard()
    {
        MainWindow w;w.setAttribute(Qt::WA_DeleteOnClose, false);w.show();w.editor()->insertPlainText("unsaved");
        respond(QMessageBox::Cancel);QVERIFY(!w.close());QVERIFY(w.isVisible());QVERIFY(w.isWindowModified());
        respond(QMessageBox::Discard);QVERIFY(w.close());QVERIFY(!w.isVisible());
    }
    void closeSavesNamedFile()
    {
        QTemporaryDir dir;MainWindow w(dir.path());w.setAttribute(Qt::WA_DeleteOnClose, false);QString error,text;
        const QString path = dir.filePath("close.txt");QVERIFY(w.saveTo(path));w.editor()->insertPlainText("saved on close");w.show();
        respond(QMessageBox::Save);QVERIFY(w.close());QVERIFY(DocumentIO::read(path, text, error));QCOMPARE(text, QString("saved on close"));
    }
    void saveFailurePreventsClose()
    {
        QTemporaryDir dir;MainWindow w(dir.path());w.setAttribute(Qt::WA_DeleteOnClose, false);const QString path = dir.filePath("blocked.txt");
        QVERIFY(w.saveTo(path));QVERIFY(QFile::remove(path));QVERIFY(QDir().mkdir(path));w.editor()->insertPlainText("keep me");w.show();
        QTimer clicks;connect(&clicks, &QTimer::timeout, [] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                const auto answer = box->windowTitle() == "尚未保存" ? QMessageBox::Save : QMessageBox::Ok;
                if (box->button(answer)) box->button(answer)->click();
            }
        });clicks.start(30);QVERIFY(!w.close());clicks.stop();QVERIFY(w.isVisible());QVERIFY(w.isWindowModified());
    }
    void actionStatesFollowSelectionAndClipboard()
    {
        MainWindow w;QVERIFY(setClipboard(""));
        QVERIFY(!action(w,"Copy")->isEnabled());QVERIFY(!action(w,"Cut")->isEnabled());QVERIFY(!action(w,"Paste")->isEnabled());
        w.editor()->insertPlainText("abc");QVERIFY(action(w,"Undo")->isEnabled());
        w.editor()->selectAll();QVERIFY(action(w,"Copy")->isEnabled());action(w,"Copy")->trigger();QTRY_COMPARE(qApp->clipboard()->text(),QString("abc"));
        action(w,"Cut")->trigger();QTRY_COMPARE(w.editor()->toPlainText(),QString());QVERIFY(!action(w,"Copy")->isEnabled());QVERIFY(action(w,"Paste")->isEnabled());
        action(w,"Paste")->trigger();QCOMPARE(w.editor()->toPlainText(),QString("abc"));
        action(w,"Undo")->trigger();QVERIFY(action(w,"Redo")->isEnabled());action(w,"Redo")->trigger();QCOMPARE(w.editor()->toPlainText(),QString("abc"));
        QVERIFY(setClipboard(""));QTRY_VERIFY(!action(w,"Paste")->isEnabled());
    }
    void searchDirectionCaseAndWrap()
    {
        MainWindow w;w.editor()->setPlainText("Qt qt Qt");w.editor()->moveCursor(QTextCursor::Start);
        SearchDialog d(w.editor(),false,&w);d.setQuery("Qt");d.findChild<QCheckBox *>("caseCheck")->setChecked(true);
        QVERIFY(d.findNext());QCOMPARE(w.editor()->textCursor().selectionStart(),0);
        QVERIFY(d.findNext());QCOMPARE(w.editor()->textCursor().selectionStart(),6);
        QVERIFY(d.findNext());QCOMPARE(w.editor()->textCursor().selectionStart(),0);
        QVERIFY(d.findChild<QLabel *>("resultLabel")->text().contains("循环"));
        d.findChild<QRadioButton *>("upRadio")->setChecked(true);QVERIFY(d.findNext());QCOMPARE(w.editor()->textCursor().selectionStart(),6);
        d.setQuery("absent");const auto old = w.editor()->textCursor();QVERIFY(!d.findNext());QCOMPARE(w.editor()->textCursor().position(),old.position());
        QVERIFY(d.findChild<QLabel *>("resultLabel")->text().contains("未找到"));
    }
    void emptySearchIsDisabled()
    {
        MainWindow w;SearchDialog d(w.editor(),true,&w);QVERIFY(!d.findChild<QPushButton *>("findButton")->isEnabled());
        QVERIFY(!d.findNext());QCOMPARE(d.replaceAll(),0);QCOMPARE(d.replaceOne(),0);
    }
    void replaceAll_data()
    {
        QTest::addColumn<QString>("text");QTest::addColumn<QString>("query");QTest::addColumn<QString>("replacement");QTest::addColumn<QString>("expected");QTest::addColumn<int>("count");
        QTest::newRow("expansion")<<"a a a"<<"a"<<"aa"<<"aa aa aa"<<3;
        QTest::newRow("delete")<<"hello hello"<<"hello"<<""<<" "<<2;
        QTest::newRow("identical")<<"Qt Qt"<<"Qt"<<"Qt"<<"Qt Qt"<<2;
        QTest::newRow("casefold")<<"Qt qt QT"<<"qt"<<"文本"<<"文本 文本 文本"<<3;
        QTest::newRow("not-found")<<"abc"<<"z"<<"X"<<"abc"<<0;
    }
    void replaceAll()
    {
        QFETCH(QString,text);QFETCH(QString,query);QFETCH(QString,replacement);QFETCH(QString,expected);QFETCH(int,count);
        MainWindow w;w.editor()->setPlainText(text);SearchDialog d(w.editor(),true,&w);d.setQuery(query);d.findChild<QLineEdit *>("replaceEdit")->setText(replacement);
        QCOMPARE(d.replaceAll(),count);QCOMPARE(w.editor()->toPlainText(),expected);
        if (count>0) { w.editor()->undo();QCOMPARE(w.editor()->toPlainText(),text); }
    }
    void replaceOneOnlyReplacesMatches()
    {
        MainWindow w;w.editor()->setPlainText("abc Qt Qt");w.editor()->moveCursor(QTextCursor::Start);
        SearchDialog d(w.editor(),true,&w);d.setQuery("Qt");d.findChild<QLineEdit *>("replaceEdit")->setText("Text");
        QCOMPARE(d.replaceOne(),0);QCOMPARE(w.editor()->toPlainText(),QString("abc Qt Qt"));
        QCOMPARE(d.replaceOne(),1);QCOMPARE(w.editor()->toPlainText(),QString("abc Text Qt"));
    }
    void viewOptionsAndUnicodeStatus()
    {
        MainWindow w;w.show();QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.editor()->setPlainText(QString::fromUtf8("中文🙂\n第二行"));w.editor()->moveCursor(QTextCursor::End);
        QVERIFY(w.findChild<QLabel *>("positionLabel")->text().contains("行：2"));QVERIFY(w.findChild<QLabel *>("positionLabel")->text().contains("列：4"));
        QVERIFY(w.findChild<QLabel *>("statsLabel")->text().contains("字符：7"));
        const int width=w.editor()->lineNumberAreaWidth();QVERIFY(width>0);action(w,"LineNumbers")->trigger();QCOMPARE(w.editor()->lineNumberAreaWidth(),0);
        action(w,"LineNumbers")->trigger();QCOMPARE(w.editor()->lineNumberAreaWidth(),width);
        action(w,"Wrap")->trigger();QCOMPARE(w.editor()->lineWrapMode(),QPlainTextEdit::NoWrap);
        action(w,"Toolbar")->trigger();QVERIFY(!w.findChild<QToolBar *>("fileToolbar")->isVisible());action(w,"Toolbar")->trigger();
        action(w,"Statusbar")->trigger();QVERIFY(!w.statusBar()->isVisible());action(w,"Statusbar")->trigger();QVERIFY(w.statusBar()->isVisible());
    }
    void displayFormattingDoesNotModifyText()
    {
        MainWindow w;w.editor()->setPlainText("显示格式测试");w.editor()->document()->setModified(false);
        w.editor()->setTextBackgroundColor(Qt::yellow);w.editor()->setEditorColors(Qt::blue,QColor("#fffdf2"));
        w.findChild<QSpinBox *>("fontSizeSpin")->setValue(18);
        QCOMPARE(w.editor()->toPlainText(),QString("显示格式测试"));QVERIFY(!w.editor()->document()->isModified());QVERIFY(w.editor()->lineNumberAreaWidth()>0);
    }
    void wrappingAndLineNumberDigits()
    {
        MainWindow w;w.show();const int one=w.editor()->lineNumberAreaWidth();
        QStringList lines;for(int i=0;i<120;++i)lines<<QString(200,'x');
        w.editor()->setPlainText(lines.join('\n'));QCOMPARE(w.editor()->blockCount(),120);
        QVERIFY(w.editor()->lineNumberAreaWidth()>one);
        action(w,"Wrap")->trigger();QCOMPARE(w.editor()->blockCount(),120);
        w.findChild<QSpinBox *>("fontSizeSpin")->setValue(24);QVERIFY(w.editor()->lineNumberAreaWidth()>one);
    }
    void backupTimerDoesNotOverwriteOriginal()
    {
        QTemporaryDir dir;MainWindow w(dir.filePath("recovery"));const QString path=dir.filePath("original.txt");QString text,error;
        w.editor()->insertPlainText("original");QVERIFY(w.saveTo(path));
        auto *timer=w.findChild<QTimer *>("autoSaveTimer");timer->setInterval(30);
        w.editor()->insertPlainText(" changed");QTRY_VERIFY(QFileInfo::exists(w.backupFile()));
        QVERIFY(DocumentIO::read(path,text,error));QCOMPARE(text,QString("original"));
        QVERIFY(DocumentIO::read(w.backupFile(),text,error));QCOMPARE(text,QString("original changed"));QVERIFY(w.isWindowModified());
        action(w,"AutoSave")->trigger();QVERIFY(!timer->isActive());
    }
    void independentWindowsAndDuplicatePaths()
    {
        MainWindow first;first.editor()->insertPlainText("first");action(first,"New")->trigger();QCoreApplication::processEvents();
        MainWindow *second=nullptr;for (auto *widget:qApp->topLevelWidgets()) if(auto *w=qobject_cast<MainWindow *>(widget);w && w!=&first) second=w;
        QVERIFY(second);second->editor()->insertPlainText("second");QCOMPARE(first.editor()->toPlainText(),QString("first"));
        QTemporaryDir dir;const QString path=dir.filePath("shared.txt");QVERIFY(first.saveTo(path));QVERIFY(!second->saveTo(path));
        second->editor()->document()->setModified(false);delete second;
    }
    void captureScreenshots()
    {
        MainWindow w;w.resize(1000,700);w.show();QVERIFY(QTest::qWaitForWindowExposed(&w));
        const QString sample="Qt 文本编辑实验\n第一行：支持中文与 English。\n第二行：Qt 查找和 Qt 替换。\n第三行：保存、撤销、行号与自动备份。\n";
        w.editor()->setPlainText(sample);w.editor()->document()->setModified(false);w.editor()->moveCursor(QTextCursor::Start);QTest::qWait(80);
        QVERIFY(w.grab().save("01-main.png"));
        QTimer::singleShot(100, [] { if(auto *d=qobject_cast<QDialog *>(QApplication::activeModalWidget())) {d->grab().save("02-about.png");d->accept();} });action(w,"About")->trigger();
        action(w,"Find")->trigger();auto *find=w.findChild<SearchDialog *>("findDialog");find->setQuery("Qt");find->findNext();QTest::qWait(60);QVERIFY(find->grab().save("03-find-design.png"));
        find->setQuery("不存在的内容");find->findNext();QVERIFY(find->grab().save("10-find-not-found.png"));find->hide();
        action(w,"Replace")->trigger();auto *replace=w.findChild<SearchDialog *>("replaceDialog");replace->setQuery("Qt");replace->findChild<QLineEdit *>("replaceEdit")->setText("文本编辑");
        QVERIFY(replace->grab().save("04-replace-design.png"));replace->replaceAll();QVERIFY(replace->grab().save("11-replace-result.png"));replace->hide();w.editor()->undo();
        const QString path=QDir::current().absoluteFilePath("录制示例.txt");QVERIFY(w.saveTo(path));QTest::qWait(60);QVERIFY(w.grab().save("05-file-save.png"));
        w.editor()->selectAll();action(w,"Copy")->trigger();QVERIFY(w.grab().save("06-edit.png"));
        w.editor()->moveCursor(QTextCursor::Start);w.editor()->setEditorColors(QColor("#174a83"),QColor("#fffdf2"));w.editor()->setTextBackgroundColor(QColor("#f7edba"));
        w.findChild<QSpinBox *>("fontSizeSpin")->setValue(15);QVERIFY(w.grab().save("07-format.png"));
        action(w,"Toolbar")->trigger();action(w,"Statusbar")->trigger();QVERIFY(w.grab().save("08-view.png"));action(w,"Toolbar")->trigger();action(w,"Statusbar")->trigger();
        w.editor()->setTextBackgroundColor({});w.editor()->setEditorColors(Qt::black,Qt::white);w.findChild<QSpinBox *>("fontSizeSpin")->setValue(12);
        QCoreApplication::processEvents();QVERIFY(w.grab().save("09-actions.png"));
        QStringList lines;for(int i=1;i<=120;++i)lines<<QString("第%1行：文本编辑、自动换行与行号显示。").arg(i);
        w.editor()->setPlainText(lines.join('\n'));w.editor()->moveCursor(QTextCursor::End);QTest::qWait(80);QVERIFY(w.grab().save("12-line-numbers.png"));
        w.editor()->document()->setModified(false);
        w.editor()->insertPlainText("未保存修改");
        w.setAttribute(Qt::WA_DeleteOnClose,false);
        QTimer::singleShot(100, [] { if(auto *box=qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {box->grab().save("13-close-warning.png");box->button(QMessageBox::Cancel)->click();} });
        QVERIFY(!w.close());
        QTimer::singleShot(100, [] { if(auto *box=qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {box->grab().save("14-open-error.png");box->button(QMessageBox::Ok)->click();} });
        w.openPath(QDir::current().absoluteFilePath("不存在的文件.txt"));
        QVERIFY(w.backupNow());QVERIFY(w.grab().save("15-backup.png"));w.editor()->document()->setModified(false);
    }
};
QTEST_MAIN(EditorTest)
#include "editor_test.moc"
