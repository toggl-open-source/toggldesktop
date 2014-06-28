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


#ifndef QSCRIPTVALUE_H
#define QSCRIPTVALUE_H

#include <qstring.h>


#include <qlist.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Script)

class QScriptClass;
class QScriptValue;
class QScriptEngine;
class QScriptString;
class QVariant;
class QObject;
struct QMetaObject;
class QDateTime;
#ifndef QT_NO_REGEXP
class QRegExp;
#endif

typedef QList<QScriptValue> QScriptValueList;

typedef double qsreal;

class QScriptValuePrivate;
class Q_SCRIPT_EXPORT QScriptValue
{
public:
    enum ResolveFlag {
        ResolveLocal        = 0x00,
        ResolvePrototype    = 0x01,
        ResolveScope        = 0x02,
        ResolveFull         = ResolvePrototype | ResolveScope
    };

    Q_DECLARE_FLAGS(ResolveFlags, ResolveFlag)

    enum PropertyFlag {
        ReadOnly            = 0x00000001,
        Undeletable         = 0x00000002,
        SkipInEnumeration   = 0x00000004,

        PropertyGetter      = 0x00000008,
        PropertySetter      = 0x00000010,

        QObjectMember       = 0x00000020,

        KeepExistingFlags   = 0x00000800,

        UserRange           = 0xff000000            // Users may use these as they see fit.
    };
    Q_DECLARE_FLAGS(PropertyFlags, PropertyFlag)

    enum SpecialValue {
        NullValue,
        UndefinedValue
    };

public:
    QScriptValue();
    ~QScriptValue();
    QScriptValue(const QScriptValue &other);
    QScriptValue(QScriptEngine *engine, SpecialValue val);
    QScriptValue(QScriptEngine *engine, bool val);
    QScriptValue(QScriptEngine *engine, int val);
    QScriptValue(QScriptEngine *engine, uint val);
    QScriptValue(QScriptEngine *engine, qsreal val);
    QScriptValue(QScriptEngine *engine, const QString &val);
#ifndef QT_NO_CAST_FROM_ASCII
    QT_ASCII_CAST_WARN_CONSTRUCTOR QScriptValue(QScriptEngine *engine, const char *val);
#endif

    QScriptValue(SpecialValue value);
    QScriptValue(bool value);
    QScriptValue(int value);
    QScriptValue(uint value);
    QScriptValue(qsreal value);
    QScriptValue(const QString &value);
    QScriptValue(const QLatin1String &value);
#ifndef QT_NO_CAST_FROM_ASCII
    QT_ASCII_CAST_WARN_CONSTRUCTOR QScriptValue(const char *value);
#endif

    QScriptValue &operator=(const QScriptValue &other);

    QScriptEngine *engine() const;

    bool isValid() const;
    bool isBool() const;
    bool isBoolean() const;
    bool isNumber() const;
    bool isFunction() const;
    bool isNull() const;
    bool isString() const;
    bool isUndefined() const;
    bool isVariant() const;
    bool isQObject() const;
    bool isQMetaObject() const;
    bool isObject() const;
    bool isDate() const;
    bool isRegExp() const;
    bool isArray() const;
    bool isError() const;

    QString toString() const;
    qsreal toNumber() const;
    bool toBool() const;
    bool toBoolean() const;
    qsreal toInteger() const;
    qint32 toInt32() const;
    quint32 toUInt32() const;
    quint16 toUInt16() const;
    QVariant toVariant() const;
    QObject *toQObject() const;
    const QMetaObject *toQMetaObject() const;
    QScriptValue toObject() const;
    QDateTime toDateTime() const;
#ifndef QT_NO_REGEXP
    QRegExp toRegExp() const;
#endif

    bool instanceOf(const QScriptValue &other) const;

    bool lessThan(const QScriptValue &other) const;
    bool equals(const QScriptValue &other) const;
    bool strictlyEquals(const QScriptValue &other) const;

    QScriptValue prototype() const;
    void setPrototype(const QScriptValue &prototype);

    QScriptValue scope() const;
    void setScope(const QScriptValue &scope);

    QScriptValue property(const QString &name,
                          const ResolveFlags &mode = ResolvePrototype) const;
    void setProperty(const QString &name, const QScriptValue &value,
                     const PropertyFlags &flags = KeepExistingFlags);

    QScriptValue property(quint32 arrayIndex,
                          const ResolveFlags &mode = ResolvePrototype) const;
    void setProperty(quint32 arrayIndex, const QScriptValue &value,
                     const PropertyFlags &flags = KeepExistingFlags);

    QScriptValue property(const QScriptString &name,
                          const ResolveFlags &mode = ResolvePrototype) const;
    void setProperty(const QScriptString &name, const QScriptValue &value,
                     const PropertyFlags &flags = KeepExistingFlags);

    QScriptValue::PropertyFlags propertyFlags(
        const QString &name, const ResolveFlags &mode = ResolvePrototype) const;
    QScriptValue::PropertyFlags propertyFlags(
        const QScriptString &name, const ResolveFlags &mode = ResolvePrototype) const;

    QScriptValue call(const QScriptValue &thisObject = QScriptValue(),
                      const QScriptValueList &args = QScriptValueList());
    QScriptValue call(const QScriptValue &thisObject,
                      const QScriptValue &arguments);
    QScriptValue construct(const QScriptValueList &args = QScriptValueList());
    QScriptValue construct(const QScriptValue &arguments);

    QScriptValue data() const;
    void setData(const QScriptValue &data);

    QScriptClass *scriptClass() const;
    void setScriptClass(QScriptClass *scriptClass);

    qint64 objectId() const;

private:
    // force compile error, prevent QScriptValue(bool) to be called
    inline QScriptValue(void *) { Q_ASSERT(false); }
    // force compile error, prevent QScriptValue(QScriptEngine*, bool) to be called
    inline QScriptValue(QScriptEngine *, void *) { Q_ASSERT(false); }

private:
    QScriptValuePrivate *d_ptr;

    Q_DECLARE_PRIVATE(QScriptValue)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QScriptValue::ResolveFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QScriptValue::PropertyFlags)

QT_END_NAMESPACE

QT_END_HEADER


#endif
