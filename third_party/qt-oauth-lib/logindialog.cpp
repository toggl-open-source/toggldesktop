#include "logindialog.h"
#include "ui_logindialog.h"

#include <QDebug>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    ui->webView->page()->profile()->cookieStore()->deleteAllCookies();
    ui->webView->page()->profile()->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    connect(ui->webView, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChanged(QUrl)));
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::urlChanged(const QUrl &url)
{
    qDebug() << "URL =" << url;
    QString str = url.toString();
    if(str.indexOf("access_token") != -1)
    {
        QStringList query = str.split("#");
        QStringList lst = query[1].split("&");
        for (int i=0; i<lst.count(); i++ )
        {
            QStringList pair = lst[i].split("=");
            if (pair[0] == "access_token")
            {
                m_strAccessToken = pair[1];
                emit accessTokenObtained();
                QDialog::accept();
            }
        }
    }
}

QString LoginDialog::accessToken()
{
    return m_strAccessToken;
}


void LoginDialog::setLoginUrl(const QString& url)
{
    ui->webView->page()->profile()->cookieStore()->deleteAllCookies();
    ui->webView->setUrl(url);
}
