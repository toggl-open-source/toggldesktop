// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYLISTWIDGET_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYLISTWIDGET_H_

#include <QWidget>
#include <QVector>
#include <QMutex>

#include <stdint.h>

#include "./timeentryview.h"

namespace Ui {
class TimeEntryListWidget;
}

class TimeEntryListWidget : public QWidget {
    Q_OBJECT

 public:
    explicit TimeEntryListWidget(QWidget *parent = 0);
    ~TimeEntryListWidget();

 private slots:  // NOLINT
    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayTimeEntryList(
        const bool open,
        QVector<TimeEntryView *> list,
        const bool show_load_more_button);

    void displayTimeEntryEditor(
        const bool open,
        TimeEntryView *view,
        const QString focused_field_name);

    void showLoadMoreButton(int size);

    void on_blankView_linkActivated(const QString &link);

 private:
    Ui::TimeEntryListWidget *ui;

    QMutex render_m_;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYLISTWIDGET_H_
