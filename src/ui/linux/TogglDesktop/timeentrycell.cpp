#include "timeentrycell.h"
#include "ui_timeentrycell.h"

TimeEntryCell::TimeEntryCell(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeEntryCell)
{
    ui->setupUi(this);
}

TimeEntryCell::~TimeEntryCell()
{
    delete ui;
}
