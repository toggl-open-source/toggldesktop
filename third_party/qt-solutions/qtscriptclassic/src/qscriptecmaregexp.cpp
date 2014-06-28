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


#include "qscriptecmaregexp_p.h"


#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

#include <QStringList>
#include <QRegExp>
#include <QtDebug>

QT_BEGIN_NAMESPACE

namespace QScript { namespace Ecma {

RegExp::RegExp(QScriptEnginePrivate *eng):
    Core(eng, QLatin1String("RegExp"), QScriptClassInfo::RegExpType)
{
    newRegExp(&publicPrototype, QString(), /*flags=*/0);

    eng->newConstructor(&ctor, this, publicPrototype);

    addPrototypeFunction(QLatin1String("exec"), method_exec, 1);
    addPrototypeFunction(QLatin1String("test"), method_test, 1);
    addPrototypeFunction(QLatin1String("toString"), method_toString, 1);
}

RegExp::~RegExp()
{
}

RegExp::Instance *RegExp::Instance::get(const QScriptValueImpl &object, QScriptClassInfo *klass)
{
    if (! klass || klass == object.classInfo())
        return static_cast<Instance*> (object.objectData());

    return 0;
}

void RegExp::execute(QScriptContextPrivate *context)
{
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionEntry(context);
#endif
    QString P;
    int F;
    QScriptValueImpl pattern = context->argument(0);
    QScriptValueImpl flags = context->argument(1);
    if (!context->isCalledAsConstructor()) {
        if ((pattern.classInfo() == classInfo()) && flags.isUndefined()) {
            context->m_result = pattern;
            goto Lout;
        }
    }
    if (pattern.classInfo() == classInfo()) {
        if (!flags.isUndefined()) {
            context->throwTypeError(QString::fromLatin1("cannot specify flags when creating a copy of a RegExp"));
            goto Lout;
        }
        Instance *data = Instance::get(pattern, classInfo());
#ifndef QT_NO_REGEXP
        P = data->value.pattern();
#else
        P = data->pattern;
#endif
        F = data->flags;
    } else {
        if (!pattern.isUndefined())
            P = pattern.toString();
        F = 0;
        if (!flags.isUndefined()) {
            QString flagsStr = flags.toString();
            for (int i = 0; i < flagsStr.length(); ++i) {
                int bitflag = flagFromChar(flagsStr.at(i));
                if (bitflag == 0) {
                    context->throwError(
                        QScriptContext::SyntaxError,
                        QString::fromUtf8("invalid regular expression flag '%0'")
                        .arg(flagsStr.at(i)));
                    goto Lout;
                }
                F |= bitflag;
            }
        }
    }
    if (context->isCalledAsConstructor()) {
        QScriptValueImpl &object = context->m_thisObject;
        object.setClassInfo(classInfo());
        object.setPrototype(publicPrototype);
#ifndef QT_NO_REGEXP
        initRegExp(&object, toRegExp(P, F), F);
#else
        initRegExp(&object, P, F);
#endif
    } else {
        newRegExp(&context->m_result, P, F);
    }
 Lout: ;
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionExit(context);
#endif
}

void RegExp::newRegExp(QScriptValueImpl *result, const QString &pattern, int flags)
{
#ifndef QT_NO_REGEXP
    QRegExp rx = toRegExp(pattern, flags);
    newRegExp_helper(result, rx, flags);
#else
    engine()->newObject(result, publicPrototype, classInfo());
    initRegExp(result, pattern, flags);
#endif // QT_NO_REGEXP
}

#ifndef QT_NO_REGEXP
void RegExp::newRegExp(QScriptValueImpl *result, const QRegExp &rx, int flags)
{
    Q_ASSERT(!(flags & IgnoreCase) || (rx.caseSensitivity() == Qt::CaseInsensitive));
    newRegExp_helper(result, rx, flags);
}

void RegExp::newRegExp_helper(QScriptValueImpl *result, const QRegExp &rx,
                              int flags)
{
    engine()->newObject(result, publicPrototype, classInfo());
    initRegExp(result, rx, flags);
}

QRegExp RegExp::toRegExp(const QScriptValueImpl &value) const
{
    Instance *rx_data = Instance::get(value, classInfo());
    Q_ASSERT(rx_data != 0);
    return rx_data->value;
}

QRegExp RegExp::toRegExp(const QString &pattern, int flags)
{
    bool ignoreCase = (flags & IgnoreCase) != 0;
    return QRegExp(pattern,
                   (ignoreCase ? Qt::CaseInsensitive: Qt::CaseSensitive),
                   QRegExp::RegExp2);
}

#endif // QT_NO_REGEXP

void RegExp::initRegExp(QScriptValueImpl *result,
#ifndef QT_NO_REGEXP
                        const QRegExp &rx,
#else
                        const QString &pattern,
#endif
                        int flags)
{
    Instance *instance = new Instance();
#ifndef QT_NO_REGEXP
    instance->value = rx;
#else
    instance->pattern = pattern;
#endif
    instance->flags = flags;
    result->setObjectData(instance);

    bool global = (flags & Global) != 0;
    bool ignoreCase = (flags & IgnoreCase) != 0;
    bool multiline = (flags & Multiline) != 0;

    QScriptValue::PropertyFlags propertyFlags = QScriptValue::SkipInEnumeration
                                                | QScriptValue::Undeletable
                                                | QScriptValue::ReadOnly;

    result->setProperty(QLatin1String("global"), QScriptValueImpl(global),
                        propertyFlags);
    result->setProperty(QLatin1String("ignoreCase"), QScriptValueImpl(ignoreCase),
                        propertyFlags);
    result->setProperty(QLatin1String("multiline"), QScriptValueImpl(multiline),
                        propertyFlags);
#ifndef QT_NO_REGEXP
    const QString &pattern = rx.pattern();
#endif
    result->setProperty(QLatin1String("source"), QScriptValueImpl(engine(), pattern),
                        propertyFlags);
    result->setProperty(QLatin1String("lastIndex"), QScriptValueImpl(0),
                        propertyFlags & ~QScriptValue::ReadOnly);
}

int RegExp::flagFromChar(const QChar &ch)
{
    static QHash<QChar, int> flagsHash;
    if (flagsHash.isEmpty()) {
        flagsHash[QLatin1Char('g')] = Global;
        flagsHash[QLatin1Char('i')] = IgnoreCase;
        flagsHash[QLatin1Char('m')] = Multiline;
    }
    QHash<QChar, int>::const_iterator it;
    it = flagsHash.constFind(ch);
    if (it == flagsHash.constEnd())
        return 0;
    return it.value();
}

QString RegExp::flagsToString(int flags)
{
    QString result;
    if (flags & Global)
        result += QLatin1Char('g');
    if (flags & IgnoreCase)
        result += QLatin1Char('i');
    if (flags & Multiline)
        result += QLatin1Char('m');
    return result;
}

QScriptValueImpl RegExp::method_exec(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo) {
        return throwThisObjectTypeError(
            context, QLatin1String("RegExp.prototype.exec"));
    }
    Instance *rx_data = Instance::get(self, classInfo);
    Q_ASSERT(rx_data != 0);

    QString S = context->argument(0).toString();
    int length = S.length();
    QScriptValueImpl lastIndex = self.property(QLatin1String("lastIndex"));

    int i = lastIndex.isValid() ? int (lastIndex.toInteger()) : 0;
    bool global = self.property(QLatin1String("global")).toBoolean();

    if (! global)
        i = 0;

    if (i < 0 || i >= length)
        return (eng->nullValue());

#ifndef QT_NO_REGEXP
    int index = rx_data->value.indexIn(S, i);
    if (index == -1)
#endif // QT_NO_REGEXP
        return eng->nullValue();

#ifndef QT_NO_REGEXP
    int e = index + rx_data->value.matchedLength();

    if (global)
        self.setProperty(QLatin1String("lastIndex"), QScriptValueImpl(e));

    QScript::Array elts(eng);
    QStringList capturedTexts = rx_data->value.capturedTexts();
    for (int i = 0; i < capturedTexts.count(); ++i)
        elts.assign(i, QScriptValueImpl(eng, capturedTexts.at(i)));

    QScriptValueImpl r = eng->newArray(elts);

    r.setProperty(QLatin1String("index"), QScriptValueImpl(index));
    r.setProperty(QLatin1String("input"), QScriptValueImpl(eng, S));

    return r;
#endif // QT_NO_REGEXP
}

QScriptValueImpl RegExp::method_test(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl r = method_exec(context, eng, classInfo);
    return QScriptValueImpl(!r.isNull());
}

QScriptValueImpl RegExp::method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QString result;
        result += QLatin1Char('/');
#ifndef QT_NO_REGEXP
        const QString &pattern = instance->value.pattern();
#else
        const QString &pattern = instance->pattern;
#endif
        if (pattern.isEmpty())
            result += QLatin1String("(?:)");
        else
            result += pattern; // ### quote
        result += QLatin1Char('/');
        result += flagsToString(instance->flags);
        return (QScriptValueImpl(eng, result));
    }

    return throwThisObjectTypeError(
        context, QLatin1String("RegExp.prototype.toString"));
}

} } // namespace QScript::Ecma

QT_END_NAMESPACE

