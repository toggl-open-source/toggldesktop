#include "timeentrylistviewcontroller.h"
#include "ui_timeentrylistviewcontroller.h"

TimeEntryListViewController::TimeEntryListViewController(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeEntryListViewController)
{
    ui->setupUi(this);
}

TimeEntryListViewController::~TimeEntryListViewController()
{
    delete ui;
}
