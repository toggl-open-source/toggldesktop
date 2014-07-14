// Copyright 2014 Toggl Desktop developers.

#include <QApplication>
#include <QMetaType>
#include <QDebug>
#include <QVector>

#include <stdint.h>
#include <stdbool.h>

#include "qtsingleapplication.h"  // NOLINT

#include "./mainwindowcontroller.h"

class TogglApplication : public QtSingleApplication {
 public:
    TogglApplication(int &argc, char **argv)  // NOLINT
        : QtSingleApplication(argc, argv) {}

    virtual bool notify(QObject *receiver, QEvent *event) {
        try {
            return QtSingleApplication::notify(receiver, event);
        } catch(std::exception e) {
            qCritical() << "Exception thrown: " << e.what();
        }
        return false;
    }
};

int main(int argc, char *argv[]) {
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<_Bool>("_Bool");
    qRegisterMetaType<QVector<TimeEntryView*> >("QVector<TimeEntryView*>");
    qRegisterMetaType<QVector<AutocompleteView*> >("QVector<AutocompleteView*");

    TogglApplication a(argc, argv);

    if (a.sendMessage(("Wake up!"))) {
        qDebug() << "An instance of TogglDesktop is already running. "
                 "This instance will now quit.";
        return 0;
    }

    a.setApplicationVersion(APP_VERSION);

    MainWindowController w;
    w.show();

    return a.exec();
}
