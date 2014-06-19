#ifndef TIMEENTRYEDITORWIDGET_H
#define TIMEENTRYEDITORWIDGET_H

#include <QWidget>

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
};

#endif // TIMEENTRYEDITORWIDGET_H
