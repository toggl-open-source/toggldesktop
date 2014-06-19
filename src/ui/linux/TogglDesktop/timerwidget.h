#ifndef TIMERWIDGET_H
#define TIMERWIDGET_H

#include <QWidget>

#include "timeentryview.h"
#include "timerwidget.h"

namespace Ui {
class TimerWidget;
}

class TimerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimerWidget(QWidget *parent = 0);
    ~TimerWidget();

signals:
    void buttonClicked();

private slots:
    void displayStoppedTimerState();

    void displayRunningTimerState(
        TimeEntryView *te);

    void on_start_clicked();

    void start();
    void stop();

private:
    Ui::TimerWidget *ui;
};

#endif // TIMERWIDGET_H
