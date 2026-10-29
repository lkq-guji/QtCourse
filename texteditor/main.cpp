#include "mainwindow.h"
#include <QApplication>
#include <QFont>
#include <QIcon>
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("QtCourseTextEditor");
    app.setOrganizationName("QtCourse");
    app.setStyle("Fusion");
    app.setFont(QFont("Microsoft YaHei UI", 10));
    app.setWindowIcon(QIcon(":/icons/new.svg"));
    auto *window = new MainWindow;
    window->show();
    for (int i = 1; i < argc; ++i) window->openPath(QString::fromLocal8Bit(argv[i]));
    return app.exec();
}
