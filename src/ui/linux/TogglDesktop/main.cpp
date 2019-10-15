// Copyright 2014 Toggl Desktop developers.

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QMetaType>
#include <QVector>
#include <QFontDatabase>
#include <qtwebengineglobal.h>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>

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

    virtual bool notify(QObject *receiver, QEvent *event);
};

bool TogglApplication::notify(QObject *receiver, QEvent *event) {
    try {
        return SingleApplication::notify(receiver, event);
    } catch(std::exception &e) {
        TogglApi::notifyBugsnag("std::exception", e.what(),
                                receiver->objectName());
    } catch(...) {
        TogglApi::notifyBugsnag("unspecified", "exception",
                                receiver->objectName());
    }
    return true;
}

int main(int argc, char *argv[]) try {
    Bugsnag::apiKey = "aa13053a88d5133b688db0f25ec103b7";

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

    QtWebEngine::initialize();
    a.setApplicationVersion(APP_VERSION);
    Bugsnag::app.version = APP_VERSION;

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

    qputenv("QML_DISABLE_DISTANCEFIELD", "1");

    QQmlApplicationEngine engine;
    qmlRegisterType<CountryView>("toggl", 1, 0, "Country");
    qmlRegisterType<TimeEntryView>("toggl", 1, 0, "TimeEntry");
    engine.rootContext()->setContextProperty("toggl", new TogglApi(nullptr));
    engine.load(QUrl(QStringLiteral("qrc:/MainWindow.qml")));
    if (!TogglApi::instance->startEvents()) {
        QMessageBox(
            QMessageBox::Warning,
            "Error",
            "The application could not start. Please inspect the log file.",
            QMessageBox::Ok|QMessageBox::Cancel).exec();
        return 1;
    }

    return a.exec();
} catch (std::exception &e) {  // NOLINT
    TogglApi::notifyBugsnag("std::exception", e.what(), "main");
    return 1;
} catch (...) {  // NOLINT
    TogglApi::notifyBugsnag("unspecified", "exception", "main");
    return 1;
}
