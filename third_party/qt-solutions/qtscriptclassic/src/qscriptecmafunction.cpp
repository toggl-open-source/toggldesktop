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


#include "qscriptecmafunction_p.h"


#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

#include <QtDebug>

#ifndef QT_NO_QOBJECT
#   include "qscriptextqobject_p.h"
#   include <QMetaMethod>
#endif

QT_BEGIN_NAMESPACE

namespace QScript { namespace Ecma {

class FunctionClassData: public QScriptClassData
{
    QScriptClassInfo *m_classInfo;

public:
    FunctionClassData(QScriptClassInfo *classInfo);
    virtual ~FunctionClassData();

    inline QScriptClassInfo *classInfo() const
        { return m_classInfo; }

    virtual bool resolve(const QScriptValueImpl &object,
                         QScriptNameIdImpl *nameId,
                         QScript::Member *member, QScriptValueImpl *base,
                         QScript::AccessMode access);
    virtual bool get(const QScriptValueImpl &obj, const Member &m,
                     QScriptValueImpl *out_value);
    virtual bool put(QScriptValueImpl *object, const QScript::Member &member,
                     const QScriptValueImpl &value);
    virtual void mark(const QScriptValueImpl &object, int generation);
};

FunctionClassData::FunctionClassData(QScriptClassInfo *classInfo)
    : m_classInfo(classInfo)
{
}

FunctionClassData::~FunctionClassData()
{
}

bool FunctionClassData::resolve(const QScriptValueImpl &object,
                                QScriptNameIdImpl *nameId,
                                QScript::Member *member, QScriptValueImpl *base,
                                QScript::AccessMode /*access*/)
{
    if (object.classInfo() != classInfo())
        return false;

    QScriptEnginePrivate *eng = object.engine();

    if ((nameId == eng->idTable()->id_length)
        || (nameId == eng->idTable()->id_arguments)) {
        member->native(nameId, /*id=*/ 0,
                       QScriptValue::Undeletable
                       | QScriptValue::ReadOnly
                       | QScriptValue::SkipInEnumeration);
        *base = object;
        return true;
    }

    return false;
}

bool FunctionClassData::get(const QScriptValueImpl &object, const Member &member,
                            QScriptValueImpl *result)
{
    if (object.classInfo() != classInfo())
        return false;

    QScriptEnginePrivate *eng = object.engine();
    if (! member.isNativeProperty())
        return false;

    if (member.nameId() == eng->idTable()->id_length) {
        *result = QScriptValueImpl(object.toFunction()->length);
        return true;
    } else if (member.nameId() == eng->idTable()->id_arguments) {
        *result = eng->nullValue();
        return true;
    }

    return false;
}

bool FunctionClassData::put(QScriptValueImpl *, const QScript::Member &,
                            const QScriptValueImpl &)
{
    return false;
}

void FunctionClassData::mark(const QScriptValueImpl &object, int generation)
{
    if (object.classInfo() != classInfo())
        return;
    QScriptFunction *fun = object.toFunction();
    QScriptEnginePrivate *eng = object.engine();
    fun->mark(eng, generation);
}

Function::Function(QScriptEnginePrivate *eng, QScriptClassInfo *classInfo):
    Core(eng, classInfo)
{
    publicPrototype = eng->createFunction(method_void, 0, classInfo); // public prototype
}

Function::~Function()
{
}

void Function::initialize()
{
    QScriptEnginePrivate *eng = engine();
    eng->newConstructor(&ctor, this, publicPrototype);

    addPrototypeFunction(QLatin1String("toString"), method_toString, 1);
    addPrototypeFunction(QLatin1String("apply"), method_apply, 2);
    addPrototypeFunction(QLatin1String("call"), method_call, 1);
    addPrototypeFunction(QLatin1String("connect"), method_connect, 1);
    addPrototypeFunction(QLatin1String("disconnect"), method_disconnect, 1);

    classInfo()->setData(new FunctionClassData(classInfo()));
}

void Function::execute(QScriptContextPrivate *context)
{
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionEntry(context);
#endif
    int lineNumber = context->currentLine;
    QString contents = buildFunction(context);
    engine()->evaluate(context, contents, lineNumber);
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionExit(context);
#endif
}

QString Function::buildFunction(QScriptContextPrivate *context)
{
    int argc = context->argumentCount();

    QString code;
    code += QLatin1String("function(");

    // the formals
    for (int i = 0; i < argc - 1; ++i) {
        if (i != 0)
            code += QLatin1Char(',');

        code += context->argument(i).toString();
    }

    code += QLatin1String("){");

    // the function body
    if (argc != 0)
        code += context->argument(argc - 1).toString();

    code += QLatin1String("\n}");

    return code;
}

void Function::newFunction(QScriptValueImpl *result, QScriptFunction *foo)
{
    engine()->newFunction(result, foo);
}

QScriptValueImpl Function::method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    QScriptValueImpl self = context->thisObject();
    if (QScriptFunction *foo = self.toFunction()) {
        QString code = foo->toString(context);
        return QScriptValueImpl(eng, code);
    }

    return throwThisObjectTypeError(
        context, QLatin1String("Function.prototype.toString"));
}

QScriptValueImpl Function::method_call(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    if (! context->thisObject().isFunction()) {
        return throwThisObjectTypeError(
            context, QLatin1String("Function.prototype.call"));
    }

    QScriptValueImpl thisObject = eng->toObject(context->argument(0));
    if (! (thisObject.isValid () && thisObject.isObject()))
        thisObject = eng->globalObject();

    QScriptValueImplList args;
    for (int i = 1; i < context->argumentCount(); ++i)
        args << context->argument(i);

    return context->thisObject().call(thisObject, args);
}

QScriptValueImpl Function::method_apply(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    if (! context->thisObject().isFunction()) {
        return throwThisObjectTypeError(
            context, QLatin1String("Function.prototype.apply"));
    }

    QScriptValueImpl thisObject = eng->toObject(context->argument(0));
    if (! (thisObject.isValid () && thisObject.isObject()))
        thisObject = eng->globalObject();

    QScriptValueImplList args;
    QScriptValueImpl undefined = eng->undefinedValue();

    QScriptValueImpl arg = context->argument(1);

    if (Ecma::Array::Instance *arr = eng->arrayConstructor->get(arg)) {
        QScript::Array actuals = arr->value;

        for (quint32 i = 0; i < actuals.count(); ++i) {
            QScriptValueImpl a = actuals.at(i);
            if (! a.isValid())
                args << undefined;
            else
                args << a;
        }
    } else if (arg.classInfo() == eng->m_class_arguments) {
        QScript::ArgumentsObjectData *arguments;
        arguments = static_cast<QScript::ArgumentsObjectData*> (arg.objectData());
        QScriptObject *activation = arguments->activation.objectValue();
        for (uint i = 0; i < arguments->length; ++i)
            args << activation->m_values[i];
    } else if (!(arg.isUndefined() || arg.isNull())) {
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Function.prototype.apply: second argument is not an array"));
    }

    return context->thisObject().call(thisObject, args);
}

QScriptValueImpl Function::method_void(QScriptContextPrivate *, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    return eng->undefinedValue();
}

QScriptValueImpl Function::method_disconnect(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
#ifndef QT_NO_QOBJECT
    if (context->argumentCount() == 0) {
        return context->throwError(
            QLatin1String("Function.prototype.disconnect: no arguments given"));
    }

    QScriptValueImpl self = context->thisObject();
    QScriptFunction *fun = self.toFunction();
    if ((fun == 0) || (fun->type() != QScriptFunction::Qt)) {
        return context->throwError(
            QScriptContext::TypeError,
            QLatin1String("Function.prototype.disconnect: this object is not a signal"));
    }

    QtFunction *qtSignal = static_cast<QtFunction*>(fun);

    const QMetaObject *meta = qtSignal->metaObject();
    if (!meta) {
        return context->throwError(
            QScriptContext::TypeError,
            QString::fromLatin1("Function.prototype.disconnect: cannot disconnect from deleted QObject"));
    }

    QMetaMethod sig = meta->method(qtSignal->initialIndex());
    if (sig.methodType() != QMetaMethod::Signal) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("Function.prototype.disconnect: %0::%1 is not a signal")
            .arg(QLatin1String(qtSignal->metaObject()->className()))
            .arg(QLatin1String(sig.signature())));
    }

    QScriptValueImpl receiver;
    QScriptValueImpl slot;
    QScriptValueImpl arg0 = context->argument(0);
    if (context->argumentCount() < 2) {
        receiver = QScriptValueImpl();
        slot = arg0;
    } else {
        receiver = arg0;
        QScriptValueImpl arg1 = context->argument(1);
        if (arg1.isFunction())
            slot = arg1;
        else
            slot = receiver.property(arg1.toString(), QScriptValue::ResolvePrototype);
    }

    if (!slot.isFunction()) {
        return context->throwError(
            QScriptContext::TypeError,
            QLatin1String("Function.prototype.disconnect: target is not a function"));
    }

    bool ok = eng->scriptDisconnect(self, receiver, slot);
    if (!ok) {
        return context->throwError(
            QString::fromLatin1("Function.prototype.disconnect: failed to disconnect from %0::%1")
            .arg(QLatin1String(qtSignal->metaObject()->className()))
            .arg(QLatin1String(sig.signature())));
    }
    return eng->undefinedValue();
#else
    Q_UNUSED(eng);
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Function.prototype.disconnect"));
#endif // QT_NO_QOBJECT
}

QScriptValueImpl Function::method_connect(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    Q_UNUSED(classInfo);

#ifndef QT_NO_QOBJECT
    if (context->argumentCount() == 0) {
        return context->throwError(
            QLatin1String("Function.prototype.connect: no arguments given"));
    }

    QScriptValueImpl self = context->thisObject();
    QScriptFunction *fun = self.toFunction();
    if ((fun == 0) || (fun->type() != QScriptFunction::Qt)) {
        return context->throwError(
            QScriptContext::TypeError,
            QLatin1String("Function.prototype.connect: this object is not a signal"));
    }

    QtFunction *qtSignal = static_cast<QtFunction*>(fun);

    const QMetaObject *meta = qtSignal->metaObject();
    if (!meta) {
        return context->throwError(
            QScriptContext::TypeError,
            QString::fromLatin1("Function.prototype.connect: cannot connect to deleted QObject"));
    }

    QMetaMethod sig = meta->method(qtSignal->initialIndex());
    if (sig.methodType() != QMetaMethod::Signal) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("Function.prototype.connect: %0::%1 is not a signal")
            .arg(QLatin1String(qtSignal->metaObject()->className()))
            .arg(QLatin1String(sig.signature())));
    }

    {
        QList<int> overloads = qtSignal->overloadedIndexes();
        if (!overloads.isEmpty()) {
            overloads.append(qtSignal->initialIndex());
            QByteArray signature = sig.signature();
            QString message = QString::fromLatin1("Function.prototype.connect: ambiguous connect to %0::%1(); candidates are\n")
                              .arg(QLatin1String(qtSignal->metaObject()->className()))
                              .arg(QLatin1String(signature.left(signature.indexOf('('))));
            for (int i = 0; i < overloads.size(); ++i) {
                QMetaMethod mtd = meta->method(overloads.at(i));
                message.append(QString::fromLatin1("    %0\n").arg(QString::fromLatin1(mtd.signature())));
            }
            message.append(QString::fromLatin1("Use e.g. object['%0'].connect() to connect to a particular overload")
                           .arg(QLatin1String(signature)));
            return context->throwError(message);
        }
    }

    QScriptValueImpl receiver;
    QScriptValueImpl slot;
    QScriptValueImpl arg0 = context->argument(0);
    if (context->argumentCount() < 2) {
        receiver = QScriptValueImpl();
        slot = arg0;
    } else {
        receiver = arg0;
        QScriptValueImpl arg1 = context->argument(1);
        if (arg1.isFunction())
            slot = arg1;
        else
            slot = receiver.property(arg1.toString(), QScriptValue::ResolvePrototype);
    }

    if (!slot.isFunction()) {
        return context->throwError(
            QScriptContext::TypeError,
            QLatin1String("Function.prototype.connect: target is not a function"));
    }

    bool ok = eng->scriptConnect(self, receiver, slot, Qt::AutoConnection);
    if (!ok) {
        return context->throwError(
            QString::fromLatin1("Function.prototype.connect: failed to connect to %0::%1")
            .arg(QLatin1String(qtSignal->metaObject()->className()))
            .arg(QLatin1String(sig.signature())));
    }
    return eng->undefinedValue();
#else
    Q_UNUSED(eng);
    Q_UNUSED(classInfo);
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Function.prototype.connect"));
#endif // QT_NO_QOBJECT
}

} } // namespace QScript::Ecma

QT_END_NAMESPACE

