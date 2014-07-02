#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

#include "settingsview.h"

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

 public:
    explicit PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

 private:
    Ui::PreferencesDialog *ui;

    bool setSettings();
    bool setProxySettings();

 private slots:
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
};

#endif // PREFERENCESDIALOG_H
