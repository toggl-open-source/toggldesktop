// Copyright 2014 Toggl Desktop developers.

#include <QApplication>
#include <QMetaType>
#include <QDebug>
#include <QVector>

#include <stdint.h>
#include <stdbool.h>

#include "qtsingleapplication.h"  // NOLINT

#include "./mainwindowcontroller.h"

int main(int argc, char *argv[]) {
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<_Bool>("_Bool");
    qRegisterMetaType<QVector<TimeEntryView*> >("QVector<TimeEntryView*>");
    qRegisterMetaType<QVector<AutocompleteView*> >("QVector<AutocompleteView*");

    QtSingleApplication a(argc, argv);

    if (a.sendMessage(("Wake up!"))) {
        qDebug() << "An instance of TogglDesktop is already running. "
                 "This instance will now quit.";
        return 0;
    }

    MainWindowController w;
    w.show();

    return a.exec();
}
