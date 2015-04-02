// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_PREFERENCESDIALOG_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_PREFERENCESDIALOG_H_

#include <QDialog>

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

    bool setSettings();
    bool setProxySettings();

 private slots:  // NOLINT
    void displaySettings(const bool open,
                         SettingsView *settings);

    void displayLogin(const bool open,
                      const uint64_t user_id);

    void on_proxyHost_editingFinished();
    void on_proxyPort_editingFinished();
    void on_proxyUsername_editingFinished();
    void on_proxyPassword_editingFinished();
    void on_idleDetection_clicked(bool checked);
    void on_recordTimeline_clicked(bool checked);
    void on_remindToTrackTime_clicked(bool checked);
    void on_useProxy_clicked(bool checked);
    void on_idleMinutes_editingFinished();
    void on_reminderMinutes_editingFinished();
    void on_useSystemProxySettings_clicked(bool checked);
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_PREFERENCESDIALOG_H_
