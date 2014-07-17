// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYCELLWIDGET_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYCELLWIDGET_H_

#include <QWidget>

#include "./timeentryview.h"

namespace Ui {
class TimeEntryCellWidget;
}

class TimeEntryCellWidget : public QWidget {
    Q_OBJECT

 public:
    TimeEntryCellWidget();
    ~TimeEntryCellWidget();

    void display(TimeEntryView *view);
    QSize getSizeHint(bool is_header);


 protected:
    virtual void mousePressEvent(QMouseEvent *event);

 private slots:  // NOLINT
    void on_continueButton_clicked();

 private:
    Ui::TimeEntryCellWidget *ui;

    QString guid;
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYCELLWIDGET_H_
