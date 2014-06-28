/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QTBROWSERPLUGIN_H
#define QTBROWSERPLUGIN_H

#include <QMetaObject>
#include <QFile>
#include <QDateTime>
#include <QPainter>


struct QtNPInstance;
class QtNPBindable;
class QtNPStreamPrivate;

struct NPP_t;
typedef NPP_t* NPP;

class QtNPBindable
{
    friend class QtNPStream;
public:
    enum Reason {
        ReasonDone = 0,
        ReasonBreak = 1,
        ReasonError = 2,
        ReasonUnknown = -1
    };
    enum DisplayMode
    {
        Embedded = 1,
        Fullpage = 2
    };

    QMap<QByteArray, QVariant> parameters() const;
    DisplayMode displayMode() const;
    QString mimeType() const;

    QString userAgent() const;
    void getNppVersion(int *major, int *minor) const;
    void getBrowserVersion(int *major, int *minor) const;

    // incoming streams (SRC=... tag)
    virtual bool readData(QIODevice *source, const QString &format);

    // URL stuff
    int openUrl(const QString &url, const QString &window = QString());
    int uploadData(const QString &url, const QString &window, const QByteArray &data);
    int uploadFile(const QString &url, const QString &window, const QString &filename);
    virtual void transferComplete(const QString &url, int id, Reason r);

    NPP instance() const;

protected:
    QtNPBindable();
    virtual ~QtNPBindable();

private:
    QtNPInstance* pi;
};

class QtNPFactory {
public:
    QtNPFactory();
    virtual ~QtNPFactory();

    virtual QStringList mimeTypes() const = 0;
    virtual QObject* createObject(const QString &type) = 0;

    virtual QString pluginName() const = 0;
    virtual QString pluginDescription() const = 0;
};

extern QtNPFactory *qtNPFactory();

template<class T>
class QtNPClass : public QtNPFactory
{
public:
    QtNPClass() {}

    QObject *createObject(const QString &key)
    {
        foreach (QString mime, mimeTypes()) {
            if (mime.left(mime.indexOf(':')) == key)
                return new T;
        }
        return 0;
    }

    QStringList mimeTypes() const
    {
        const QMetaObject &mo = T::staticMetaObject;
        return QString::fromLatin1(mo.classInfo(mo.indexOfClassInfo("MIME")).value()).split(';');
    }

    QString pluginName() const { return QString(); }
    QString pluginDescription() const { return QString(); }
};

#define QTNPFACTORY_BEGIN(Name, Description) \
class QtNPClassList : public QtNPFactory \
{ \
    QHash<QString, QtNPFactory*> factories; \
    QStringList mimeStrings; \
    QString m_name, m_description; \
public: \
    QtNPClassList() \
    : m_name(Name), m_description(Description) \
    { \
        QtNPFactory *factory = 0; \
        QStringList keys; \

#define QTNPCLASS(Class) \
        { \
        factory = new QtNPClass<Class>; \
        keys = factory->mimeTypes(); \
        foreach (QString key, keys) { \
            mimeStrings.append(key); \
            factories.insert(key.left(key.indexOf(':')), factory); \
        } \
        } \

#define QTNPFACTORY_END() \
    } \
    ~QtNPClassList() { /*crashes? qDeleteAll(factories);*/ } \
    QObject *createObject(const QString &mime) { \
        QtNPFactory *factory = factories.value(mime); \
        return factory ? factory->createObject(mime) : 0; \
    } \
    QStringList mimeTypes() const { return mimeStrings; } \
    QString pluginName() const { return m_name; } \
    QString pluginDescription() const { return m_description; } \
}; \
QtNPFactory *qtns_instantiate() { return new QtNPClassList; } \

#define QTNPFACTORY_EXPORT(Class) \
QtNPFactory *qtns_instantiate() { return new Class; }

#endif // QTBROWSERPLUGIN_H
