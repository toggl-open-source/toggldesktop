#include "timeentryeditviewcontroller.h"
#include "ui_timeentryeditviewcontroller.h"

TimeEntryEditViewController::TimeEntryEditViewController(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeEntryEditViewController)
{
    ui->setupUi(this);
}

TimeEntryEditViewController::~TimeEntryEditViewController()
{
    delete ui;
}
