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


#ifndef QSCRIPTCONTEXT_P_H
#define QSCRIPTCONTEXT_P_H

#include "qscriptcontextfwd_p.h"


#include "qscriptenginefwd_p.h"
#include "qscriptnameid_p.h"

#include <qnumeric.h>

QT_BEGIN_NAMESPACE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

inline QScriptContextPrivate::QScriptContextPrivate()
    : previous(0),
      argc(0),
      m_state(QScriptContext::NormalState),
      args(0),
      tempStack(0),
      stackPtr(0),
      m_code(0),
      iPtr(0),
      firstInstruction(0),
      lastInstruction(0),
      currentLine(0),
      currentColumn(0),
      errorLineNumber(0),
      catching(false),
      m_calledAsConstructor(false),
      calleeMetaIndex(0),
      q_ptr(0)
{
}

inline QScriptContextPrivate *QScriptContextPrivate::get(QScriptContext *q)
{
    if (q)
        return q->d_func();
    return 0;
}

inline const QScriptContextPrivate *QScriptContextPrivate::get(const QScriptContext *q)
{
    if (q)
        return q->d_func();
    return 0;
}

inline QScriptContext *QScriptContextPrivate::create()
{
    return new QScriptContext;
}

inline QScriptEnginePrivate *QScriptContextPrivate::engine() const
{
    return m_activation.engine();
}

inline QScriptContextPrivate *QScriptContextPrivate::parentContext() const
{
    return previous;
}

inline void QScriptContextPrivate::init(QScriptContextPrivate *parent)
{
    m_state = QScriptContext::NormalState;
    previous = parent;
    args = 0;
    argc = 0;
    m_code = 0;
    iPtr = firstInstruction = lastInstruction = 0;
    stackPtr = tempStack = (parent != 0) ? parent->stackPtr : 0;
    m_activation.invalidate();
    m_thisObject.invalidate();
    m_result.invalidate();
    m_scopeChain.invalidate();
    m_callee.invalidate();
    m_arguments.invalidate();
    currentLine = -1;
    currentColumn = -1;
    errorLineNumber = -1;
    m_calledAsConstructor = false;
}

inline QScriptValueImpl QScriptContextPrivate::argument(int index) const
{
    if (index >= argc)
        return engine()->undefinedValue();

    Q_ASSERT(args != 0);
    return args[index];
}

inline int QScriptContextPrivate::argumentCount() const
{
    return argc;
}

inline QScriptValueImpl QScriptContextPrivate::argumentsObject() const
{
    if (!m_arguments.isValid() && m_activation.isValid()) {
        QScriptContextPrivate *dd = const_cast<QScriptContextPrivate*>(this);
        engine()->newArguments(&dd->m_arguments, m_activation,
                               argc, m_callee);
    }
    return m_arguments;
}

inline void QScriptContextPrivate::throwException()
{
    m_state = QScriptContext::ExceptionState;
}

inline bool QScriptContextPrivate::hasUncaughtException() const
{
    return m_state == QScriptContext::ExceptionState;
}

inline void QScriptContextPrivate::recover()
{
    m_state = QScriptContext::NormalState;
    errorLineNumber = -1;
}

inline bool QScriptContextPrivate::isNumerical(const QScriptValueImpl &v)
{
    switch (v.type()) {
    case QScript::BooleanType:
    case QScript::IntegerType:
    case QScript::NumberType:
        return true;

    default:
        return false;
    }
}

inline bool QScriptContextPrivate::eq_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs)
{
    if (lhs.type() == rhs.type()) {
        switch (lhs.type()) {
        case QScript::InvalidType:
        case QScript::UndefinedType:
        case QScript::NullType:
            return true;

        case QScript::NumberType:
            return lhs.m_number_value == rhs.m_number_value;

        case QScript::ReferenceType:
        case QScript::IntegerType:
            return lhs.m_int_value == rhs.m_int_value;

        case QScript::BooleanType:
            return lhs.m_bool_value == rhs.m_bool_value;

        case QScript::StringType:
            if (lhs.m_string_value->unique && rhs.m_string_value->unique)
                return lhs.m_string_value == rhs.m_string_value;
            return lhs.m_string_value->s == rhs.m_string_value->s;

        case QScript::PointerType:
            return lhs.m_ptr_value == rhs.m_ptr_value;

        case QScript::ObjectType:
            if (lhs.isVariant())
                return lhs.m_object_value == rhs.m_object_value || lhs.toVariant() == rhs.toVariant();
#ifndef QT_NO_QOBJECT
            else if (lhs.isQObject())
                return lhs.m_object_value == rhs.m_object_value || lhs.toQObject() == rhs.toQObject();
#endif
            else
                return lhs.m_object_value == rhs.m_object_value;

        case QScript::LazyStringType:
            return *lhs.m_lazy_string_value == *rhs.m_lazy_string_value;
        }
    }

    return eq_cmp_helper(lhs, rhs);
}

inline bool QScriptContextPrivate::strict_eq_cmp( const QScriptValueImpl &lhs, const QScriptValueImpl &rhs)
{
    if (lhs.type() != rhs.type())
        return false;

    switch (lhs.type()) {
    case QScript::InvalidType:
    case QScript::UndefinedType:
    case QScript::NullType:
        return true;

    case QScript::NumberType:
        if (qIsNaN(lhs.m_number_value) || qIsNaN(rhs.m_number_value))
            return false;
        return lhs.m_number_value == rhs.m_number_value;

    case QScript::IntegerType:
        return lhs.m_int_value == rhs.m_int_value;

    case QScript::BooleanType:
        return lhs.m_bool_value == rhs.m_bool_value;

    case QScript::StringType:
        if (lhs.m_string_value->unique && rhs.m_string_value->unique)
            return lhs.m_string_value == rhs.m_string_value;
        return lhs.m_string_value->s == rhs.m_string_value->s;

    case QScript::ObjectType:
        return lhs.m_object_value == rhs.m_object_value;

    case QScript::ReferenceType:
        return lhs.m_int_value == rhs.m_int_value;

    case QScript::PointerType:
        return lhs.m_ptr_value == rhs.m_ptr_value;

    case QScript::LazyStringType:
        return *lhs.m_lazy_string_value == *rhs.m_lazy_string_value;
    }

    return false;
}

inline QScriptValueImpl QScriptContextPrivate::throwTypeError(const QString &text)
{
    return throwError(QScriptContext::TypeError, text);
}

inline QScriptValueImpl QScriptContextPrivate::throwSyntaxError(const QString &text)
{
    return throwError(QScriptContext::SyntaxError, text);
}

inline QScriptValueImpl QScriptContextPrivate::thisObject() const
{
    return m_thisObject;
}

inline void QScriptContextPrivate::setThisObject(const QScriptValueImpl &object)
{
    m_thisObject = object;
}

inline QScriptValueImpl QScriptContextPrivate::callee() const
{
    return m_callee;
}

inline bool QScriptContextPrivate::isCalledAsConstructor() const
{
    return m_calledAsConstructor;
}

inline QScriptValueImpl QScriptContextPrivate::returnValue() const
{
    return m_result;
}

inline void QScriptContextPrivate::setReturnValue(const QScriptValueImpl &value)
{
    m_result = value;
}

inline QScriptValueImpl QScriptContextPrivate::activationObject() const
{
    if (previous && !m_activation.property(QLatin1String("arguments")).isValid()) {
        QScriptContextPrivate *dd = const_cast<QScriptContextPrivate*>(this);
        dd->m_activation.setProperty(QLatin1String("arguments"), argumentsObject());
    }
    return m_activation;
}

inline void QScriptContextPrivate::setActivationObject(const QScriptValueImpl &activation)
{
    m_activation = activation;
}

inline const QScriptInstruction *QScriptContextPrivate::instructionPointer()
{
    return iPtr;
}

inline void QScriptContextPrivate::setInstructionPointer(const QScriptInstruction *instructionPointer)
{
    iPtr = instructionPointer;
}

inline const QScriptValueImpl *QScriptContextPrivate::baseStackPointer() const
{
    return tempStack;
}

inline const QScriptValueImpl *QScriptContextPrivate::currentStackPointer() const
{
    return stackPtr;
}

inline QScriptContext::ExecutionState QScriptContextPrivate::state() const
{
    return m_state;
}

QT_END_NAMESPACE

#endif
