// Copyright 2019 Toggl Desktop developers.

#include "./powermanagement.h"

#include "./toggl.h"

PowerManagement::PowerManagement(QObject *parent)
    : QObject(parent)
    , available(true)
{
    login1 = new QDBusInterface("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus(), this);

    getInhibitor();

    available = available && connect(login1, SIGNAL(PrepareForShutdown(bool)), this, SLOT(onPrepareForShutdown(bool)));
    available = available && connect(login1, SIGNAL(PrepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));
}

bool PowerManagement::isAvailable() const {
    return available;
}

void PowerManagement::onPrepareForShutdown(bool shuttingDown) {
    if (shuttingDown) {
        TogglApi::instance->stopEntryOnShutdown();
        clearInhibitor();
    }
    else {
        // can this even happen?
        getInhibitor();
    }
}

void PowerManagement::onPrepareForSleep(bool suspending) {
    if (suspending) {
        TogglApi::instance->stopEntryOnShutdown();
        clearInhibitor();
    }
    else {
        getInhibitor();
    }
}

void PowerManagement::getInhibitor() {
    auto reply = login1->call("Inhibit", "shutdown:sleep", "TogglDesktop", "To stop the timer on shutdown or suspend", "delay");
    qCritical() << reply << reply.errorMessage();
    inhibit = qvariant_cast<QDBusUnixFileDescriptor>(reply.arguments().at(0));
}

void PowerManagement::clearInhibitor() {
    inhibit = QDBusUnixFileDescriptor();
}
