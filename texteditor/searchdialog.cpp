#include "searchdialog.h"
#include "codeeditor.h"
#include "ui_searchdialog.h"
#include <QTextCursor>
SearchDialog::SearchDialog(CodeEditor *editor, bool replaceMode, QWidget *parent)
    : QDialog(parent), ui(new Ui::SearchDialog), editor(editor)
{
    ui->setupUi(this);setModal(false);
    setObjectName(replaceMode ? "replaceDialog" : "findDialog");
    setWindowTitle(replaceMode ? "替换" : "查找");
    ui->replaceEdit->setVisible(replaceMode);ui->replaceLabel->setVisible(replaceMode);
    ui->replaceButton->setVisible(replaceMode);ui->allButton->setVisible(replaceMode);
    connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::hide);
    connect(ui->findButton, &QPushButton::clicked, this, &SearchDialog::findNext);
    connect(ui->replaceButton, &QPushButton::clicked, this, &SearchDialog::replaceOne);
    connect(ui->allButton, &QPushButton::clicked, this, &SearchDialog::replaceAll);
    connect(ui->findEdit, &QLineEdit::returnPressed, this, &SearchDialog::findNext);
    const auto update = [this] {
        const bool ready = !ui->findEdit->text().isEmpty();
        ui->findButton->setEnabled(ready);ui->replaceButton->setEnabled(ready);ui->allButton->setEnabled(ready);
    };
    connect(ui->findEdit, &QLineEdit::textChanged, this, update);update();
    ui->findButton->setDefault(true);
}
SearchDialog::~SearchDialog() = default;
void SearchDialog::setQuery(const QString &query) { ui->findEdit->setText(query);ui->findEdit->selectAll(); }
QTextDocument::FindFlags SearchDialog::flags() const
{
    QTextDocument::FindFlags value;
    if (ui->caseCheck->isChecked()) value |= QTextDocument::FindCaseSensitively;
    if (ui->upRadio->isChecked()) value |= QTextDocument::FindBackward;
    return value;
}
void SearchDialog::message(const QString &text, bool error)
{
    ui->resultLabel->setText(text);ui->resultLabel->setStyleSheet(error ? "color:#b42318" : "color:#315b7d");
}
bool SearchDialog::findNext()
{
    const QString query = ui->findEdit->text();
    if (query.isEmpty()) { message("请输入查找内容。", true);return false; }
    QTextCursor hit = editor->document()->find(query, editor->textCursor(), flags());
    bool wrapped = false;
    if (hit.isNull() && ui->wrapCheck->isChecked()) {
        QTextCursor start(editor->document());
        start.movePosition(ui->upRadio->isChecked() ? QTextCursor::End : QTextCursor::Start);
        hit = editor->document()->find(query, start, flags());wrapped = !hit.isNull();
    }
    if (hit.isNull()) { message("未找到：" + query, true);return false; }
    editor->setTextCursor(hit);editor->ensureCursorVisible();
    message(wrapped ? "已循环到文档另一端。" : "已找到匹配内容。");return true;
}
int SearchDialog::replaceOne()
{
    const QString query = ui->findEdit->text();
    if (query.isEmpty()) { message("请输入查找内容。", true);return 0; }
    auto cursor = editor->textCursor();
    const auto cs = ui->caseCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    if (!cursor.hasSelection() || cursor.selectedText().compare(query, cs) != 0) {
        if (findNext()) message("已定位匹配，点击“替换”执行。");return 0;
    }
    cursor.insertText(ui->replaceEdit->text());editor->setTextCursor(cursor);
    findNext();message("已替换1处。");return 1;
}
int SearchDialog::replaceAll()
{
    const QString query = ui->findEdit->text();
    if (query.isEmpty()) { message("请输入查找内容。", true);return 0; }
    QList<QTextCursor> matches;
    QTextCursor cursor(editor->document());
    // 先收集原文匹配，再倒序修改；替换文本包含查找词时也不会重复处理。
    const auto forwardFlags = flags() & ~QTextDocument::FindBackward;
    while (true) {
        cursor = editor->document()->find(query, cursor, forwardFlags);
        if (cursor.isNull()) break;
        matches.append(cursor);
    }
    if (matches.isEmpty()) { message("未找到：" + query, true);return 0; }
    QTextCursor group(editor->document());group.beginEditBlock();
    for (auto it = matches.rbegin(); it != matches.rend(); ++it) it->insertText(ui->replaceEdit->text());
    group.endEditBlock();message(QString("已替换%1处，可一次撤销。").arg(matches.size()));return matches.size();
}
