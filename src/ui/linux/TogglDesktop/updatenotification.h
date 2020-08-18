#ifndef UPDATENOTIFICATION_H
#define UPDATENOTIFICATION_H

#include <QDialog>

#include <QtDBus/QtDBus>

namespace Ui {
class UpdateNotification;
}

class UpdateNotification : public QDialog
{
    Q_OBJECT
public:
    explicit UpdateNotification(QWidget *parent = nullptr);
    ~UpdateNotification();
private slots:
    void onUpdateAvailable(const QVariantMap &args);

private:
    Ui::UpdateNotification *ui { nullptr };
    QDBusInterface *updateMonitor { nullptr };
};

#endif // UPDATENOTIFICATION_H
