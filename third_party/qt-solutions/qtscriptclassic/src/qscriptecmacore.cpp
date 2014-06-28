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


#include "qscriptecmacore_p.h"


#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

QT_BEGIN_NAMESPACE

namespace QScript { namespace Ecma {

Core::Core(QScriptEnginePrivate *engine, const QString &className,
           QScriptClassInfo::Type type)
    : m_engine(engine)
{
    m_classInfo = engine->registerClass(className, type);
    this->length = 1;
}

Core::Core(QScriptEnginePrivate *engine, QScriptClassInfo *classInfo)
    : m_engine(engine), m_classInfo(classInfo)
{
    this->length = 1;
}

Core::~Core()
{
}

void Core::addPrototypeFunction(const QString &name, QScriptInternalFunctionSignature fun,
                                int length, const QScriptValue::PropertyFlags flags)
{
    addFunction(publicPrototype, name, fun, length, flags);
}

void Core::addConstructorFunction(const QString &name, QScriptInternalFunctionSignature fun,
                                  int length, const QScriptValue::PropertyFlags flags)
{
    addFunction(ctor, name, fun, length, flags);
}

void Core::addFunction(QScriptValueImpl &object, const QString &name,
                       QScriptInternalFunctionSignature fun, int length,
                       const QScriptValue::PropertyFlags flags)
{
    QScriptValueImpl val = engine()->createFunction(fun, length, m_classInfo, name);
    object.setProperty(name, val, flags);
}

QString Core::functionName() const
{
    return m_classInfo->name();
}

void Core::mark(QScriptEnginePrivate *eng, int generation)
{
    QScriptFunction::mark(eng, generation);
    eng->markObject(ctor, generation);
    eng->markObject(publicPrototype, generation);
}

QScriptValueImpl Core::throwThisObjectTypeError(QScriptContextPrivate *context,
                                                const QString &functionName)
{
    return context->throwError(QScriptContext::TypeError,
                               QString::fromLatin1("%0 called on incompatible object")
                               .arg(functionName));
}

} // namespace Ecma

} // namespace QScript

QT_END_NAMESPACE

