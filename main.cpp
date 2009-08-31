#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QtPlugin>


//Q_IMPORT_PLUGIN(qsqlodbc)

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(res);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    //a.setLibraryPaths(QStringList("plugins"));
    return a.exec();
}

