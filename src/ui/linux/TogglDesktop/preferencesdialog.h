// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_PREFERENCESDIALOG_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_PREFERENCESDIALOG_H_

#include <QDialog>
#include <QKeyEvent>
#include "./settingsview.h"

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog {
    Q_OBJECT

 public:
    explicit PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

 private:
    Ui::PreferencesDialog *ui;
    int keyId;
    QString keySequence;

    bool setSettings();
    bool setProxySettings();

 private slots:  // NOLINT
    void displaySettings(const bool open,
                         SettingsView *settings);

    void displayLogin(const bool open,
                      const uint64_t user_id);

    void onDayCheckboxClicked(bool checked);

    void on_proxyHost_editingFinished();
    void on_proxyPort_editingFinished();
    void on_proxyUsername_editingFinished();
    void on_proxyPassword_editingFinished();
    void on_idleDetection_clicked(bool checked);
    void on_recordTimeline_clicked(bool checked);
    void on_remindToTrackTime_clicked(bool checked);
    void on_pomodoroTimer_clicked(bool checked);
    void on_pomodoroBreakTimer_clicked(bool checked);
    void on_useProxy_clicked(bool checked);
    void on_idleMinutes_editingFinished();
    void on_reminderMinutes_editingFinished();
    void on_pomodoroMinutes_editingFinished();
    void on_pomodoroBreakMinutes_editingFinished();
    void on_useSystemProxySettings_clicked(bool checked);
    void on_focusAppOnShortcut_clicked(bool checked);
    void on_continueStopClear_clicked();
    void on_showHideClear_clicked();
    void on_continueStopButton_clicked();
    void on_showHideButton_clicked();
    void on_reminderStartTimeEdit_editingFinished();
    void on_reminderEndTimeEdit_editingFinished();
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void saveCurrentShortcut();
    void onStopEntryCheckboxClicked(bool checked);

    void updateShowHideShortcut();
    void updateContinueStopShortcut();
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_PREFERENCESDIALOG_H_
