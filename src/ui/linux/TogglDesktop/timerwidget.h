// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_TIMERWIDGET_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_TIMERWIDGET_H_

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QLabel>
#include "./autocompletedropdownlist.h"

namespace Ui {
class TimerWidget;
}

class AutocompleteView;
class TimeEntryView;

class TimerWidget : public QWidget {
    Q_OBJECT

 public:
    explicit TimerWidget(QWidget *parent = 0);
    ~TimerWidget();

 public slots:
    void fillInData(AutocompleteView *view);

 signals:
    void buttonClicked();

 protected:
    void mousePressEvent(QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent *);

 private slots:  // NOLINT
    void displayStoppedTimerState();

    void displayRunningTimerState(
        TimeEntryView *te);

    void displayMinitimerAutocomplete(
        QVector<AutocompleteView *> list);

    void start();
    void stop();

    void timeout();

    void focusChanged(QWidget *old, QWidget *now);
    void on_start_clicked();

    void on_duration_returnPressed();

    void fillInAndStart();

 private:
    Ui::TimerWidget *ui;

    QTimer *timer;

    int64_t duration;

    QString project;
    QString descriptionPlaceholder;
    QString tagsHolder;
    uint64_t taskId;
    uint64_t projectId;

    AutocompleteDropdownList *dropdown;

    bool timeEntryAutocompleteNeedsUpdate;
    QVector<AutocompleteView *> timeEntryAutocompleteUpdate;
    void setEllipsisTextToLabel(QLabel *label, QString text);
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_TIMERWIDGET_H_
