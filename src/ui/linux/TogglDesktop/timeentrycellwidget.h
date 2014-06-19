#ifndef TIMEENTRYCELLWIDGET_H
#define TIMEENTRYCELLWIDGET_H

#include <QWidget>

#include "timeentryview.h"

namespace Ui {
class TimeEntryCellWidget;
}

class TimeEntryCellWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimeEntryCellWidget(TimeEntryView *view);
    ~TimeEntryCellWidget();

private slots:
    void on_continueButton_clicked();

private:
    Ui::TimeEntryCellWidget *ui;

    QString guid;
};

#endif // TIMEENTRYCELLWIDGET_H
