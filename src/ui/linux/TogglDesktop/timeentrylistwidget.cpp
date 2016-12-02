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

    connect(TogglApi::instance, SIGNAL(displayTimeEntryList(bool,QVector<TimeEntryView*>,bool)),  // NOLINT
            this, SLOT(displayTimeEntryList(bool,QVector<TimeEntryView*>,bool)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTimeEntryEditor(bool,TimeEntryView*,QString)),  // NOLINT
            this, SLOT(displayTimeEntryEditor(bool,TimeEntryView*,QString)));  // NOLINT

    ui->blankView->setVisible(false);
}

TimeEntryListWidget::~TimeEntryListWidget() {
    delete ui;
}

void TimeEntryListWidget::displayLogin(
    const bool open,
    const uint64_t user_id) {

    if (open || !user_id) {
        ui->list->clear();
        setVisible(false);
    }
}

void TimeEntryListWidget::displayTimeEntryList(
    const bool open,
    QVector<TimeEntryView *> list,
    const bool show_load_more_button) {

    int size = list.size();
    if (open) {
        setVisible(true);
    }
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    render_m_.lock();

    for (int i = 0; i < size; i++) {
        TimeEntryView *te = list.at(i);

        QListWidgetItem *item = 0;
        TimeEntryCellWidget *cell = 0;

        if (ui->list->count() > i) {
            item = ui->list->item(i);
            cell = static_cast<TimeEntryCellWidget *>(
                ui->list->itemWidget(item));
        }

        if (!item) {
            item = new QListWidgetItem();
            cell = new TimeEntryCellWidget();

            ui->list->addItem(item);
            ui->list->setItemWidget(item, cell);
        }

        cell->display(te);

        QSize sizeHint = cell->getSizeHint(te->IsHeader);
        item->setSizeHint(sizeHint);
    }

    if (show_load_more_button) {
        showLoadMoreButton(size);
        size++;
    }

    while (ui->list->count() > size) {
        ui->list->model()->removeRow(size);
    }

    ui->list->setVisible(!list.isEmpty());
    ui->blankView->setVisible(list.isEmpty());

    render_m_.unlock();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void TimeEntryListWidget::displayTimeEntryEditor(
    const bool open,
    TimeEntryView *view,
    const QString focused_field_name) {
    if (open) {
        setVisible(false);
    }
}

void TimeEntryListWidget::showLoadMoreButton(int size) {
    QListWidgetItem *item = 0;
    TimeEntryCellWidget *cell = 0;

    if (ui->list->count() > size) {
        item = ui->list->item(size);
        cell = static_cast<TimeEntryCellWidget *>(
            ui->list->itemWidget(item));
    }

    if (!item) {
        item = new QListWidgetItem();
        cell = new TimeEntryCellWidget();

        ui->list->addItem(item);
        ui->list->setItemWidget(item, cell);
    }

    cell->setLoadMore(true);

    QSize sizeHint = cell->getSizeHint(false);
    item->setSizeHint(sizeHint);
}

void TimeEntryListWidget::on_blankView_linkActivated(const QString &link) {
    TogglApi::instance->openInBrowser();
}
