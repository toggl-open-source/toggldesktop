// Copyright 2014 Toggl Desktop developers.

#include "./idlenotificationdialog.h"
#include "./ui_idlenotificationdialog.h"

#include "./toggl.h"
#include "./settingsview.h"

#include <X11/extensions/scrnsaver.h>  // NOLINT

IdleNotificationDialog::IdleNotificationDialog(QWidget *parent)
    : QDialog(parent),
  ui(new Ui::IdleNotificationDialog),
  idleStarted(0),
  timer(new QTimer(this)),
  timeEntryGUID("") {
    ui->setupUi(this);

    connect(
        TogglApi::instance,
        SIGNAL(displayIdleNotification(QString,QString,QString,uint64_t,QString)),  // NOLINT
        this,
        SLOT(displayIdleNotification(QString,QString,QString,uint64_t,QString)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displaySettings(bool,SettingsView*)),  // NOLINT
            this, SLOT(displaySettings(bool,SettingsView*)));  // NOLINT

    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayStoppedTimerState()),  // NOLINT
            this, SLOT(displayStoppedTimerState()));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT
}

IdleNotificationDialog::~IdleNotificationDialog() {
    delete ui;
}

void IdleNotificationDialog::displayLogin(const bool open,
        const uint64_t user_id) {
    if (open || !user_id) {
        hide();
    }
}

void IdleNotificationDialog::displayStoppedTimerState() {
    hide();
}

void IdleNotificationDialog::on_keepTimeButton_clicked() {
    hide();
}

void IdleNotificationDialog::on_discardTimeButton_clicked() {
    TogglApi::instance->discardTimeAt(timeEntryGUID, idleStarted, false);
    hide();
}

void IdleNotificationDialog::on_discardTimeAndContinueButton_clicked() {
    TogglApi::instance->discardTimeAndContinue(timeEntryGUID, idleStarted);
    hide();
}

void IdleNotificationDialog::displaySettings(
    const bool open,
    SettingsView *settings) {
    if (settings->UseIdleDetection && !timer->isActive()) {
        timer->start(1000);
    } else if (!settings->UseIdleDetection && timer->isActive()) {
        timer->stop();
    }
}

void IdleNotificationDialog::displayIdleNotification(
    const QString guid,
    const QString since,
    const QString duration,
    const uint64_t started,
    const QString description) {
    idleStarted = started;
    timeEntryGUID = guid;

    ui->idleSince->setText(since);
    ui->idleDuration->setText(duration);

    ui->timeEntryDescriptionLabel->setText(description);

    show();
}

void IdleNotificationDialog::timeout() {
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        return;
    }
    XScreenSaverInfo *info = XScreenSaverAllocInfo();
    if (XScreenSaverQueryInfo(display, DefaultRootWindow(display), info)) {
        uint64_t idleSeconds = info->idle / 1000;
        TogglApi::instance->setIdleSeconds(idleSeconds);
    }
    XFree(info);
    XCloseDisplay(display);
}

void IdleNotificationDialog::on_pushButton_clicked() {
    TogglApi::instance->discardTimeAt(timeEntryGUID, idleStarted, true);
    hide();
}
