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


#include "qscriptecmanumber_p.h"


#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

#include <QtDebug>
#include <qnumeric.h>
#include <math.h>

QT_BEGIN_NAMESPACE

namespace QScript { namespace Ecma {

Number::Number(QScriptEnginePrivate *eng):
    Core(eng, QLatin1String("Number"), QScriptClassInfo::NumberType)
{
    newNumber(&publicPrototype, 0);

    eng->newConstructor(&ctor, this, publicPrototype);

    addPrototypeFunction(QLatin1String("toString"), method_toString, 0);
    addPrototypeFunction(QLatin1String("toLocaleString"), method_toLocaleString, 0);
    addPrototypeFunction(QLatin1String("valueOf"), method_valueOf, 0);
    addPrototypeFunction(QLatin1String("toFixed"), method_toFixed, 1);
    addPrototypeFunction(QLatin1String("toExponential"), method_toExponential, 1);
    addPrototypeFunction(QLatin1String("toPrecision"), method_toPrecision, 1);

    QScriptValue::PropertyFlags flags = QScriptValue::Undeletable
                                        | QScriptValue::ReadOnly
                                        | QScriptValue::SkipInEnumeration;
    ctor.setProperty(QLatin1String("NaN"),
                     QScriptValueImpl(qSNaN()), flags);
    ctor.setProperty(QLatin1String("NEGATIVE_INFINITY"),
                     QScriptValueImpl(-qInf()), flags);
    ctor.setProperty(QLatin1String("POSITIVE_INFINITY"),
                     QScriptValueImpl(qInf()), flags);
    ctor.setProperty(QLatin1String("MAX_VALUE"),
                     QScriptValueImpl(1.7976931348623158e+308), flags);
#ifdef __INTEL_COMPILER
# pragma warning( push )
# pragma warning(disable: 239)
#endif
    ctor.setProperty(QLatin1String("MIN_VALUE"),
                     QScriptValueImpl(5e-324), flags);
#ifdef __INTEL_COMPILER
# pragma warning( pop )
#endif
}

Number::~Number()
{
}

void Number::execute(QScriptContextPrivate *context)
{
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionEntry(context);
#endif
    qsreal value;
    if (context->argumentCount() > 0)
        value = context->argument(0).toNumber();
    else
        value = 0;

    QScriptValueImpl num(value);
    if (!context->isCalledAsConstructor()) {
        context->setReturnValue(num);
    } else {
        QScriptValueImpl &obj = context->m_thisObject;
        obj.setClassInfo(classInfo());
        obj.setInternalValue(num);
        obj.setPrototype(publicPrototype);
        context->setReturnValue(obj);
    }
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionExit(context);
#endif
}

void Number::newNumber(QScriptValueImpl *result, qsreal value)
{
    engine()->newObject(result, publicPrototype, classInfo());
    result->setInternalValue(QScriptValueImpl(value));
}

QScriptValueImpl Number::method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo) {
        return throwThisObjectTypeError(
            context, QLatin1String("Number.prototype.toString"));
    }
    QScriptValueImpl arg = context->argument(0);
    if (!arg.isUndefined()) {
        int radix = arg.toInt32();
        if (radix < 2 || radix > 36)
            return context->throwError(QString::fromLatin1("Number.prototype.toString: %0 is not a valid radix")
                                       .arg(radix));
        if (radix != 10) {
            QString str;
            qsreal num = self.internalValue().toNumber();
            if (qIsNaN(num))
                return QScriptValueImpl(eng, QLatin1String("NaN"));
            else if (qIsInf(num))
                return QScriptValueImpl(eng, QLatin1String(num < 0 ? "-Infinity" : "Infinity"));
            bool negative = false;
            if (num < 0) {
                negative = true;
                num = -num;
            }
            qsreal frac = num - ::floor(num);
            num = QScriptEnginePrivate::toInteger(num);
            do {
                char c = (char)::fmod(num, radix);
                c = (c < 10) ? (c + '0') : (c - 10 + 'a');
                str.prepend(QLatin1Char(c));
                num = ::floor(num / radix);
            } while (num != 0);
            if (frac != 0) {
                str.append(QLatin1Char('.'));
                do {
                    frac = frac * radix;
                    char c = (char)::floor(frac);
                    c = (c < 10) ? (c + '0') : (c - 10 + 'a');
                    str.append(QLatin1Char(c));
                    frac = frac - ::floor(frac);
                } while (frac != 0);
            }
            if (negative)
                str.prepend(QLatin1Char('-'));
            return QScriptValueImpl(eng, str);
        }
    }
    QString str = self.internalValue().toString();
    return (QScriptValueImpl(eng, str));
}

QScriptValueImpl Number::method_toLocaleString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo) {
        return throwThisObjectTypeError(
            context, QLatin1String("Number.prototype.toLocaleString"));
    }
    QString str = self.internalValue().toString();
    return (QScriptValueImpl(eng, str));
}

QScriptValueImpl Number::method_valueOf(QScriptContextPrivate *context, QScriptEnginePrivate *, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo) {
        return throwThisObjectTypeError(
            context, QLatin1String("Number.prototype.valueOf"));
    }
    return (self.internalValue());
}

QScriptValueImpl Number::method_toFixed(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo) {
        return throwThisObjectTypeError(
            context, QLatin1String("Number.prototype.toFixed"));
    }
    qsreal fdigits = 0;

    if (context->argumentCount() > 0)
        fdigits = context->argument(0).toInteger();

    if (qIsNaN(fdigits))
        fdigits = 0;

    qsreal v = self.internalValue().toNumber();
    QString str;
    if (qIsNaN(v))
        str = QString::fromLatin1("NaN");
    else if (qIsInf(v))
        str = QString::fromLatin1(v < 0 ? "-Infinity" : "Infinity");
    else
        str = QString::number(v, 'f', int (fdigits));
    return (QScriptValueImpl(eng, str));
}

QScriptValueImpl Number::method_toExponential(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo) {
        return throwThisObjectTypeError(
            context, QLatin1String("Number.prototype.toExponential"));
    }
    qsreal fdigits = 0;

    if (context->argumentCount() > 0)
        fdigits = context->argument(0).toInteger();

    qsreal v = self.internalValue().toNumber();
    QString z = QString::number(v, 'e', int (fdigits));
    return (QScriptValueImpl(eng, z));
}

QScriptValueImpl Number::method_toPrecision(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo) {
        return throwThisObjectTypeError(
            context, QLatin1String("Number.prototype.toPrecision"));
    }
    qsreal fdigits = 0;

    if (context->argumentCount() > 0)
        fdigits = context->argument(0).toInteger();

    qsreal v = self.internalValue().toNumber();
    return (QScriptValueImpl(eng, QString::number(v, 'g', int (fdigits))));
}

} } // namespace QScript::Ecma

QT_END_NAMESPACE

