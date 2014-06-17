#include "mainwindowcontroller.h"
#include <QApplication>
#include <QMetaType>

int main(int argc, char *argv[])
{
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<_Bool>("_Bool");

    QApplication a(argc, argv);
    MainWindowController w;
    w.show();

    return a.exec();
}
