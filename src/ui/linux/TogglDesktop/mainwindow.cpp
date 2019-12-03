#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QQmlEngine>
#include <QQmlContext>
#include <QUrl>

#include <toggl.h>

#ifdef __APPLE__
#include "macosSpecific.h"
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->centralwidget->engine()->rootContext()->setContextProperty("toggl", new TogglApi(nullptr));
    ui->centralwidget->setSource(QUrl("qrc:/MainWindow.qml"));
#ifdef __APPLE__
    auto color = QPalette().color(QPalette::Base);
    changeTitleBarColor(effectiveWinId(), color.redF(), color.greenF(), color.blueF());
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}
