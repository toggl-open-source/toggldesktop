#include "timeentrylistwidget.h"
#include "ui_timeentrylistwidget.h"

#include "toggl_api.h"
#include "timerwidget.h"

TimeEntryListWidget::TimeEntryListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeEntryListWidget)
{
    ui->setupUi(this);

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)), this, SLOT(displayLogin(bool,uint64_t)));
    connect(TogglApi::instance, SIGNAL(displayTimeEntryList(bool,QVector<TimeEntryView*>)),
            this, SLOT(displayTimeEntryList(bool,QVector<TimeEntryView*>)));

}

TimeEntryListWidget::~TimeEntryListWidget()
{
    delete ui;
}

void TimeEntryListWidget::displayLogin(
    const bool open,
    const uint64_t user_id) {

    if (open || !user_id) {
        setVisible(false);
    }
}

void TimeEntryListWidget::displayTimeEntryList(
    const bool open,
    QVector<TimeEntryView *> list)
{
    if (open) {
        setVisible(true);
    }
    // FIXME: render list
}

