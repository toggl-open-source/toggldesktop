#ifndef TIMERWIDGET_H
#define TIMERWIDGET_H

#include <QWidget>
#include <QVector>
#include <QTimer>

#include "timeentryview.h"
#include "timerwidget.h"
#include "autocompleteview.h"

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

    void displayTimeEntryAutocomplete(
        QVector<AutocompleteView *> list);

    void displayProjectAutocomplete(
        QVector<AutocompleteView *> list);

    void on_start_clicked();

    void start();
    void stop();

    void timeout();

private:
    Ui::TimerWidget *ui;

    QTimer *timer;

    int64_t duration;
};

#endif // TIMERWIDGET_H
