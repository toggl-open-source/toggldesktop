#include "mainwindowcontroller.h"
#include "ui_mainwindowcontroller.h"

#include "kopsik_api.h"

MainWindowController::MainWindowController(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowController)
{
    ui->setupUi(this);

    ctx_ = kopsik_context_init("linux_native_app", "FIXME");
}

MainWindowController::~MainWindowController()
{
    delete ui;
}
