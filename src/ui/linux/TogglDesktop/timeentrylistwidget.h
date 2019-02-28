// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYLISTWIDGET_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYLISTWIDGET_H_

#include <QWidget>
#include <QVector>
#include <QMutex>
#include <QStackedWidget>

#include <stdint.h>

#include "./timeentryview.h"

namespace Ui {
class TimeEntryListWidget;
}

class TimerWidget;

class TimeEntryListWidget : public QWidget {
    Q_OBJECT

 public:
    explicit TimeEntryListWidget(QStackedWidget *parent = nullptr);
    ~TimeEntryListWidget();

    void display();

    TimerWidget *timer();

 private slots:  // NOLINT

    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayTimeEntryList(
        const bool open,
        QVector<TimeEntryView *> list,
        const bool show_load_more_button);

    void showLoadMoreButton(int size);

    void on_blankView_linkActivated(const QString &link);

 private:
    Ui::TimeEntryListWidget *ui;

    QMutex render_m_;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYLISTWIDGET_H_
