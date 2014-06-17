#include "timereditviewcontroller.h"
#include "ui_timereditviewcontroller.h"

TimerEditViewController::TimerEditViewController(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimerEditViewController)
{
    ui->setupUi(this);
}

TimerEditViewController::~TimerEditViewController()
{
    delete ui;
}
