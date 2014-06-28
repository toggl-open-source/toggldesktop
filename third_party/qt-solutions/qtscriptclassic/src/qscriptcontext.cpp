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


#include "qscriptcontext.h"


#include "qscriptcontextinfo.h"
#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

QT_BEGIN_NAMESPACE

/*!
  \since 4.3
  \class QScriptContext

  \brief The QScriptContext class represents a Qt Script function invocation.

  \ingroup script
  \mainclass

  A QScriptContext provides access to the `this' object and arguments
  passed to a script function. You typically want to access this
  information when you're writing a native (C++) function (see
  QScriptEngine::newFunction()) that will be called from script
  code. For example, when the script code

  \snippet doc/src/snippets/code/src_script_qscriptcontext.cpp 0

  is evaluated, a QScriptContext will be created, and the context will
  carry the arguments as QScriptValues; in this particular case, the
  arguments will be one QScriptValue containing the number 20.5, a second
  QScriptValue containing the string \c{"hello"}, and a third QScriptValue
  containing a Qt Script object.

  Use argumentCount() to get the number of arguments passed to the
  function, and argument() to get an argument at a certain index. The
  argumentsObject() function returns a Qt Script array object
  containing all the arguments; you can use the QScriptValueIterator
  to iterate over its elements, or pass the array on as arguments to
  another script function using QScriptValue::call().

  Use thisObject() to get the `this' object associated with the function call,
  and setThisObject() to set the `this' object. If you are implementing a
  native "instance method", you typically fetch the thisObject() and access
  one or more of its properties:

  \snippet doc/src/snippets/code/src_script_qscriptcontext.cpp 1

  Use isCalledAsConstructor() to determine if the function was called
  as a constructor (e.g. \c{"new foo()"} (as constructor) or just
  \c{"foo()"}).  When a function is called as a constructor, the
  thisObject() contains the newly constructed object that the function
  is expected to initialize.

  Use throwValue() or throwError() to throw an exception.

  Use callee() to obtain the QScriptValue that represents the function being
  called. This can for example be used to call the function recursively.

  Use parentContext() to get a pointer to the context that precedes
  this context in the activation stack. This is mostly useful for
  debugging purposes (e.g. when constructing some form of backtrace).

  The activationObject() function returns the object that is used to
  hold the local variables associated with this function call. You can
  replace the activation object by calling setActivationObject(). A
  typical usage of these functions is when you want script code to be
  evaluated in the context of the parent context, e.g. to implement an
  include() function:

  \snippet doc/src/snippets/code/src_script_qscriptcontext.cpp 2

  Use backtrace() to get a human-readable backtrace associated with
  this context. This can be useful for debugging purposes when
  implementing native functions. The toString() function provides a
  string representation of the context. (QScriptContextInfo provides
  more detailed debugging-related information about the
  QScriptContext.)

  Use engine() to obtain a pointer to the QScriptEngine that this context
  resides in.

  \sa QScriptContextInfo, QScriptEngine::newFunction(), QScriptable
*/

/*!
    \enum QScriptContext::ExecutionState

    This enum specifies the execution state of the context.

    \value NormalState The context is in a normal state.

    \value ExceptionState The context is in an exceptional state.
*/

/*!
    \enum QScriptContext::Error

    This enum specifies types of error.

    \value ReferenceError A reference error.

    \value SyntaxError A syntax error.

    \value TypeError A type error.

    \value RangeError A range error.

    \value URIError A URI error.

    \value UnknownError An unknown error.
*/

/*!
  Throws an exception with the given \a value.
  Returns the value thrown (the same as the argument).

  \sa throwError(), state()
*/
QScriptValue QScriptContext::throwValue(const QScriptValue &value)
{
    Q_D(QScriptContext);
    d->m_result = d->engine()->toImpl(value);
    d->m_state = QScriptContext::ExceptionState;
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
        d->engine()->notifyException(d);
#endif
    return value;
}

/*!
  Throws an \a error with the given \a text.
  Returns the created error object.

  The \a text will be stored in the \c{message} property of the error
  object.

  The error object will be initialized to contain information about
  the location where the error occurred; specifically, it will have
  properties \c{lineNumber}, \c{fileName} and \c{stack}. These
  properties are described in \l {QtScript Extensions to ECMAScript}.

  \sa throwValue(), state()
*/
QScriptValue QScriptContext::throwError(Error error, const QString &text)
{
    Q_D(QScriptContext);
    return d->engine()->toPublic(d->throwError(error, text));
}

/*!
  \overload

  Throws an error with the given \a text.
  Returns the created error object.

  \sa throwValue(), state()
*/
QScriptValue QScriptContext::throwError(const QString &text)
{
    Q_D(QScriptContext);
    return d->engine()->toPublic(d->throwError(text));
}

/*!
  \internal
*/
QScriptContext::QScriptContext():
    d_ptr(new QScriptContextPrivate())
{
    d_ptr->q_ptr = this;
}

/*!
  Destroys this QScriptContext.
*/
QScriptContext::~QScriptContext()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
  Returns the QScriptEngine that this QScriptContext belongs to.
*/
QScriptEngine *QScriptContext::engine() const
{
    Q_D(const QScriptContext);
    return QScriptEnginePrivate::get(d->engine());
}

/*!
  Returns the function argument at the given \a index.

  If \a index >= argumentCount(), a QScriptValue of
  the primitive type Undefined is returned.

  \sa argumentCount()
*/
QScriptValue QScriptContext::argument(int index) const
{
    Q_D(const QScriptContext);
    if (index < 0)
        return QScriptValue();
    return d->engine()->toPublic(d->argument(index));
}

/*!
  Returns the callee. The callee is the function object that this
  QScriptContext represents an invocation of.
*/
QScriptValue QScriptContext::callee() const
{
    Q_D(const QScriptContext);
    return d->engine()->toPublic(d->m_callee);
}

/*!
  Returns the arguments object of this QScriptContext.

  The arguments object has properties \c callee (equal to callee())
  and \c length (equal to argumentCount()), and properties \c 0, \c 1,
  ..., argumentCount() - 1 that provide access to the argument
  values. Initially, property \c P (0 <= \c P < argumentCount()) has
  the same value as argument(\c P). In the case when \c P is less
  than the number of formal parameters of the function, \c P shares
  its value with the corresponding property of the activation object
  (activationObject()). This means that changing this property changes
  the corresponding property of the activation object and vice versa.

  \sa argument(), activationObject()
*/
QScriptValue QScriptContext::argumentsObject() const
{
    Q_D(const QScriptContext);
    return d->engine()->toPublic(d->argumentsObject());
}

/*!
  Returns true if the function was called as a constructor
  (e.g. \c{"new foo()"}); otherwise returns false.

  When a function is called as constructor, the thisObject()
  contains the newly constructed object to be initialized.
*/
bool QScriptContext::isCalledAsConstructor() const
{
    Q_D(const QScriptContext);
    return d->m_calledAsConstructor;
}

/*!
  Returns the parent context of this QScriptContext.
*/
QScriptContext *QScriptContext::parentContext() const
{
    Q_D(const QScriptContext);
    return QScriptContextPrivate::get(d->previous);
}

/*!
  Returns the number of arguments passed to the function
  in this invocation.

  Note that the argument count can be different from the
  formal number of arguments (the \c{length} property of
  callee()).

  \sa argument()
*/
int QScriptContext::argumentCount() const
{
    Q_D(const QScriptContext);
    return d->argc;
}

/*!
  \internal
*/
QScriptValue QScriptContext::returnValue() const
{
    Q_D(const QScriptContext);
    return d->engine()->toPublic(d->m_result);
}

/*!
  \internal
*/
void QScriptContext::setReturnValue(const QScriptValue &result)
{
    Q_D(QScriptContext);
    d->m_result = d->engine()->toImpl(result);
}

/*!
  Returns the activation object of this QScriptContext. The activation
  object provides access to the local variables associated with this
  context.

  \sa argument(), argumentsObject()
*/
QScriptValue QScriptContext::activationObject() const
{
    Q_D(const QScriptContext);
    return d->engine()->toPublic(d->activationObject());
}

/*!
  Sets the activation object of this QScriptContext to be the given \a
  activation.

  If \a activation is not an object, this function does nothing.
*/
void QScriptContext::setActivationObject(const QScriptValue &activation)
{
    Q_D(QScriptContext);
    if (!activation.isObject()) {
        return;
    } else if (activation.engine() != engine()) {
        qWarning("QScriptContext::setActivationObject() failed: "
                 "cannot set an object created in "
                 "a different engine");
    } else {
        d->m_activation = d->engine()->toImpl(activation);
    }
}

/*!
  Returns the `this' object associated with this QScriptContext.
*/
QScriptValue QScriptContext::thisObject() const
{
    Q_D(const QScriptContext);
    return d->engine()->toPublic(d->m_thisObject);
}

/*!
  Sets the `this' object associated with this QScriptContext to be
  \a thisObject.

  If \a thisObject is not an object, this function does nothing.
*/
void QScriptContext::setThisObject(const QScriptValue &thisObject)
{
    Q_D(QScriptContext);
    if (!thisObject.isObject()) {
    } else if (thisObject.engine() != engine()) {
        qWarning("QScriptContext::setThisObject() failed: "
                 "cannot set an object created in "
                 "a different engine");
    } else {
        d->m_thisObject = d->engine()->toImpl(thisObject);
    }
}

/*!
  Returns the execution state of this QScriptContext.
*/
QScriptContext::ExecutionState QScriptContext::state() const
{
    Q_D(const QScriptContext);
    return d->m_state;
}

/*!
  Returns a human-readable backtrace of this QScriptContext.

  Each line is of the form \c{<function-name>(<arguments>)@<file-name>:<line-number>}.

  To access individual pieces of debugging-related information (for
  example, to construct your own backtrace representation), use
  QScriptContextInfo.

  \sa QScriptEngine::uncaughtExceptionBacktrace(), QScriptContextInfo, toString()
*/
QStringList QScriptContext::backtrace() const
{
    Q_D(const QScriptContext);
    return d->backtrace();
}

static QString safeValueToString(const QScriptValue &value)
{
    if (value.isObject())
        return QLatin1String("[object Object]");
    else
        return value.toString();
}

/*!
  \since 4.4

  Returns a string representation of this context.
  This is useful for debugging.

  \sa backtrace()
*/
QString QScriptContext::toString() const
{
    QScriptContextInfo info(this);
    QString result;

    QString functionName = info.functionName();
    if (functionName.isEmpty()) {
        if (parentContext()) {
            if (info.functionType() == QScriptContextInfo::ScriptFunction)
                result.append(QLatin1String("<anonymous>"));
            else
                result.append(QLatin1String("<native>"));
        } else {
            result.append(QLatin1String("<global>"));
        }
    } else {
        result.append(functionName);
    }

    QStringList parameterNames = info.functionParameterNames();
    result.append(QLatin1String(" ("));
    for (int i = 0; i < argumentCount(); ++i) {
        if (i > 0)
            result.append(QLatin1String(", "));
        if (i < parameterNames.count()) {
            result.append(parameterNames.at(i));
            result.append(QLatin1Char('='));
        }
        QScriptValue arg = argument(i);
        result.append(safeValueToString(arg));
    }
    result.append(QLatin1Char(')'));

    QString fileName = info.fileName();
    int lineNumber = info.lineNumber();
    result.append(QLatin1String(" at "));
    if (!fileName.isEmpty()) {
        result.append(fileName);
        result.append(QLatin1Char(':'));
    }
    result.append(QString::number(lineNumber));
    return result;
}

/*!
  \internal
  \since 4.5

  Returns the scope chain of this QScriptContext.
*/
QScriptValueList QScriptContext::scopeChain() const
{
    Q_D(const QScriptContext);
    // make sure arguments properties are initialized
    const QScriptContextPrivate *ctx = d;
    while (ctx) {
        (void)ctx->activationObject();
        ctx = ctx->previous;
    }
    QScriptValueList result;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    QScriptValueImpl scope = d->m_scopeChain;
    while (scope.isObject()) {
        if (scope.classInfo() == eng_p->m_class_with)
            result.append(eng_p->toPublic(scope.prototype()));
        else
            result.append(eng_p->toPublic(scope));
        scope = scope.scope();
    }
    return result;
}

/*!
  \internal
  \since 4.5

  Adds the given \a object to the front of this context's scope chain.

  If \a object is not an object, this function does nothing.
*/
void QScriptContext::pushScope(const QScriptValue &object)
{
    Q_D(QScriptContext);
    if (!object.isObject()) {
        return;
    } else if (object.engine() != engine()) {
        qWarning("QScriptContext::pushScope() failed: "
                 "cannot push an object created in "
                 "a different engine");
        return;
    }
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    if (!d->m_scopeChain.isValid()) {
        d->m_scopeChain = eng_p->toImpl(object);
    } else {
        QScriptValueImpl withObject;
        eng_p->newObject(&withObject, eng_p->toImpl(object), eng_p->m_class_with);
        withObject.m_object_value->m_scope = d->m_scopeChain;
        withObject.setInternalValue(1); // to differentiate from with-statement objects
        d->m_scopeChain = withObject;
    }
}

/*!
  \internal
  \since 4.5

  Removes the front object from this context's scope chain, and
  returns the removed object.

  If the scope chain is already empty, this function returns an
  invalid QScriptValue.
*/
QScriptValue QScriptContext::popScope()
{
    Q_D(QScriptContext);
    if (!d->m_scopeChain.isObject())
        return QScriptValue();
    QScriptValueImpl result;
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());
    if (d->m_scopeChain.classInfo() != eng_p->m_class_with)
        result = d->m_scopeChain;
    else
        result = d->m_scopeChain.prototype();
    d->m_scopeChain = d->m_scopeChain.m_object_value->m_scope;
    return eng_p->toPublic(result);
}

QT_END_NAMESPACE

