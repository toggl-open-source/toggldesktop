// Copyright 2014 Toggl Desktop developers.

#include "./idlenotificationwidget.h"
#include "./ui_idlenotificationwidget.h"

#include "./toggl.h"
#include "./settingsview.h"

#include <X11/extensions/scrnsaver.h>  // NOLINT

IdleNotificationWidget::IdleNotificationWidget(QStackedWidget *parent)
    : QWidget(parent),
  ui(new Ui::IdleNotificationWidget),
  idleStarted(0),
  timeEntryGUID(""),
  idleHintTimer(new QTimer(this)) {
    ui->setupUi(this);

    screensaver = new QDBusInterface("org.freedesktop.ScreenSaver", "/org/freedesktop/ScreenSaver", "org.freedesktop.ScreenSaver", QDBusConnection::sessionBus(), this);

    connect(
        TogglApi::instance,
        SIGNAL(displayIdleNotification(QString,QString,QString,uint64_t,QString)),  // NOLINT
        this,
        SLOT(displayIdleNotification(QString,QString,QString,uint64_t,QString)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayStoppedTimerState()),  // NOLINT
            this, SLOT(displayStoppedTimerState()));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT

    connect(idleHintTimer, &QTimer::timeout, this, &IdleNotificationWidget::requestIdleHint);
}

void IdleNotificationWidget::displaySettings(const bool open, SettingsView *settings) {
    if (settings->UseIdleDetection && !idleHintTimer->isActive()) {
        idleHintTimer->start(1000);
    } else if (!settings->UseIdleDetection && idleHintTimer->isActive()) {
        idleHintTimer->stop();
    }
}

void IdleNotificationWidget::requestIdleHint() {
    auto pendingCall = screensaver->asyncCall("GetSessionIdleTime");
    auto watcher = new QDBusPendingCallWatcher(pendingCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &IdleNotificationWidget::idleHintReceived);
}

void IdleNotificationWidget::idleHintReceived(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<uint> reply = *watcher;
    if (reply.isError()) {
        qWarning() << reply.error();
    }
    else {
        qulonglong value = reply.argumentAt<0>();
        TogglApi::instance->setIdleSeconds(value / 1000);
    }
    watcher->deleteLater();
}

void IdleNotificationWidget::display() {
    auto sw = qobject_cast<QStackedWidget*>(parent());
    previousView = sw->currentWidget();
    sw->setCurrentWidget(this);
}

void IdleNotificationWidget::hide() {
    if (previousView)
        qobject_cast<QStackedWidget*>(parent())->setCurrentWidget(previousView);
}

IdleNotificationWidget::~IdleNotificationWidget() {
    delete ui;
}

void IdleNotificationWidget::displayLogin(const bool open,
        const uint64_t user_id) {
    if (open || !user_id) {
        hide();
    }
}

void IdleNotificationWidget::displayStoppedTimerState() {
    hide();
}

void IdleNotificationWidget::on_keepTimeButton_clicked() {
    hide();
}

void IdleNotificationWidget::on_discardTimeButton_clicked() {
    TogglApi::instance->discardTimeAt(timeEntryGUID, idleStarted, false);
    hide();
}

void IdleNotificationWidget::on_discardTimeAndContinueButton_clicked() {
    TogglApi::instance->discardTimeAndContinue(timeEntryGUID, idleStarted);
    hide();
}

void IdleNotificationWidget::displayIdleNotification(
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

    display();
}

void IdleNotificationWidget::on_pushButton_clicked() {
    TogglApi::instance->discardTimeAt(timeEntryGUID, idleStarted, true);
    hide();
}
