#include "calculatorwindow.h"
#include "ui_calculatorwindow.h"
#include <QPushButton>
CalculatorWindow::CalculatorWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::CalculatorWindow)
{
    ui->setupUi(this);
    for (auto *button : findChildren<QPushButton *>()) {
        connect(button, &QPushButton::clicked, this, [this, button] {
            dispatch(button->property("command").toString());
        });
}
}
void CalculatorWindow::dispatch(const QString &command)
{
    engine.input(command);
    ui->displayEdit->setText(engine.display());
    ui->expressionLabel->setText(engine.expression());
    ui->displayEdit->setProperty("error", engine.state() == CalculatorEngine::State::Error);
}
CalculatorWindow::~CalculatorWindow() = default;
