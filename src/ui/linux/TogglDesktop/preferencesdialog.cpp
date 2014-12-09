// Copyright 2014 Toggl Desktop developers.

#include "./preferencesdialog.h"
#include "./ui_preferencesdialog.h"

#include "./toggl.h"
#include "./settingsview.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent),
ui(new Ui::PreferencesDialog) {
    ui->setupUi(this);

    connect(TogglApi::instance, SIGNAL(displaySettings(bool,SettingsView*)),  // NOLINT
            this, SLOT(displaySettings(bool,SettingsView*)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT
}

PreferencesDialog::~PreferencesDialog() {
    delete ui;
}

void PreferencesDialog::displaySettings(const bool open,
                                        SettingsView *settings) {
    if (open) {
        show();
    }

    ui->useProxy->setChecked(settings->UseProxy);
    ui->proxyHost->setText(settings->ProxyHost);
    ui->proxyPort->setText(QString::number(settings->ProxyPort));
    ui->proxyUsername->setText(settings->ProxyUsername);
    ui->proxyPassword->setText(settings->ProxyPassword);

    ui->idleDetection->setChecked(settings->UseIdleDetection);
    ui->recordTimeline->setChecked(settings->RecordTimeline);  // user based!
    ui->remindToTrackTime->setChecked(settings->Reminder);
    ui->idleMinutes->setText(QString::number(settings->IdleMinutes));
}

void PreferencesDialog::displayLogin(const bool open,
                                     const uint64_t user_id) {
    ui->recordTimeline->setEnabled(!open && user_id);
}

void PreferencesDialog::on_proxyHost_editingFinished() {
    setProxySettings();
}

void PreferencesDialog::on_proxyPort_editingFinished() {
    setProxySettings();
}

void PreferencesDialog::on_proxyUsername_editingFinished() {
    setProxySettings();
}

void PreferencesDialog::on_proxyPassword_editingFinished() {
    setProxySettings();
}

void PreferencesDialog::on_idleDetection_clicked(bool checked) {
    setSettings();
}

void PreferencesDialog::on_recordTimeline_clicked(bool checked) {
    TogglApi::instance->toggleTimelineRecording(checked);
}

void PreferencesDialog::on_remindToTrackTime_clicked(bool checked) {
    setSettings();
}

bool PreferencesDialog::setProxySettings() {
    return TogglApi::instance->setProxySettings(ui->useProxy->isChecked(),
            ui->proxyHost->text(),
            ui->proxyPort->text().toInt(),
            ui->proxyUsername->text(),
            ui->proxyPassword->text());
}

bool PreferencesDialog::setSettings() {
    return TogglApi::instance->setSettings(ui->idleDetection->isChecked(),
                                           ui->remindToTrackTime->isChecked(),
                                           ui->idleMinutes->text().toInt());
}

void PreferencesDialog::on_useProxy_clicked(bool checked) {
    setProxySettings();
}

void PreferencesDialog::on_idleMinutes_editingFinished() {
    setSettings();
}
