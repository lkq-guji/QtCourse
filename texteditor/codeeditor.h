#pragma once
#include <QPlainTextEdit>
#include <QColor>
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    int lineNumberAreaWidth() const;
    void setLineNumbersVisible(bool visible);
    bool lineNumbersVisible() const { return numbersVisible; }
    void paintLineNumbers(QPaintEvent *event);
    void setTextBackgroundColor(const QColor &color);
    void setEditorColors(const QColor &foreground, const QColor &background);
signals:
    void copyRequested(bool cut);
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;
private:
    QWidget *numberArea;
    bool numbersVisible = true;
    QColor textBackground;
    void updateMargins();
    void updateSelections();
};
