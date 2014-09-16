// Copyright 2014 Toggl Desktop developers.

#include <QApplication>
#include <QMetaType>
#include <QDebug>
#include <QVector>

#include <stdint.h>
#include <stdbool.h>

#include "qtsingleapplication.h"  // NOLINT

#include "./mainwindowcontroller.h"
#include "./bugsnag.h"

class TogglApplication : public QtSingleApplication {
 public:
    TogglApplication(int &argc, char **argv)  // NOLINT
        : QtSingleApplication(argc, argv) {}

    virtual bool notify(QObject *receiver, QEvent *event) {
        try {
            return QtSingleApplication::notify(receiver, event);
        } catch(std::exception e) {
            Bugsnag::notify("std::exception", e.what(), receiver->objectName());
        } catch(...) {
            Bugsnag::notify("unspecified", "exception", receiver->objectName());
        }
        return true;
    }
};

int main(int argc, char *argv[]) try {
    Bugsnag::apiKey = "2a46aa1157256f759053289f2d687c2f";

    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<_Bool>("_Bool");
    qRegisterMetaType<QVector<TimeEntryView*> >("QVector<TimeEntryView*>");
    qRegisterMetaType<QVector<AutocompleteView*> >("QVector<AutocompleteView*");
    qRegisterMetaType<QVector<GenericView*> >("QVector<GenericView*");

    TogglApplication a(argc, argv);

    if (a.sendMessage(("Wake up!"))) {
        qDebug() << "An instance of TogglDesktop is already running. "
                 "This instance will now quit.";
        return 0;
    }

    a.setApplicationVersion(APP_VERSION);
    Bugsnag::app.version = APP_VERSION;

    MainWindowController w;
    w.show();

    return a.exec();
} catch (std::exception &e) {  // NOLINT
    Bugsnag::notify("std::exception", e.what(), "main");
    return 1;
} catch (...) {  // NOLINT
    Bugsnag::notify("unspecified", "exception", "main");
    return 1;
}
