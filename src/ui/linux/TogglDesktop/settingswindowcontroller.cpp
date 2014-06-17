#include "settingswindowcontroller.h"
#include "ui_settingswindowcontroller.h"

SettingsWindowController::SettingsWindowController(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsWindowController)
{
    ui->setupUi(this);
}

SettingsWindowController::~SettingsWindowController()
{
    delete ui;
}
