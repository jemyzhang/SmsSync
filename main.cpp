#include <QtGui/QApplication>
#include "mainwindow.h"



int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(res);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    a.setLibraryPaths(QStringList("plugins"));
    return a.exec();
}

