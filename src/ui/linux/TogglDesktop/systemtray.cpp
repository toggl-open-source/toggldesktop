// Copyright 2018 Toggl Desktop developers.

#include <QTimer>
#include <QProcessEnvironment>

#include "./toggl.h"
#include "./systemtray.h"
#include "./mainwindowcontroller.h"

SystemTray::SystemTray(MainWindowController *parent, QIcon defaultIcon) :
    QSystemTrayIcon(parent),
    notificationsPresent(true)
{
    setIcon(defaultIcon);

    show();

    connect(TogglApi::instance, &TogglApi::displayIdleNotification, this, &SystemTray::displayIdleNotification);

    connect(TogglApi::instance, SIGNAL(displayReminder(QString,QString)),  // NOLINT
            this, SLOT(displayReminder(QString,QString)));  // NOLINT

    notifications = new QDBusInterface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", QDBusConnection::sessionBus(), this);
    notificationsPresent = notifications->isValid();

    connect(notifications, SIGNAL(NotificationClosed(uint,uint)), this, SLOT(notificationClosed(uint,uint)));
    connect(notifications, SIGNAL(ActionInvoked(uint,QString)), this, SLOT(notificationActionInvoked(uint,QString)));

    auto pendingCall = notifications->asyncCall("GetCapabilities");
    auto watcher = new QDBusPendingCallWatcher(pendingCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &SystemTray::notificationCapabilitiesReceived);
}

MainWindowController *SystemTray::mainWindow() {
    return qobject_cast<MainWindowController*>(parent());
}

bool SystemTray::isVisible() const {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    return true;
#endif
    QString currentDesktop = QProcessEnvironment::systemEnvironment().value(
        "XDG_CURRENT_DESKTOP", "");
    return "Unity" != currentDesktop;
}

bool SystemTray::notificationsAvailable() {
    return notificationsPresent;
}

void SystemTray::notificationCapabilitiesReceived(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<QStringList> reply = *watcher;
    if (reply.isError()) {
        qWarning() << reply.error();
    }
    else {
        QStringList value = reply.argumentAt<0>();
        if (!value.contains("actions")) {
            qWarning() << "DBus backend doesn't support notifications";
        }
    }
    watcher->deleteLater();
}

uint SystemTray::requestNotification(uint previous, const QString &title, const QString &description) {
    // on supported platforms, this makes icons work as we want
    if (notificationsPresent) {
        QByteArray data;
        QImage im(":/icons/64x64/toggldesktop.png");
        im = im.convertToFormat(QImage::Format_RGBA8888);

        for (int i = 0; i < im.height(); i++) {
            data.append(reinterpret_cast<const char*>(im.scanLine(i)), im.width() * 4);
        }

        QStringList actions;
        actions << "default" << "Open Toggl";

        // prepare the structure with the image beforehand
        // as far as I know, it cannot be done inline (without defining a custom stream operator)
        QDBusArgument hints;
        hints.beginStructure();
        hints << im.width() // width in pixels
              << im.height() // height in pixels
              << im.width() * 4 // line stride (line length in bytes including padding)
              << 1 // has alpha
              << 8 // bits per pixel
              << 4 // samples per pixel (4 - ARGB format)
              << data; // byte array with pixel data
        hints.endStructure();

        auto reply = notifications->call("Notify", // function name
                                         "TogglDesktop", // application name
                                         previous, // replaces ID
                                         "", // application icon - we need none because we pass it with the data
                                         title,
                                         description,
                                         actions, // actions - pairs - first string is identifier, second is display text
                                         QVariantMap { // hints
                                             { "icon_data", QVariant::fromValue(hints) }
                                         },
                                         0); // timeout - 0 is never expire, -1 is default, >0 in ms

        if (reply.type() == QDBusMessage::InvalidMessage) {
            qWarning() << "When requesting a notification:" << reply.errorMessage();
            return 0;
        }

        return reply.arguments()[0].toUInt();
    }
    // fallback for other platforms
    else {
        showMessage(title, description, QIcon(":/icons/64x64/toggldesktop.png"));
        return 0;
    }
}

void SystemTray::notificationClosed(uint id, uint reason) {
    Q_UNUSED(reason);
    if (id == lastIdleNotification)
        lastIdleNotification = 0;
    if (id == lastReminder)
        lastReminder = 0;
}

void SystemTray::notificationActionInvoked(uint id, const QString &action) {
    Q_UNUSED(id);
    if (action == "default") {
        mainWindow()->setWindowState(Qt::WindowActive);
    }
}

void SystemTray::displayIdleNotification(
        const QString guid,
        const QString since,
        const QString duration,
        const uint64_t started,
        const QString description) {
    lastTimeEntryGuid = guid;
    lastStarted = started;
    QString title = QString("%1 (%2)").arg(since).arg(duration);
    lastIdleNotification = requestNotification(lastIdleNotification, title, description);
}

void SystemTray::displayReminder(QString title, QString description) {
    lastReminder = requestNotification(lastReminder, title, description);
}
