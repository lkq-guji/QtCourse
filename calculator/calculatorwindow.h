#pragma once
#include <QWidget>
#include <memory>
#include "calculatorengine.h"
namespace Ui { class CalculatorWindow; }
class CalculatorWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CalculatorWindow(QWidget *parent = nullptr);
    ~CalculatorWindow();
    void dispatch(const QString &command);
private:
    std::unique_ptr<Ui::CalculatorWindow> ui;
    CalculatorEngine engine;
};
