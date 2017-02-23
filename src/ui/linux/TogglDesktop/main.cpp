// Copyright 2014 Toggl Desktop developers.

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QMetaType>
#include <QVector>
#include <QFontDatabase>

#include <stdint.h>
#include <stdbool.h>

#include "singleapplication.h"  // NOLINT

#include "./autocompleteview.h"
#include "./bugsnag.h"
#include "./genericview.h"
#include "./mainwindowcontroller.h"
#include "./toggl.h"

class TogglApplication : public SingleApplication {
 public:
    TogglApplication(int &argc, char **argv)  // NOLINT
        : SingleApplication(argc, argv) {}

    virtual bool notify(QObject *receiver, QEvent *event) {
        try {
            return SingleApplication::notify(receiver, event);
        } catch(std::exception e) {
            TogglApi::notifyBugsnag("std::exception", e.what(),
                                    receiver->objectName());
        } catch(...) {
            TogglApi::notifyBugsnag("unspecified", "exception",
                                    receiver->objectName());
        }
        return true;
    }
};

int main(int argc, char *argv[]) try {
    Bugsnag::apiKey = "2a46aa1157256f759053289f2d687c2f";

    TogglApplication::setQuitOnLastWindowClosed(false);

    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<bool_t>("bool_t");
    qRegisterMetaType<QVector<TimeEntryView*> >("QVector<TimeEntryView*>");
    qRegisterMetaType<QVector<AutocompleteView*> >("QVector<AutocompleteView*>");
    qRegisterMetaType<QVector<GenericView*> >("QVector<GenericView*>");

    QApplication::setApplicationName("Toggl Desktop");
    QApplication::setOrganizationName("Toggl");

    TogglApplication a(argc, argv);
    if (a.isRunning()) {
        qDebug() << "An instance of the app is already running";
        return 0;
    }

    a.setApplicationVersion(APP_VERSION);
    Bugsnag::app.version = APP_VERSION;

    // Use bundled fonts
    int id = QFontDatabase::addApplicationFont(
        ":/fonts/RobotoTTF/Roboto-Regular.ttf");
    if (-1 == id) {
        qDebug() << "Error! Could not load bundled font!";
    } else {
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);
        QFont font(family);
        QApplication::setFont(font);
    }
    // qDebug() << "Application font: " << QApplication::font().toString();

    QCommandLineParser parser;
    parser.setApplicationDescription("Toggl Desktop");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption logPathOption(
        QStringList() << "log-path",
        "<path> of the app log file",
        "path");
    parser.addOption(logPathOption);

    QCommandLineOption dbPathOption(
        QStringList() << "db-path",
        "<path> of the app DB file",
        "path");
    parser.addOption(dbPathOption);

    QCommandLineOption scriptPathOption(
        QStringList() << "script-path",
        "<path> of a Lua script to run",
        "path");
    parser.addOption(scriptPathOption);

    // A boolean option with multiple names (-b, --background)
    QCommandLineOption forceOption(QStringList() << "b" << "background",
                                   QCoreApplication::translate("main", "Start app in background."));
    parser.addOption(forceOption);

    parser.process(a);

    MainWindowController w(0,
                           parser.value(logPathOption),
                           parser.value(dbPathOption),
                           parser.value(scriptPathOption));

    a.w = &w;

    if (parser.isSet(forceOption)) {
        w.hide();
    } else {
        w.show();
    }

    return a.exec();
} catch (std::exception &e) {  // NOLINT
    TogglApi::notifyBugsnag("std::exception", e.what(), "main");
    return 1;
} catch (...) {  // NOLINT
    TogglApi::notifyBugsnag("unspecified", "exception", "main");
    return 1;
}
