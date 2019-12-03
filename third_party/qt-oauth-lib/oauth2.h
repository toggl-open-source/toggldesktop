#ifndef OAUTH2_H
#define OAUTH2_H

#include <QString>
#include <QObject>

#if defined(_WIN32) || defined(WIN32)
# ifdef OAUTH2_BUILD_DLL
#  define OAUTH2_INTERNAL_EXPORT __declspec(dllexport)
# else
#  define OAUTH2_INTERNAL_EXPORT __declspec(dllimport)
# endif
#else
# define OAUTH2_INTERNAL_EXPORT
#endif // _WIN32 || WIN32

class LoginDialog;

class OAUTH2_INTERNAL_EXPORT OAuth2 : public QObject
{
    Q_OBJECT

public:
    OAuth2(QWidget* parent = 0);
    QString accessToken();
    bool isAuthorized();
    void startLogin(bool bForce);

    //Functions to set application's details.
    void setScope(const QString& scope);
    void setClientID(const QString& clientID);
    void setRedirectURI(const QString& redirectURI);
    void setCompanyName(const QString& companyName);
    void setAppName(const QString& appName);

    QString loginUrl();

signals:
    //Signal that is emitted when login is ended OK.
    void loginDone();

private slots:
    void accessTokenObtained();

private:
    QString m_strAccessToken;

    QString m_strEndPoint;
    QString m_strScope;
    QString m_strClientID;
    QString m_strRedirectURI;
    QString m_strResponseType;

    QString m_strCompanyName;
    QString m_strAppName;

    LoginDialog* m_pLoginDialog;
    QWidget* m_pParent;
};

#endif // OAUTH2_H
