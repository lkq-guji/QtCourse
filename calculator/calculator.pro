QT += widgets
CONFIG += c++17
TARGET = calculator
TEMPLATE = app
SOURCES += main.cpp calculatorwindow.cpp calculatorengine.cpp
HEADERS += calculatorwindow.h calculatorengine.h
FORMS += calculatorwindow.ui
RESOURCES += resources.qrc
