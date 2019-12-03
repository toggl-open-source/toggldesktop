#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QString>
#include <QUrl>

#include "oauth2.h"

namespace Ui {
    class LoginDialog;
}

class OAUTH2_INTERNAL_EXPORT LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();
    void setLoginUrl(const QString& url);
    QString accessToken();

    void keyPressEvent(QKeyEvent *e) override;

signals:
    void accessTokenObtained();

private slots:
    void urlChanged(const QUrl& url);


private:
    Ui::LoginDialog *ui;
    QString m_strAccessToken;
};

#endif // LOGINDIALOG_H
