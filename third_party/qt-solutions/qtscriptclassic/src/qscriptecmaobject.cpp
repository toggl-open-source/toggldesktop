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


#include "qscriptecmaobject_p.h"


#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

#include <QtDebug>

QT_BEGIN_NAMESPACE

namespace QScript { namespace Ecma {

Object::Object(QScriptEnginePrivate *eng, QScriptClassInfo *classInfo):
    Core(eng, classInfo)
{
    newObject(&publicPrototype, eng->nullValue());
}

Object::~Object()
{
}

void Object::initialize()
{
    QScriptEnginePrivate *eng = engine();

    eng->newConstructor(&ctor, this, publicPrototype);

    addPrototypeFunction(QLatin1String("toString"), method_toString, 1);
    addPrototypeFunction(QLatin1String("toLocaleString"), method_toLocaleString, 1);
    addPrototypeFunction(QLatin1String("valueOf"), method_valueOf, 0);
    addPrototypeFunction(QLatin1String("hasOwnProperty"), method_hasOwnProperty, 1);
    addPrototypeFunction(QLatin1String("isPrototypeOf"), method_isPrototypeOf, 1);
    addPrototypeFunction(QLatin1String("propertyIsEnumerable"), method_propertyIsEnumerable, 1);
    addPrototypeFunction(QLatin1String("__defineGetter__"), method_defineGetter, 2);
    addPrototypeFunction(QLatin1String("__defineSetter__"), method_defineSetter, 2);
}

void Object::execute(QScriptContextPrivate *context)
{
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionEntry(context);
#endif
    QScriptValueImpl value;

    if (context->argumentCount() > 0)
        value = engine()->toObject(context->argument(0));
    else
        value.invalidate();

    if (! value.isValid())
        newObject(&value);

    context->setReturnValue(value);
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionExit(context);
#endif
}

void Object::newObject(QScriptValueImpl *result, const QScriptValueImpl &proto)
{
    engine()->newObject(result, proto, classInfo());
}

QScriptValueImpl Object::method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    QScriptValueImpl glo = eng->globalObject();
    QString s = QLatin1String("[object ");
    QScriptValueImpl self = context->thisObject();
    s += self.classInfo()->name();
    s += QLatin1Char(']');
    return (QScriptValueImpl(eng, s));
}

QScriptValueImpl Object::method_toLocaleString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    return method_toString(context, eng, classInfo);
}

QScriptValueImpl Object::method_valueOf(QScriptContextPrivate *context, QScriptEnginePrivate *, QScriptClassInfo *)
{
    return (context->thisObject());
}

QScriptValueImpl Object::method_hasOwnProperty(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    bool result = false;

    if (context->thisObject().isObject() && (context->argumentCount() > 0)) {
        QScriptValueImpl arg = context->argument(0);

        QScriptNameIdImpl *id = 0;
        if (arg.isString())
            id = arg.stringValue();

        if (! id || ! id->unique) {
            QString str = arg.toString();
            id = eng->nameId(str);
        }

        QScript::Member member;
        QScriptValueImpl base;
        QScriptValueImpl self = context->thisObject();
        if (self.resolve(id, &member, &base, QScriptValue::ResolveLocal, QScript::Read))
            result = true;
    }

    return (QScriptValueImpl(result));
}

QScriptValueImpl Object::method_isPrototypeOf(QScriptContextPrivate *context, QScriptEnginePrivate *, QScriptClassInfo *)
{
    bool result = false;

    if (context->thisObject().isObject() && (context->argumentCount() > 0)) {
        QScriptValueImpl arg = context->argument(0);

        if (arg.isObject()) {
            QScriptValueImpl proto = arg.prototype();

            if (proto.isObject()) {
                QScriptValueImpl self = context->thisObject();
                result = self.objectValue() == proto.objectValue();
            }
        }
    }

    return (QScriptValueImpl(result));
}

QScriptValueImpl Object::method_propertyIsEnumerable(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    bool result = false;

    if (context->thisObject().isObject() && (context->argumentCount() > 0)) {
        QScriptValueImpl arg = context->argument(0);

        QScriptNameIdImpl *id = 0;
        if (arg.isString())
            id = arg.stringValue();

        if (! id || ! id->unique) {
            QString str = arg.toString();
            id = eng->nameId(str);
        }

        QScript::Member member;
        QScriptValueImpl base;
        QScriptValueImpl self = context->thisObject();
        if (self.resolve(id, &member, &base, QScriptValue::ResolveLocal, QScript::Read)) {
            result = ! member.dontEnum();
            if (result) {
                QScriptValueImpl tmp;
                base.get(member, &tmp);
                result = tmp.isValid();
            }
        }
    }

    return (QScriptValueImpl(result));
}

QScriptValueImpl Object::method_defineGetter(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                             QScriptClassInfo *)
{
    QString propertyName = context->argument(0).toString();
    if (context->thisObject().propertyFlags(propertyName) & QScriptValue::ReadOnly)
        return context->throwError(QLatin1String("cannot redefine read-only property"));
    QScriptValueImpl getter = context->argument(1);
    if (!getter.isFunction())
        return context->throwError(QLatin1String("getter must be a function"));
    context->thisObject().setProperty(propertyName, getter, QScriptValue::PropertyGetter);
    return eng->undefinedValue();
}

QScriptValueImpl Object::method_defineSetter(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                             QScriptClassInfo *)
{
    QString propertyName = context->argument(0).toString();
    if (context->thisObject().propertyFlags(propertyName) & QScriptValue::ReadOnly)
        return context->throwError(QLatin1String("cannot redefine read-only property"));
    QScriptValueImpl setter = context->argument(1);
    if (!setter.isFunction())
        return context->throwError(QLatin1String("setter must be a function"));
    context->thisObject().setProperty(propertyName, setter, QScriptValue::PropertySetter);
    return eng->undefinedValue();
}

} } // namespace QScript::Ecma

QT_END_NAMESPACE

