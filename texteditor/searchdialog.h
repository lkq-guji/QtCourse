#pragma once
#include <QDialog>
#include <QTextDocument>
#include <memory>
class CodeEditor;
namespace Ui { class SearchDialog; }
class SearchDialog : public QDialog
{
    Q_OBJECT
public:
    SearchDialog(CodeEditor *editor, bool replaceMode, QWidget *parent);
    ~SearchDialog();
    bool findNext();
    int replaceOne();
    int replaceAll();
    void setQuery(const QString &query);
private:
    std::unique_ptr<Ui::SearchDialog> ui;
    CodeEditor *editor;
    QTextDocument::FindFlags flags() const;
    void message(const QString &text, bool error = false);
};
