#ifndef NETWORKMANAGEMENT_H
#define NETWORKMANAGEMENT_H

#include <QObject>
#include <QNetworkConfigurationManager>

class NetworkManagement : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManagement(QObject *parent = nullptr);

    bool isOnline() const;

signals:
    void onlineStateChanged();

private slots:
    void onUpdateCompleted();
    void onConfigurationChanged(const QNetworkConfiguration &config);

    void checkForActive();

private:
    QNetworkConfigurationManager *manager;
    bool online;
};

#endif // NETWORKMANAGEMENT_H
