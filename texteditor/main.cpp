#include "mainwindow.h"
#include <QApplication>
#include <QFont>
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("QtCourseTextEditor");
    app.setOrganizationName("QtCourse");
    app.setStyle("Fusion");
    app.setFont(QFont("Microsoft YaHei UI", 10));
    auto *window = new MainWindow;
    window->show();
    return app.exec();
}
