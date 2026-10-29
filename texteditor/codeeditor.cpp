#include "codeeditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QResizeEvent>
// 行号只绘制可见的文本块；滚动时跟随 viewport 更新。
class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor) : QWidget(editor), owner(editor) { setObjectName("lineNumberArea"); }
    QSize sizeHint() const override { return {owner->lineNumberAreaWidth(), 0}; }
protected:
    void paintEvent(QPaintEvent *event) override { owner->paintLineNumbers(event); }
private:
    CodeEditor *owner;
};
CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent), numberArea(new LineNumberArea(this))
{
    setFont(QFont("Consolas", 12));
    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateMargins);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::updateSelections);
    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditor::updateSelections);
    connect(this, &QPlainTextEdit::updateRequest, this, [this](const QRect &rect, int dy) {
        if (dy) numberArea->scroll(0, dy);
        else numberArea->update(0, rect.y(), numberArea->width(), rect.height());
        if (rect.contains(viewport()->rect())) updateMargins();
    });
    updateMargins(); updateSelections();
}
int CodeEditor::lineNumberAreaWidth() const
{
    if (!numbersVisible) return 0;
    return 14 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * QString::number(blockCount()).size();
}
void CodeEditor::updateMargins()
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    const QRect r = contentsRect();
    numberArea->setGeometry(r.left(), r.top(), lineNumberAreaWidth(), r.height());
    numberArea->setVisible(numbersVisible);
    numberArea->update();
}
void CodeEditor::setLineNumbersVisible(bool visible) { numbersVisible = visible; updateMargins(); }
void CodeEditor::resizeEvent(QResizeEvent *event) { QPlainTextEdit::resizeEvent(event); updateMargins(); }
void CodeEditor::changeEvent(QEvent *event)
{
    QPlainTextEdit::changeEvent(event);
    if (event->type() == QEvent::FontChange) updateMargins();
}
void CodeEditor::paintLineNumbers(QPaintEvent *event)
{
    QPainter p(numberArea);p.fillRect(event->rect(), QColor("#eef1f5"));
    p.setFont(font());p.setPen(QColor("#66788a"));
    QTextBlock block = firstVisibleBlock();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    while (block.isValid() && top <= event->rect().bottom()) {
        const int height = qRound(blockBoundingRect(block).height());
        if (block.isVisible() && top + height >= event->rect().top())
            p.drawText(0, top, numberArea->width() - 6, fontMetrics().height(), Qt::AlignRight,
                       QString::number(block.blockNumber() + 1));
        top += height;block = block.next();
    }
}
void CodeEditor::updateSelections()
{
    QList<QTextEdit::ExtraSelection> selections;
    QTextEdit::ExtraSelection line;
    line.cursor = textCursor();line.cursor.clearSelection();
    line.format.setBackground(QColor("#eef6ff"));
    line.format.setProperty(QTextFormat::FullWidthSelection, true);selections << line;
    // ExtraSelection 仅影响显示，不给 TXT 文档写入格式或改变修改状态。
    if (textBackground.isValid()) {
        QTextEdit::ExtraSelection all;
        all.cursor = QTextCursor(document());all.cursor.select(QTextCursor::Document);
        all.format.setBackground(textBackground);selections << all;
    }
    setExtraSelections(selections);
}
void CodeEditor::setTextBackgroundColor(const QColor &color) { textBackground = color;updateSelections(); }
void CodeEditor::setEditorColors(const QColor &foreground, const QColor &background)
{
    QPalette p = palette();p.setColor(QPalette::Text, foreground);p.setColor(QPalette::Base, background);setPalette(p);
}
