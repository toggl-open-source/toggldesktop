// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_TIMERWIDGET_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_TIMERWIDGET_H_

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QLabel>

namespace Ui {
class TimerWidget;
}

class AutocompleteListModel;
class AutocompleteView;
class TimeEntryView;

class TimerWidget : public QWidget {
    Q_OBJECT

 public:
    explicit TimerWidget(QWidget *parent = 0);
    ~TimerWidget();

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
    void on_description_currentIndexChanged(int index);

    void on_duration_returnPressed();

    void descriptionReturnPressed();

 private:
    Ui::TimerWidget *ui;

    QTimer *timer;

    int64_t duration;

    QString project;
    QString descriptionPlaceholder;
    QString tagsHolder;

    bool timeEntryAutocompleteNeedsUpdate;
    QVector<AutocompleteView *> timeEntryAutocompleteUpdate;
    AutocompleteListModel *descriptionModel;
    void setEllipsisTextToLabel(QLabel *label, QString text);
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_TIMERWIDGET_H_
