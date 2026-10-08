QT += widgets testlib
CONFIG += testcase console c++17
TARGET = calculator_test
TEMPLATE = app
INCLUDEPATH += ..
SOURCES += calculator_test.cpp ../calculatorengine.cpp ../calculatorwindow.cpp
HEADERS += ../calculatorengine.h ../calculatorwindow.h
FORMS += ../calculatorwindow.ui
