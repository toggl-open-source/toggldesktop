#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QQmlEngine>
#include <QQmlContext>
#include <QUrl>

#include <toggl.h>

#ifdef __APPLE__
#include "macosSpecific.h"
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , preferences_(new PreferencesDialog(this))
{
    ui->setupUi(this);
    ui->centralwidget->engine()->rootContext()->setContextProperty("toggl", new TogglApi(nullptr));
    ui->centralwidget->setSource(QUrl("qrc:/MainWindow.qml"));
#ifdef __APPLE__
    auto color = QPalette().color(QPalette::Base);
    changeTitleBarColor(effectiveWinId(), color.redF(), color.greenF(), color.blueF());
#endif
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionAbout_triggered() {
    // TODO
}

void MainWindow::on_actionHelp_triggered() {
    TogglApi::instance->getSupport();
}

void MainWindow::on_actionUpdateCheck_triggered() {
    // TODO
}

void MainWindow::on_actionPreferences_triggered() {
    preferences_->show();
}

void MainWindow::on_actionQuit_triggered() {
    qApp->quit();
}

void MainWindow::on_actionNew_triggered() {
    TogglApi::instance->start("", "", 0, 0, nullptr, false);
}

void MainWindow::on_actionContinue_triggered() {
    TogglApi::instance->continueLatestTimeEntry();
}

void MainWindow::on_actionStop_triggered() {
    TogglApi::instance->stop();
}

void MainWindow::on_actionShow_triggered() {
    // TODO
}

void MainWindow::on_actionEdit_triggered() {
    TogglApi::instance->editRunningTimeEntry("description");
}

void MainWindow::on_actionSync_triggered() {
    TogglApi::instance->sync();
}

void MainWindow::on_actionClearCache_triggered() {
    // TODO
}

void MainWindow::on_actionLogout_triggered() {
    TogglApi::instance->logout();
}
