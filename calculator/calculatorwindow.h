#pragma once
#include <QWidget>
#include <memory>
namespace Ui { class CalculatorWindow; }
class CalculatorWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CalculatorWindow(QWidget *parent = nullptr);
    ~CalculatorWindow();
private:
    std::unique_ptr<Ui::CalculatorWindow> ui;
};
