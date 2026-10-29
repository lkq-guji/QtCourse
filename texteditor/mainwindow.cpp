#include "mainwindow.h"
#include "codeeditor.h"
#include "documentio.h"
#include "searchdialog.h"
#include "ui_mainwindow.h"
#include "ui_aboutdialog.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFontDialog>
#include <QColorDialog>
#include <QLabel>
#include <QSpinBox>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>
#include <QTextBlock>

MainWindow::MainWindow(const QString &backupRoot, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);setAttribute(Qt::WA_DeleteOnClose);setMinimumSize(860, 560);
    statsLabel = new QLabel(this);statsLabel->setObjectName("statsLabel");
    positionLabel = new QLabel(this);positionLabel->setObjectName("positionLabel");
    encodingLabel = new QLabel("UTF-8 · TXT", this);
    ui->statusbar->addWidget(statsLabel, 1);ui->statusbar->addPermanentWidget(positionLabel);
    ui->statusbar->addPermanentWidget(encodingLabel);
    ui->statusbar->addPermanentWidget(new QLabel("刘楷钦 · 2024414290221", this));
    auto *sizeSpin = new QSpinBox(this);sizeSpin->setObjectName("fontSizeSpin");
    sizeSpin->setRange(8, 72);sizeSpin->setValue(12);sizeSpin->setSuffix(" 磅");
    ui->formatToolbar->addWidget(sizeSpin);
    connect(sizeSpin, &QSpinBox::valueChanged, this, [this](int size) { QFont f = editor()->font();f.setPointSize(size);editor()->setFont(f); });
    connect(ui->actionNew, &QAction::triggered, this, [] { (new MainWindow)->show(); });
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openDialog);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionClose, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionExit, &QAction::triggered, qApp, &QApplication::closeAllWindows);
    connect(ui->actionUndo, &QAction::triggered, editor(), &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, editor(), &QPlainTextEdit::redo);
    connect(ui->actionCut, &QAction::triggered, editor(), &QPlainTextEdit::cut);
    connect(ui->actionCopy, &QAction::triggered, editor(), &QPlainTextEdit::copy);
    connect(ui->actionPaste, &QAction::triggered, editor(), &QPlainTextEdit::paste);
    connect(ui->actionSelectAll, &QAction::triggered, editor(), &QPlainTextEdit::selectAll);
    connect(ui->actionFind, &QAction::triggered, this, [this] { showSearch(false); });
    connect(ui->actionReplace, &QAction::triggered, this, [this] { showSearch(true); });
    connect(ui->actionFindNext, &QAction::triggered, this, [this] { showSearch(false);findDialog->findNext(); });
    connect(ui->actionFindPrevious, &QAction::triggered, this, [this] {
        showSearch(false);findDialog->findChild<QRadioButton *>("upRadio")->setChecked(true);findDialog->findNext();
        findDialog->findChild<QRadioButton *>("downRadio")->setChecked(true);
    });
    connect(ui->actionWrap, &QAction::toggled, this, [this](bool on) { editor()->setLineWrapMode(on ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap); });
    connect(ui->actionLineNumbers, &QAction::toggled, editor(), &CodeEditor::setLineNumbersVisible);
    connect(ui->actionToolbar, &QAction::toggled, this, [this](bool on) { ui->fileToolbar->setVisible(on);ui->formatToolbar->setVisible(on); });
    connect(ui->actionStatusbar, &QAction::toggled, ui->statusbar, &QWidget::setVisible);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    connect(ui->actionFont, &QAction::triggered, this, [this, sizeSpin] {
        bool ok = false;const QFont font = QFontDialog::getFont(&ok, editor()->font(), this, "选择字体");
        if (ok) { editor()->setFont(font);QSignalBlocker block(sizeSpin);sizeSpin->setValue(font.pointSize()); }
    });
    connect(ui->actionFontColor, &QAction::triggered, this, [this] {
        const QColor color = QColorDialog::getColor(editor()->palette().color(QPalette::Text), this, "字体颜色");
        if (color.isValid()) editor()->setEditorColors(color, editor()->palette().color(QPalette::Base));
    });
    connect(ui->actionBackground, &QAction::triggered, this, [this] {
        const QColor color = QColorDialog::getColor(editor()->palette().color(QPalette::Base), this, "编辑器背景色");
        if (color.isValid()) editor()->setEditorColors(editor()->palette().color(QPalette::Text), color);
    });
    connect(ui->actionTextBackground, &QAction::triggered, this, [this] {
        const QColor color = QColorDialog::getColor(Qt::yellow, this, "文字底色");
        if (color.isValid()) editor()->setTextBackgroundColor(color);
    });
    connect(editor(), &QPlainTextEdit::textChanged, this, &MainWindow::updateState);
    connect(editor(), &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateState);
    connect(editor(), &QPlainTextEdit::selectionChanged, this, &MainWindow::updateState);
    connect(editor(), &QPlainTextEdit::undoAvailable, this, &MainWindow::updateState);
    connect(editor(), &QPlainTextEdit::redoAvailable, this, &MainWindow::updateState);
    connect(editor()->document(), &QTextDocument::modificationChanged, this, &MainWindow::updateState);
    connect(qApp->clipboard(), &QClipboard::dataChanged, this, &MainWindow::updateState);
    const QString folder = backupRoot.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/recovery" : backupRoot;
    recoveryPath = folder + "/backup-" + QUuid::createUuid().toString(QUuid::WithoutBraces) + ".txt";
    autoSaveTimer = new QTimer(this);autoSaveTimer->setObjectName("autoSaveTimer");autoSaveTimer->setInterval(60000);
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::backupNow);autoSaveTimer->start();
    connect(ui->actionAutoSave, &QAction::toggled, this, [this](bool on) { if (on) autoSaveTimer->start();else autoSaveTimer->stop(); });
    connect(ui->actionRecovery, &QAction::triggered, this, &MainWindow::recover);
    updateState();
}
MainWindow::~MainWindow() = default;
CodeEditor *MainWindow::editor() const { return ui->editor; }
void MainWindow::updateState()
{
    const bool selected = editor()->textCursor().hasSelection();
    ui->actionCopy->setEnabled(selected);ui->actionCut->setEnabled(selected && !editor()->isReadOnly());
    ui->actionPaste->setEnabled(editor()->canPaste() && !editor()->isReadOnly());
    ui->actionUndo->setEnabled(editor()->document()->isUndoAvailable());ui->actionRedo->setEnabled(editor()->document()->isRedoAvailable());
    ui->actionSelectAll->setEnabled(!editor()->document()->isEmpty());
    ui->actionSave->setEnabled(editor()->document()->isModified() || filePath.isEmpty());
    setWindowTitle((filePath.isEmpty() ? "未命名文档" : QFileInfo(filePath).fileName()) + "[*] — 简单文本编辑器");
    setWindowModified(editor()->document()->isModified());
    const auto cursor = editor()->textCursor();
    statsLabel->setText(QString("字符：%1   行数：%2").arg(editor()->toPlainText().toUcs4().size()).arg(editor()->blockCount()));
    const int column = cursor.block().text().left(cursor.positionInBlock()).toUcs4().size() + 1;
    positionLabel->setText(QString("行：%1   列：%2  ").arg(cursor.blockNumber() + 1).arg(column));
}
bool MainWindow::loadFile(const QString &path)
{
    if (editor()->document()->isModified()) { errorText = "当前窗口有未保存内容，请使用新窗口打开。";return false; }
    QString text;
    // 先完整读取和验证，再改变编辑器；失败时保留已有内容和文件路径。
    if (!DocumentIO::read(path, text, errorText)) return false;
    editor()->setPlainText(text);filePath = QFileInfo(path).absoluteFilePath();
    editor()->document()->setModified(false);updateState();return true;
}
bool MainWindow::saveTo(const QString &path)
{
    const QString absolute = QFileInfo(path).absoluteFilePath();
    for (QWidget *widget : qApp->topLevelWidgets()) {
        auto *other = qobject_cast<MainWindow *>(widget);
        if (other && other != this && !other->currentFile().isEmpty() && QFileInfo(other->currentFile()) == QFileInfo(absolute)) {
            errorText = "该文件正在另一个编辑窗口中打开。";return false;
        }
    }
    if (!DocumentIO::write(path, editor()->toPlainText(), errorText)) return false;
    filePath = absolute;editor()->document()->setModified(false);updateState();
    ui->statusbar->showMessage("已保存：" + filePath, 4000);return true;
}
bool MainWindow::saveAs()
{
    QFileDialog dialog(this, "另存为", filePath.isEmpty() ? "未命名.txt" : filePath, "文本文件 (*.txt);;所有文件 (*)");
    dialog.setAcceptMode(QFileDialog::AcceptSave);dialog.setDefaultSuffix("txt");
    if (dialog.exec() != QDialog::Accepted) return false;
    if (saveTo(dialog.selectedFiles().first())) return true;
    QMessageBox::warning(this, "保存失败", errorText);return false;
}
bool MainWindow::save()
{
    if (filePath.isEmpty()) return saveAs();
    if (saveTo(filePath)) return true;
    QMessageBox::warning(this, "保存失败", errorText);return false;
}
bool MainWindow::maybeSave()
{
    if (!editor()->document()->isModified()) return true;
    const auto answer = QMessageBox::warning(this, "尚未保存", "文档已修改，是否保存后关闭？",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (answer == QMessageBox::Save) return save();
    return answer == QMessageBox::Discard;
}
void MainWindow::closeEvent(QCloseEvent *event) { if (maybeSave()) event->accept();else event->ignore(); }
void MainWindow::openDialog()
{
    const QString path = QFileDialog::getOpenFileName(this, "打开文本文件", {}, "文本文件 (*.txt);;所有文件 (*)");
    if (!path.isEmpty()) openPath(path);
}
void MainWindow::openPath(const QString &path)
{
    for (QWidget *widget : qApp->topLevelWidgets()) {
        auto *other = qobject_cast<MainWindow *>(widget);
        if (other && !other->currentFile().isEmpty() && QFileInfo(other->currentFile()) == QFileInfo(path)) { other->showNormal();other->raise();other->activateWindow();return; }
    }
    MainWindow *target = this;
    if (!filePath.isEmpty() || editor()->document()->isModified() || !editor()->document()->isEmpty()) target = new MainWindow;
    if (!target->loadFile(path)) {
        const QString message = target->lastError();if (target != this) delete target;
        QMessageBox::warning(this, "打开失败", message);return;
    }
    target->show();target->raise();target->activateWindow();
}
void MainWindow::showSearch(bool replaceMode)
{
    SearchDialog *&dialog = replaceMode ? replaceDialog : findDialog;
    if (!dialog) dialog = new SearchDialog(editor(), replaceMode, this);
    if (editor()->textCursor().hasSelection() && !dialog->isVisible()) dialog->setQuery(editor()->textCursor().selectedText());
    dialog->show();dialog->raise();dialog->activateWindow();dialog->findChild<QLineEdit *>("findEdit")->setFocus();
}
void MainWindow::showAbout()
{
    QDialog dialog(this);dialog.setObjectName("aboutDialog");Ui::AboutDialog about;about.setupUi(&dialog);
    about.titleLabel->setStyleSheet("font-size:24px;font-weight:600;color:#244e72");
    about.versionLabel->setText(QString("Qt %1 · UTF-8 纯文本编辑器").arg(qVersion()));
    connect(about.okButton, &QPushButton::clicked, &dialog, &QDialog::accept);dialog.exec();
}
bool MainWindow::backupNow()
{
    if (!editor()->document()->isModified()) return true;
    if (!QDir().mkpath(QFileInfo(recoveryPath).absolutePath()) || !DocumentIO::write(recoveryPath, editor()->toPlainText(), errorText)) {
        ui->statusbar->showMessage("自动备份失败，请手动保存。", 5000);return false;
    }
    ui->statusbar->showMessage("恢复备份已保存，原文件尚未覆盖。", 4000);return true;
}
void MainWindow::recover()
{
    const QString path = QFileDialog::getOpenFileName(this, "恢复自动备份（恢复后请另存）", QFileInfo(recoveryPath).absolutePath(), "恢复备份 (*.txt)");
    if (path.isEmpty()) return;
    QString text;
    if (!DocumentIO::read(path, text, errorText)) { QMessageBox::warning(this, "恢复失败", errorText);return; }
    auto *window = new MainWindow;window->editor()->setPlainText(text);window->editor()->document()->setModified(true);window->show();
}
