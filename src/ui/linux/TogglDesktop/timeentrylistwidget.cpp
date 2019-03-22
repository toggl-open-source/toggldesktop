// Copyright 2014 Toggl Desktop developers.

#include "./timeentrylistwidget.h"
#include "./ui_timeentrylistwidget.h"

#include "./toggl.h"
#include "./timerwidget.h"
#include "./timeentrycellwidget.h"

TimeEntryListWidget::TimeEntryListWidget(QStackedWidget *parent) : QWidget(parent),
ui(new Ui::TimeEntryListWidget) {
    ui->setupUi(this);

    connect(ui->list, &QListWidget::currentRowChanged, [=](int row){
        qCritical() << row;
    });

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTimeEntryList(bool,QVector<TimeEntryView*>,bool)),  // NOLINT
            this, SLOT(displayTimeEntryList(bool,QVector<TimeEntryView*>,bool)));  // NOLINT

    ui->blankView->setVisible(false);
}

TimeEntryListWidget::~TimeEntryListWidget() {
    delete ui;
}

void TimeEntryListWidget::display() {
    qobject_cast<QStackedWidget*>(parent())->setCurrentWidget(this);
}

TimeEntryCellWidget *TimeEntryListWidget::highlightedCell() {
    auto w = focusWidget();
    while (w) {
        auto cell = qobject_cast<TimeEntryCellWidget*>(w);
        if (cell)
            return cell;
        w = w->parentWidget();
    }
    return nullptr;
}

TimerWidget *TimeEntryListWidget::timer() {
    return ui->timer;
}

void TimeEntryListWidget::displayLogin(
    const bool open,
    const uint64_t user_id) {

    if (open || !user_id) {
        ui->list->clear();
    }
}

void TimeEntryListWidget::displayTimeEntryList(
    const bool open,
    QVector<TimeEntryView *> list,
    const bool show_load_more_button) {

    int size = list.size();
    if (open) {
        display();
    }
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    render_m_.lock();

    for (int i = 0; i < size; i++) {
        TimeEntryView *te = list.at(i);

        QListWidgetItem *item = nullptr;
        TimeEntryCellWidget *cell = nullptr;

        if (ui->list->count() > i) {
            item = ui->list->item(i);
            cell = static_cast<TimeEntryCellWidget *>(
                ui->list->itemWidget(item));
        }

        if (!item) {
            item = new QListWidgetItem();
            cell = new TimeEntryCellWidget(item);

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

void TimeEntryListWidget::showLoadMoreButton(int size) {
    QListWidgetItem *item = nullptr;
    TimeEntryCellWidget *cell = nullptr;

    if (ui->list->count() > size) {
        item = ui->list->item(size);
        cell = static_cast<TimeEntryCellWidget *>(
            ui->list->itemWidget(item));
    }

    if (!item) {
        item = new QListWidgetItem();
        cell = new TimeEntryCellWidget(item);

        ui->list->addItem(item);
        ui->list->setItemWidget(item, cell);
    }

    cell->setLoadMore(true);

    QSize sizeHint = cell->getSizeHint(false);
    item->setSizeHint(sizeHint);
}

void TimeEntryListWidget::on_blankView_linkActivated(const QString &link) {
    Q_UNUSED(link);
    TogglApi::instance->openInBrowser();
}
