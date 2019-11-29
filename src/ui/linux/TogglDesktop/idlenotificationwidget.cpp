// Copyright 2014 Toggl Desktop developers.

#include "./idlenotificationwidget.h"
#include "./ui_idlenotificationwidget.h"

#include "./toggl.h"
#include "./settingsview.h"

#ifdef __linux
#include <X11/extensions/scrnsaver.h>  // NOLINT
#endif

IdleNotificationWidget::IdleNotificationWidget(QStackedWidget *parent)
    : QWidget(parent),
  ui(new Ui::IdleNotificationWidget),
  idleStarted(0),
  dbusApiAvailable(true),
  screenLocked(false),
  timeEntryGUID(""),
  idleHintTimer(new QTimer(this)) {
    ui->setupUi(this);

#ifdef __linux
    screensaver = new QDBusInterface("org.freedesktop.ScreenSaver", "/org/freedesktop/ScreenSaver", "org.freedesktop.ScreenSaver", QDBusConnection::sessionBus(), this);
    connect(screensaver, SIGNAL(ActiveChanged(bool)), this, SLOT(onScreensaverActiveChanged(bool)));
#endif
    connect(TogglApi::instance, &TogglApi::displayIdleNotification, this, &IdleNotificationWidget::displayIdleNotification);

    connect(TogglApi::instance, SIGNAL(displaySettings(bool,SettingsView*)),  // NOLINT
            this, SLOT(displaySettings(bool,SettingsView*)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayStoppedTimerState()),  // NOLINT
            this, SLOT(displayStoppedTimerState()));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT


    connect(idleHintTimer, &QTimer::timeout, this, &IdleNotificationWidget::requestIdleHint);
    idleHintTimer->setInterval(5000);
    idleHintTimer->start();
}

void IdleNotificationWidget::displaySettings(const bool open, SettingsView *settings) {
    if (settings->UseIdleDetection && !idleHintTimer->isActive()) {
        idleHintTimer->start(1000);
    } else if (!settings->UseIdleDetection && idleHintTimer->isActive()) {
        idleHintTimer->stop();
    }
}

void IdleNotificationWidget::requestIdleHint() {
#ifdef __linux
    if (dbusApiAvailable) {
        auto pendingCall = screensaver->asyncCall("GetSessionIdleTime");
        auto watcher = new QDBusPendingCallWatcher(pendingCall, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &IdleNotificationWidget::idleHintReceived);
    }
    else {
        Display *display = XOpenDisplay(NULL);
        if (!display) {
            return;
        }
        XScreenSaverInfo *info = XScreenSaverAllocInfo();
        if (XScreenSaverQueryInfo(display, DefaultRootWindow(display), info)) {
            uint64_t idleSeconds = info->idle / 1000;
            storeIdlePeriod(idleSeconds);
        }
        XFree(info);
        XCloseDisplay(display);
    }
#endif
}

#ifdef __linux
void IdleNotificationWidget::idleHintReceived(QDBusPendingCallWatcher *watcher) {

    QDBusPendingReply<uint> reply = *watcher;
    if (reply.isError()) {
        dbusApiAvailable = false;
        qWarning() << reply.error();
    }
    else {
        qulonglong value = reply.argumentAt<0>();
        uint64_t idleSeconds = value / 1000;
        storeIdlePeriod(idleSeconds);
    }
    watcher->deleteLater();
}
#endif

void IdleNotificationWidget::onScreensaverActiveChanged(bool active) {
    screenLocked = active;
}

void IdleNotificationWidget::display() {
    auto sw = qobject_cast<QStackedWidget*>(parent());
    if (sw->currentWidget() != this) {
        if (sw->currentIndex() >= 0)
            previousView = sw->currentWidget();
        else
            previousView = nullptr;
        sw->setCurrentWidget(this);
    }
}

void IdleNotificationWidget::hide() {
    if (previousView) {
        qobject_cast<QStackedWidget*>(parent())->setCurrentWidget(previousView);
        previousView = nullptr;
    }
}

bool IdleNotificationWidget::isScreenLocked() const {
    return screenLocked;
}

void IdleNotificationWidget::storeIdlePeriod(uint64_t period) {
    if (isScreenLocked()) {
        TogglApi::instance->setIdleSeconds(static_cast<uint64_t>(time(nullptr)) - lastActiveTime);
    }
    else {
        lastActiveTime = static_cast<uint64_t>(time(nullptr)) - period;
        TogglApi::instance->setIdleSeconds(period);
    }
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

void IdleNotificationWidget::on_addIdleTimeAsNewEntryButton_clicked() {
    TogglApi::instance->discardTimeAt(timeEntryGUID, idleStarted, true);
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
