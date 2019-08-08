// Copyright 2019 Toggl Desktop developers.

#ifndef POWERMANAGEMENT_H
#define POWERMANAGEMENT_H

#include <QObject>
#include <QtDBus/QtDBus>

class PowerManagement : public QObject
{
    Q_OBJECT
public:
    explicit PowerManagement(QObject *parent = nullptr);

    bool isAvailable() const;
    bool aboutToShutdown() const;

signals:

public slots:

private slots:
    void onPrepareForShutdown(bool shuttingDown);
    void onPrepareForSleep(bool suspending);

    void getInhibitor();
    void clearInhibitor();

private:
    QDBusInterface *login1;
    bool available;
    QDBusUnixFileDescriptor inhibit;
    bool commitDataRequested;
};

#endif // POWERMANAGEMENT_H
