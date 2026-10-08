#pragma once
#include <QString>

// 纯计算状态机：鼠标和键盘只发送命令，不分别维护状态。
class CalculatorEngine
{
public:
    enum class State { Entering, WaitingOperand, Result, Error };
    void input(const QString &command);
    QString display() const { return entry; }
    QString expression() const { return expressionText; }
    State state() const { return currentState; }
private:
    QString entry = "0";
    QString expressionText;
    QString pendingOperator;
    QString lastOperator;
    double accumulator = 0;
    double lastOperand = 0;
    State currentState = State::Entering;
    void clearAll();
    void beginEntry();
    bool calculate(double right, const QString &op);
    static QString format(double value);
    static QString symbol(const QString &op);
};
