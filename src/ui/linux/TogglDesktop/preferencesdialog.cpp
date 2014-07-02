#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include "toggl.h"
#include "settingsview.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    connect(TogglApi::instance, SIGNAL(displaySettings(bool,SettingsView*)),
            this, SLOT(displaySettings(bool,SettingsView*)));

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),
            this, SLOT(displayLogin(bool,uint64_t)));
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::displaySettings(const bool open,
                                        SettingsView *settings)
{
    if (open)
    {
        show();
    }

    ui->useProxy->setChecked(settings->UseProxy);
    ui->proxyHost->setText(settings->ProxyHost);
    ui->proxyPort->setText(QString::number(settings->ProxyPort));
    ui->proxyUsername->setText(settings->ProxyUsername);
    ui->proxyPassword->setText(settings->ProxyPassword);

    ui->idleDetection->setChecked(settings->UseIdleDetection);
    ui->recordTimeline->setChecked(settings->RecordTimeline); // user based!
    ui->remindToTrackTime->setChecked(settings->Reminder);
}

void PreferencesDialog::displayLogin(const bool open,
                                     const uint64_t user_id)
{
    ui->recordTimeline->setEnabled(!open && user_id);
}

void PreferencesDialog::on_proxyHost_editingFinished()
{
    setProxySettings();
}

void PreferencesDialog::on_proxyPort_editingFinished()
{
    setProxySettings();
}

void PreferencesDialog::on_proxyUsername_editingFinished()
{
    setProxySettings();
}

void PreferencesDialog::on_proxyPassword_editingFinished()
{
    setProxySettings();
}

void PreferencesDialog::on_idleDetection_clicked(bool checked)
{
    setSettings();
}

void PreferencesDialog::on_recordTimeline_clicked(bool checked)
{
    TogglApi::instance->toggleTimelineRecording();
}

void PreferencesDialog::on_remindToTrackTime_clicked(bool checked)
{
    setSettings();
}

bool PreferencesDialog::setProxySettings()
{
    return TogglApi::instance->setProxySettings(ui->useProxy->isChecked(),
                                                ui->proxyHost->text(),
                                                ui->proxyPort->text().toInt(),
                                                ui->proxyUsername->text(),
                                                ui->proxyPassword->text());
}

bool PreferencesDialog::setSettings()
{
    return TogglApi::instance->setSettings(ui->idleDetection->isChecked(),
                                           false,
                                           false,
                                           false,
                                           ui->remindToTrackTime->isChecked());
}

void PreferencesDialog::on_useProxy_clicked(bool checked)
{
    setProxySettings();
}
