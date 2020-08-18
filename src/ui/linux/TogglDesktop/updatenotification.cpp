#include "updatenotification.h"
#include "ui_updatenotification.h"

#include <iostream>

UpdateNotification::UpdateNotification(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateNotification)
{
    ui->setupUi(this);

    QDBusInterface flatpak("org.freedesktop.portal.Flatpak",
                           "/org/freedesktop/portal/Flatpak",
                           "org.freedesktop.portal.Flatpak",
                           QDBusConnection::sessionBus(),
                           nullptr);
    std::cerr << "CONNECTING: " << connect(&flatpak, SIGNAL("UpdateAvailable(QVariantMap)"), this, SLOT("onUpdateAvailable(QVariantMap"));
    std::cerr << flatpak.isValid();
    if (flatpak.isValid()) {
        auto reply = flatpak.call("CreateUpdateMonitor", QVariantMap{});

        std::cerr << reply.arguments().count() << reply.errorMessage().toStdString() << std::endl;
        if (reply.arguments().count() == 1 && reply.arguments().first().canConvert<QDBusObjectPath>()) {
            std::cerr << qvariant_cast<QDBusObjectPath>(reply.arguments().first()).path().toStdString() << std::endl;

            auto i = new QDBusInterface("org.freedesktop.portal.Flatpak",
                                        qvariant_cast<QDBusObjectPath>(reply.arguments().first()).path(),
                                        "org.freedesktop.portal.Flatpak.UpdateMonitor",
                                        QDBusConnection::sessionBus(),
                                        this);

            QDBusInterface hu("org.freedesktop.portal.Flatpak",
                                                    qvariant_cast<QDBusObjectPath>(reply.arguments().first()).path(),
                                                    "org.freedesktop.DBus.Introspectable",
                                                    QDBusConnection::sessionBus(),
                                                    this);
            auto result = hu.call("Introspect");
            for (auto i : result.arguments())
                 {
                std::cerr << "Introspection result: " << i.toString().toStdString() << std::endl;
            }

            if (i->isValid()) {
                std::cerr << "VALID" << std::endl;
                updateMonitor = i;
                for (auto i : i->dynamicPropertyNames())
                    std::cerr<< QString(i).toStdString() << ",";
                std::cerr<<"\n";
                std::cerr << "CONNECTING: " << connect(i, "UpdateAvailable(QVariantMap)", this, SLOT("onUpdateAvailable(QVariantMap"));
                std::cerr << "CONNECTING: " << QDBusConnection::sessionBus().connect("org.freedesktop.portal.Flatpak",
                                                                                     "/org/freedesktop/portal/Flatpak/UpdateMonitor",
                                                                                     "org.freedesktop.portal.Flatpak.UpdateMonitor",
                                                                                     "UpdateAvailable",
                                                                                     this, SLOT("onUpdateAvailable(QVariantMap)")) << std::endl;
                std::cerr << "CONNECTING: " << QDBusConnection::sessionBus().connect("org.freedesktop.portal.Flatpak",
                                                                                     "/org/freedesktop/portal/Flatpak/update_monitor",
                                                                                     "org.freedesktop.portal.Flatpak.UpdateMonitor",
                                                                                     "UpdateAvailable",
                                                                                     this, SLOT("onUpdateAvailable(QVariantMap)")) << std::endl;
            }

        }
    }
    if (updateMonitor) {
        //std::cerr << "CONNECTING: " << connect(updateMonitor, SIGNAL("UpdateAvailable(QVariantMap)"), this, SLOT("onUpdateAvailable(QVariantMap)")) << std::endl;
    }
}

UpdateNotification::~UpdateNotification()
{
    delete ui;
}

void UpdateNotification::onUpdateAvailable(const QVariantMap &args) {
    std::cerr << "UPDATE AVAILABLE";
    for (auto i : args.keys()) {
        std::cerr << ", " << i.toStdString();
        if (args[i].canConvert<QString>())
            std::cerr << ": " << args[i].toString().toStdString();
    }
    std::cerr << std::endl;
    show();
}
