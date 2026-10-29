QT += widgets
CONFIG += c++17
TARGET = texteditor
TEMPLATE = app
SOURCES += main.cpp mainwindow.cpp codeeditor.cpp documentio.cpp searchdialog.cpp
HEADERS += mainwindow.h codeeditor.h documentio.h searchdialog.h
FORMS += mainwindow.ui searchdialog.ui aboutdialog.ui
RESOURCES += resources.qrc
