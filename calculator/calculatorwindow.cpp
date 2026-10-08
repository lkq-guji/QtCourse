#include "calculatorwindow.h"
#include "ui_calculatorwindow.h"
#include <QPushButton>
#include <QApplication>
#include <QKeyEvent>
#include <QStyle>
CalculatorWindow::CalculatorWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::CalculatorWindow)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);
    for (auto *button : findChildren<QPushButton *>()) {
        button->setFocusPolicy(Qt::NoFocus);
        connect(button, &QPushButton::clicked, this, [this, button] {
            dispatch(button->property("command").toString());
        });
    }
}
bool CalculatorWindow::eventFilter(QObject *watched, QEvent *event)
{
    auto *widget = qobject_cast<QWidget *>(watched);
    if (!widget || (widget != this && !isAncestorOf(widget))) return QWidget::eventFilter(watched, event);
    if (event->type() != QEvent::KeyPress && event->type() != QEvent::ShortcutOverride)
        return QWidget::eventFilter(watched, event);
    auto *key = static_cast<QKeyEvent *>(event);
    if (key->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) return false;
    QString command;
    if (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter || key->text() == "=") command = "=";
    else if (key->key() == Qt::Key_Backspace) command = "BS";
    else if (key->key() == Qt::Key_Escape || key->key() == Qt::Key_C) command = "C";
    else if (key->key() == Qt::Key_Delete) command = "CE";
    else if (key->key() == Qt::Key_F9) command = "SIGN";
    else if (key->text() == "," || key->text() == ".") command = ".";
    else if (key->text().size() == 1 && QString("0123456789+-*/").contains(key->text())) command = key->text();
    if (command.isEmpty()) return false;
    key->accept();
    // ShortcutOverride 只声明接管按键；KeyPress 才执行，避免一次按键计算两次。
    if (event->type() == QEvent::KeyPress) dispatch(command);
    return true;
}
void CalculatorWindow::dispatch(const QString &command)
{
    engine.input(command);
    ui->displayEdit->setText(engine.display());
    ui->expressionLabel->setText(engine.expression());
    ui->displayEdit->setProperty("error", engine.state() == CalculatorEngine::State::Error);
    ui->displayEdit->style()->unpolish(ui->displayEdit);
    ui->displayEdit->style()->polish(ui->displayEdit);
}
CalculatorWindow::~CalculatorWindow() = default;
