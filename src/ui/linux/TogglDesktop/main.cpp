#include "mainwindowcontroller.h"
#include <QApplication>
#include <QMetaType>

#include <stdint.h>
#include <stdbool.h>

#include "qtsingleapplication.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<_Bool>("_Bool");

    QtSingleApplication a(argc, argv);

    if (a.sendMessage(("Wake up!"))) {
        return 0;
    }

    MainWindowController w;
    w.show();

    return a.exec();
}
