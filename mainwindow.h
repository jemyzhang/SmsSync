#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int sqlite_export();
private:
    Ui::MainWindow *ui;

private slots:
    void on_btnImport_clicked();
};

#endif // MAINWINDOW_H
