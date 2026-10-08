#include "calculatorengine.h"
#include <cmath>

QString CalculatorEngine::format(double value)
{
    return value == 0 ? "0" : QString::number(value, 'g', 15);
}
QString CalculatorEngine::symbol(const QString &op)
{
    if (op == "*") return QStringLiteral("×");
    if (op == "/") return QStringLiteral("÷");
    if (op == "-") return QStringLiteral("−");
    return op;
}
void CalculatorEngine::clearAll()
{
    entry = "0";
    expressionText.clear();
    pendingOperator.clear();
    lastOperator.clear();
    accumulator = lastOperand = 0;
    currentState = State::Entering;
}
void CalculatorEngine::beginEntry()
{
    if (currentState == State::Result || currentState == State::Error) clearAll();
    if (currentState == State::WaitingOperand) entry = "0";
    currentState = State::Entering;
}
bool CalculatorEngine::calculate(double right, const QString &op)
{
    if (op == "/" && right == 0) {
        entry = QStringLiteral("不能除以零");
        currentState = State::Error;
    } else {
        if (op == "+") accumulator += right;
        else if (op == "-") accumulator -= right;
        else if (op == "*") accumulator *= right;
        else if (op == "/") accumulator /= right;
        if (!std::isfinite(accumulator)) {
            entry = QStringLiteral("结果超出范围");
            currentState = State::Error;
        }
    }
    if (currentState == State::Error) {
        pendingOperator.clear();
        lastOperator.clear();
        return false;
    }
    entry = format(accumulator);
    return true;
}
void CalculatorEngine::input(const QString &command)
{
    if (command == "C") { clearAll(); return; }
    const bool digit = command.size() == 1 && command.at(0) >= QChar('0') && command.at(0) <= QChar('9');
    if (digit || command == ".") {
        beginEntry();
        if (command == ".") {
            if (!entry.contains('.')) entry += '.';
        } else {
            QString digits = entry;
            digits.remove('-'); digits.remove('.');
            if (digits.size() >= 15) return; //限制输入长度，计算采用 double 和 15 位有效显示。
            if (entry == "0") entry = command;
            else if (entry == "-0") entry = "-" + command;
            else entry += command;
        }
        return;
    }
    if (command == "CE") {
        if (currentState == State::Result || currentState == State::Error) clearAll();
        else { entry = "0"; currentState = State::Entering; lastOperator.clear(); }
        return;
    }
    if (currentState == State::Error) return;
    if (command == "BS") {
        if (currentState != State::Entering) return; //不改写已经确认的左操作数或计算结果。
        entry.chop(1);
        if (entry.isEmpty() || entry == "-") entry = "0";
        return;
    }
    if (command == "SIGN") {
        if (currentState == State::WaitingOperand) { entry = "0"; currentState = State::Entering; }
        const QString previousEntry = entry;
        entry = entry.startsWith('-') ? entry.mid(1) : "-" + entry;
        if (currentState == State::Result) expressionText = QStringLiteral("−(%1)").arg(previousEntry);
        lastOperator.clear();
        return;
    }
    if (command == "+" || command == "-" || command == "*" || command == "/") {
        if (!pendingOperator.isEmpty() && currentState == State::Entering) {
            if (!calculate(entry.toDouble(), pendingOperator)) return;
        } else accumulator = entry.toDouble();
        pendingOperator = command;
        lastOperator.clear();
        expressionText = format(accumulator) + " " + symbol(command);
        currentState = State::WaitingOperand;
        return;
    }
    if (command == "=") {
        if (!pendingOperator.isEmpty()) {
            if (currentState == State::WaitingOperand) return; //缺少右操作数时等待输入。
            const double right = entry.toDouble();
            const QString op = pendingOperator;
            expressionText = format(accumulator) + " " + symbol(op) + " " + entry + " =";
            if (!calculate(right, op)) return;
            lastOperator = op;
            lastOperand = right;
            pendingOperator.clear();
        } else if (currentState == State::Result && !lastOperator.isEmpty()) {
            accumulator = entry.toDouble();
            expressionText = entry + " " + symbol(lastOperator) + " " + format(lastOperand) + " =";
            if (!calculate(lastOperand, lastOperator)) return;
        } else expressionText = entry + " =";
        currentState = State::Result;
    }
}
