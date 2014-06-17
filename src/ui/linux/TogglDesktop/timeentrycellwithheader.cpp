#include "timeentrycellwithheader.h"
#include "ui_timeentrycellwithheader.h"

TimeEntryCellWithHeader::TimeEntryCellWithHeader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeEntryCellWithHeader)
{
    ui->setupUi(this);
}

TimeEntryCellWithHeader::~TimeEntryCellWithHeader()
{
    delete ui;
}
