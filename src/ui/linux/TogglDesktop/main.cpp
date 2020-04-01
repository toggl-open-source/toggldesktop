// Copyright 2014 Toggl Desktop developers.

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QMetaType>
#include <QVector>
#include <QFontDatabase>
#include <qtwebengineglobal.h>
#include <QMessageBox>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "singleapplication.h"  // NOLINT

#include "./autocompleteview.h"
#include "./bugsnag.h"
#include "./genericview.h"
#include "./toggl.h"
#include "./timeentryview.h"
#include "./countryview.h"
#include "./autocompletelistmodel.h"

#include "mainwindow.h"

class TogglApplication : public SingleApplication {
 public:
    TogglApplication(int &argc, char **argv)
        : SingleApplication(argc, argv)
    {}

    virtual bool notify(QObject *receiver, QEvent *event);
};

bool TogglApplication::notify(QObject *receiver, QEvent *event) {
    try {
        return SingleApplication::notify(receiver, event);
    } catch(std::exception &e) {
        TogglApi::notifyBugsnag("std::exception", e.what(), receiver->objectName());
    } catch(...) {
        TogglApi::notifyBugsnag("unspecified", "exception", receiver->objectName());
    }
    return true;
}

void setOptions(const TogglApplication &a) {
    QCommandLineParser parser;
    parser.setApplicationDescription("Toggl Desktop");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption logPathOption("log-path", "<path> of the app log file", "path");
    parser.addOption(logPathOption);

    QCommandLineOption dbPathOption("db-path", "<path> of the app DB file", "path");
    parser.addOption(dbPathOption);

    // A boolean option with multiple names (-b, --background)
    QCommandLineOption forceOption({ "b", "background" }, "Start app in background.");
    parser.addOption(forceOption);

    parser.process(a);
}

void registerTypes() {
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<bool_t>("bool_t");
    qRegisterMetaType<char_t>("char_t");
    qRegisterMetaType<QVector<TimeEntryView*> >("QVector<TimeEntryView*>");
    qRegisterMetaType<QVector<AutocompleteView*> >("QVector<AutocompleteView*>");
    qRegisterMetaType<QVector<GenericView*> >("QVector<GenericView*>");

    qmlRegisterUncreatableType<CountryView>("toggl", 1, 0, "Country", "Created by backend code");
    qmlRegisterUncreatableType<TimeEntryView>("toggl", 1, 0, "TimeEntry", "Created by backend code");
    qmlRegisterUncreatableType<TimeEntryViewStorage>("toggl", 1, 0, "TimeEntryList", "Created by backend code");
    qmlRegisterUncreatableType<AutocompleteView>("toggl", 1, 0, "Autocomplete", "Created by backend code");
    qmlRegisterUncreatableType<AutocompleteListModel>("toggl", 1, 0, "AutocompleteListModel", "Created by backend code");
    qmlRegisterUncreatableType<AutocompleteProxyModel>("toggl", 1, 0, "AutocompleteProxyModel", "Created by backend code");
}


int main(int argc, char *argv[]) try {
    Bugsnag::apiKey = "aa13053a88d5133b688db0f25ec103b7";
    Bugsnag::app.version = APP_VERSION;

    qputenv("QML_DISABLE_DISTANCEFIELD", "1");

    TogglApplication::setQuitOnLastWindowClosed(false);
    QApplication::setApplicationName("Toggl Desktop");
    QApplication::setOrganizationName("Toggl");

    TogglApplication a(argc, argv);
    if (a.isRunning()) {
        qDebug() << "An instance of the app is already running";
        return 0;
    }
    a.setApplicationVersion(APP_VERSION);
    setOptions(a);

    QtWebEngine::initialize();
    registerTypes();

    /*
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("toggl", new TogglApi(nullptr));
    engine.load(QUrl(QStringLiteral("qrc:/MainWindow.qml")));
    */

    auto w = new MainWindow();
    w->show();

    if (!TogglApi::instance->startEvents()) {
        QMessageBox(QMessageBox::Warning,
                    "Error",
                    "The application could not start. Please inspect the log file.",
                    QMessageBox::Ok|QMessageBox::Cancel)
            .exec();
        return 1;
    /* TODO readd
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

    w = new MainWindowController(nullptr,
                                 parser.value(logPathOption),
                                 parser.value(dbPathOption),
                                 parser.value(scriptPathOption));

    a.w = w;

    w->show();
    if (parser.isSet(forceOption)) {
        QTimer::singleShot(1, w, &MainWindowController::hide);
    */
    }

    return a.exec();
} catch (std::exception &e) {
    TogglApi::notifyBugsnag("std::exception", e.what(), "main");
    return 1;
} catch (...) {
    TogglApi::notifyBugsnag("unspecified", "exception", "main");
    return 1;
}
