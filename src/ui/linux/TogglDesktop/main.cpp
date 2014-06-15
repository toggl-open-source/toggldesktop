#include "mainwindowcontroller.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindowController w;
    w.show();

    return a.exec();
}
