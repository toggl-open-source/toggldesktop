// Copyright 2014 Tanel Lebedev.

#ifndef BUGSNAG_H_
#define BUGSNAG_H_

#include "./bugsnag-qt_global.h"

#include <QJsonObject>
#include <QObject>
#include <QList>
#include <QJsonArray>
#include <QHash>
#include <QStringList>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QDebug>
#include <QNetworkReply>

class Notifier {
 public:
    Notifier()
        : name("Bugsnag QT")
    , version("1.0.0")
    , url("https://github.com/tanel/bugsnag-qt") {}

    void write(QJsonObject &json) const {  // NOLINT
        json["name"] = name;
        json["version"] = version;
        json["url"] = url;
    }

    QString name;
    QString version;
    QString url;
};

class StackTrace {
 public:
    StackTrace()
        : file("")
    , lineNumber(0)
    , columnNumber(0)
    , method("")
    , inProject(false) {}

    void write(QJsonObject &json) const {  // NOLINT
        json["file"] = file;
        json["lineNumber"] = lineNumber;
        json["columnNumber"] = columnNumber;
        json["method"] = method;
        json["inProject"] = inProject;
    }

    QString file;
    int lineNumber;
    int columnNumber;
    QString method;
    bool inProject;
};

class Exception {
 public:
    Exception()
        : errorClass("")
    , message("") {}

    void write(QJsonObject &json) const {  // NOLINT
        json["errorClass"] = errorClass;
        json["message"] = message;

        QJsonArray stacktraceJSON;
        foreach(StackTrace trace, stacktrace) {
            QJsonObject lineJSON;
            trace.write(lineJSON);
            stacktraceJSON.append(lineJSON);
        }
        json["stacktrace"] = stacktraceJSON;
    }

    QString errorClass;
    QString message;
    QList<StackTrace> stacktrace;
};

class User {
 public:
    User()
        : id("")
    , name("")
    , email("") {}

    void write(QJsonObject &json) const {  // NOLINT
        json["id"] = id;
        json["name"] = name;
        json["email"] = email;
    }

    QString id;
    QString name;
    QString email;
};

class App {
 public:
    App()
        : version("")
    , releaseStage("production") {}

    void write(QJsonObject &json) const {  // NOLINT
        json["version"] = version;
        json["releaseStage"] = releaseStage;
    }

    QString version;
    QString releaseStage;
};

class Device {
 public:
    Device()
        : osVersion("")
    , hostname("") {}

    void write(QJsonObject &json) const {  // NOLINT
        json["osVersion"] = osVersion;
        json["hostname"] = hostname;
    }

    QString osVersion;
    QString hostname;
};

class Event {
 public:
    Event()
        : context("")
    , groupingHash("")
    , severity("error") {}

    void write(QJsonObject &json) const {  // NOLINT
        json["payloadVersion"] = QString("2");

        if (exceptions.count()) {
            QJsonArray exceptionsJSON;
            foreach(Exception e, exceptions) {
                QJsonObject exceptionJSON;
                e.write(exceptionJSON);
                exceptionsJSON.append(exceptionJSON);
            }
            json["exceptions"] = exceptionsJSON;
        }

        if (!context.isEmpty()) {
            json["context"] = context;
        }
        if (!groupingHash.isEmpty()) {
            json["groupingHash"] = groupingHash;
        }
        if (!severity.isEmpty()) {
            json["severity"] = severity;
        }

        if (!user.email.isEmpty() || !user.id.isEmpty() || !user.email.isEmpty()) {
            QJsonObject userJSON;
            user.write(userJSON);
            json["user"] = userJSON;
        }

        if (!app.releaseStage.isEmpty() || !app.version.isEmpty()) {
            QJsonObject appJSON;
            app.write(appJSON);
            json["app"] = appJSON;
        }

        if (!device.hostname.isEmpty() || !device.osVersion.isEmpty()) {
            QJsonObject deviceJSON;
            device.write(deviceJSON);
            json["device"] = deviceJSON;
        }

        QJsonObject metaDataJSON;
        QHash<QString, QHash<QString, QString> >::const_iterator i =
            metaData.begin();
        bool hasMetaData(false);
        while (i != metaData.constEnd()) {
            QJsonObject valuesJSON;
            QHash<QString, QString>::const_iterator j = i.value().constBegin();
            while (j != i.value().constEnd()) {
                hasMetaData = true;
                valuesJSON[j.key()] = j.value();
                ++j;
            }
            metaDataJSON[i.key()] = valuesJSON;
            ++i;
        }
	if (hasMetaData) {
		json["metaData"] = metaDataJSON;
	}
    }

    QList<Exception> exceptions;
    QString context;
    QString groupingHash;
    QString severity;
    User user;
    App app;
    Device device;
    QHash<QString, QHash<QString, QString> > metaData;
};

class Payload {
 public:
    Payload() {}

    void write(QJsonObject &json) const {  // NOLINT
        json["apiKey"] = apiKey;

        QJsonObject notifierJSON;
        notifier.write(notifierJSON);
        json["notifier"] = notifierJSON;

        QJsonArray eventsJSON;
        foreach(Event event, events) {
            QJsonObject eventJSON;
            event.write(eventJSON);
            eventsJSON.append(eventJSON);
        }
        json["events"] = eventsJSON;
    }

    QString apiKey;
    Notifier notifier;
    QList<Event> events;
};

class BUGSNAGQTSHARED_EXPORT Bugsnag : public QObject {
    Q_OBJECT

 public:
    Bugsnag() {}

    static bool notify(
        const QString errorClass,
        const QString message,
        const QString context,
        QHash<QString, QHash<QString, QString> > *metadata = 0) {

        qDebug() << "Bugsnag notify message:" << message
                    << " context:" << context;

        Payload payload;
        payload.apiKey = Bugsnag::apiKey;
        payload.notifier = Notifier();

        Exception exception;
        exception.message = message;
        exception.errorClass = errorClass;

	// Bugsnag will reject notifications without a 
	// stack trace. So just send something to get
	// things going.
        StackTrace trace;
	trace.file = "somefile";
	trace.lineNumber = 123;
	trace.method = "somemethod";
        exception.stacktrace << trace;

        Event event;
        event.context = context;
        event.exceptions << exception;
        event.user = Bugsnag::user;
        event.app = Bugsnag::app;
        event.device = Bugsnag::device;
        if (metadata) {
            event.metaData = *metadata;
        }

        payload.events << event;

        QString protocol("https");
        if (!Bugsnag::useSSL) {
            protocol = "http";
        }
        QUrl url(protocol + "://notify.bugsnag.com");
        QNetworkRequest request(url);

        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          "application/json");

        QNetworkAccessManager *manager = new QNetworkAccessManager(&instance);

        connect(manager, SIGNAL(finished(QNetworkReply*)),  // NOLINT
                &instance, SLOT(requestFinished(QNetworkReply*)));  // NOLINT

        QJsonObject payloadJSON;
        payload.write(payloadJSON);
        QJsonDocument doc(payloadJSON);

        QByteArray data = doc.toJson();
        qDebug() << "Bugsnag payload " << data;

        manager->post(request, data);

        return true;
    }

    static QString apiKey;
    static QString releaseStage;
    static QStringList notifyReleaseStages;
    static bool autoNotify;
    static bool useSSL;

    static Device device;
    static App app;
    static User user;

    static Bugsnag instance;

 private slots:  // NOLINT
    void requestFinished(QNetworkReply *reply) {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Bugsnag requestFinished with error " << reply->error()
                        << reply->errorString();
        } else {
            QVariant statusCode = reply->attribute(
                        QNetworkRequest::HttpStatusCodeAttribute);
            QByteArray b = reply->readAll();
            qDebug() << "Bugsnag requestFinished success"
                        << ", status code " << statusCode
                           << ", content " << QString(b);
        }
        delete reply;
    }
};

#endif  // BUGSNAG_H_
