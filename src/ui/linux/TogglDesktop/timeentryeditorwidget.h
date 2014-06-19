#ifndef TIMEENTRYEDITORWIDGET_H
#define TIMEENTRYEDITORWIDGET_H

#include <QWidget>
#include <QVector>

#include <stdint.h>

#include "timeentryview.h"

namespace Ui {
class TimeEntryEditorWidget;
}

class TimeEntryEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimeEntryEditorWidget(QWidget *parent = 0);
    ~TimeEntryEditorWidget();

private:
    Ui::TimeEntryEditorWidget *ui;

    QString guid;

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
    void on_doneButton_clicked();
};

#endif // TIMEENTRYEDITORWIDGET_H
