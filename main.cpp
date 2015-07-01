#include <QApplication>
#include "mainwindow.h"
#include "include/plate.h"
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
