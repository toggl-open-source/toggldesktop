#include "networkmanagement.h"

#include <QDebug>
#include <QTimer>

NetworkManagement::NetworkManagement(QObject *parent)
    : QObject(parent)
    , manager(new QNetworkConfigurationManager(this))
    , online(false)
{
    connect(manager, &QNetworkConfigurationManager::updateCompleted, this, &NetworkManagement::onUpdateCompleted);
    connect(manager, &QNetworkConfigurationManager::configurationChanged, this, &NetworkManagement::onConfigurationChanged);
    manager->updateConfigurations();
}

bool NetworkManagement::isOnline() const {
    return online;
}

void NetworkManagement::onUpdateCompleted() {
    checkForActive();
}

void NetworkManagement::onConfigurationChanged(const QNetworkConfiguration &config) {
    checkForActive();
}

void NetworkManagement::checkForActive() {
    bool newState = false;

    for (auto config : manager->allConfigurations()) {
        // look only for potential internet-connected devices
        if (config.bearerType() != QNetworkConfiguration::BearerUnknown) {
            // == instead of & is intentional (Active is a bitmask of three different flags)
            if (config.state() == QNetworkConfiguration::Active) {
                newState = true;
                break;
            }
        }
    }

    if (newState != online) {
        online = newState;
        emit onlineStateChanged();
    }
}
