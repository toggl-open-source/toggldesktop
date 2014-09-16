// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYCELLWIDGET_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYCELLWIDGET_H_

#include <QWidget>

#include "./timeentryview.h"
#include "./clickablelabel.h"

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
    void labelClicked(QString field_name);

 protected:
    virtual void mousePressEvent(QMouseEvent *event);

 private slots:  // NOLINT
    void on_continueButton_clicked();

 private:
    Ui::TimeEntryCellWidget *ui;

    QString guid;
    QString getProjectColor(QString color);
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYCELLWIDGET_H_
