#ifndef TIMEENTRYLISTWIDGET_H
#define TIMEENTRYLISTWIDGET_H

#include <QWidget>
#include <QVector>

#include <stdint.h>

#include "timeentryview.h"

namespace Ui {
class TimeEntryListWidget;
}

class TimeEntryListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimeEntryListWidget(QWidget *parent = 0);
    ~TimeEntryListWidget();

private slots:
    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayTimeEntryList(
        const bool open,
        QVector<TimeEntryView *> list);

    void displayTimeEntryEditor(
        const bool open,
        TimeEntryView *view,
        const QString focused_field_name);

private:
    Ui::TimeEntryListWidget *ui;
};

#endif // TIMEENTRYLISTWIDGET_H
