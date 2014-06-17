#include "aboutwindowcontroller.h"
#include "ui_aboutwindowcontroller.h"

AboutWindowController::AboutWindowController(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutWindowController)
{
    ui->setupUi(this);
}

AboutWindowController::~AboutWindowController()
{
    delete ui;
}
