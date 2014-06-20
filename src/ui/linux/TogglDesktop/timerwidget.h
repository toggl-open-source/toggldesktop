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

protected:
    void mousePressEvent(QMouseEvent *event);

private slots:
    void displayStoppedTimerState();

    void displayRunningTimerState(
        TimeEntryView *te);

    void displayTimeEntryAutocomplete(
        QVector<AutocompleteView *> list);

    void on_start_clicked();

    void start();

    void stop();

    void timeout();

    void focusChanged(QWidget *old, QWidget *now);

    void on_description_editTextChanged(const QString &arg1);

    void on_description_currentIndexChanged(int index);

private:
    Ui::TimerWidget *ui;

    QTimer *timer;

    int64_t duration;

    bool timeEntryAutocompleteNeedsUpdate;
    QVector<AutocompleteView *> timeEntryAutocompleteUpdate;
};

#endif // TIMERWIDGET_H
