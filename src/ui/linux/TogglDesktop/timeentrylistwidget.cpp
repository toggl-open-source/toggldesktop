// Copyright 2014 Toggl Desktop developers.

#include "./timeentrylistwidget.h"
#include "./ui_timeentrylistwidget.h"

#include "./toggl.h"
#include "./timerwidget.h"
#include "./timeentrycellwidget.h"

TimeEntryListWidget::TimeEntryListWidget(QWidget *parent) : QWidget(parent),
ui(new Ui::TimeEntryListWidget) {
    ui->setupUi(this);

    setVisible(false);

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTimeEntryList(bool,QVector<TimeEntryView*>)),  // NOLINT
            this, SLOT(displayTimeEntryList(bool,QVector<TimeEntryView*>)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTimeEntryEditor(bool,TimeEntryView*,QString)),  // NOLINT
            this, SLOT(displayTimeEntryEditor(bool,TimeEntryView*,QString)));  // NOLINT
}

TimeEntryListWidget::~TimeEntryListWidget() {
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
    QVector<TimeEntryView *> list) {
    if (open) {
        setVisible(true);
    }
    ui->list->clear();
    foreach(TimeEntryView *view, list) {
        QWidget *widget = 0;
        if (view->IsHeader) {
            widget = new TimeEntryCellWidget(view);
        } else {
            widget = new TimeEntryCellWidget(view);
        }

        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(widget->sizeHint());
        ui->list->addItem(item);
        ui->list->setItemWidget(item, widget);
    }
}

void TimeEntryListWidget::displayTimeEntryEditor(
    const bool open,
    TimeEntryView *view,
    const QString focused_field_name) {
    if (open) {
        setVisible(false);
    }
}
