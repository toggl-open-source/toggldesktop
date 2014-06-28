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


#ifndef QSCRIPTFUNCTION_P_H
#define QSCRIPTFUNCTION_P_H

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

#include "qscriptobjectdata_p.h"


#include "qscriptglobals_p.h"
#include "qscriptnodepool_p.h"

#include <QList>

#ifndef QT_NO_QOBJECT
# include <QPointer>
# include <QMetaMethod>
#endif

QT_BEGIN_NAMESPACE

class QScriptContext;
class QScriptContextPrivate;
class QScriptNameIdImpl;

class QScriptFunction: public QScriptObjectData
{
public:
    enum Type {
        Unknown,
        Script,
        C,
        C2,
        C3,
        Qt,
        QtProperty
    };

    QScriptFunction(int len = 0)
        : length(len)
        { }
    virtual ~QScriptFunction();

    virtual void execute(QScriptContextPrivate *context) = 0;
    virtual QString toString(QScriptContextPrivate *context) const;

    virtual Type type() const { return Unknown; }

    // name of the file the function is defined in
    virtual QString fileName() const;

    virtual QString functionName() const;

    virtual int startLineNumber() const;

    virtual int endLineNumber() const;

    virtual void mark(QScriptEnginePrivate *engine, int generation);

public: // ### private
    int length;
    QList<QScriptNameIdImpl*> formals;
};

namespace QScript {

// public API function
class CFunction: public QScriptFunction
{
public:
    CFunction(QScriptFunctionSignature funPtr, int length)
        : QScriptFunction(length), m_funPtr(funPtr)
        { }

    virtual ~CFunction() { }

    virtual void execute(QScriptContextPrivate *context);

    virtual Type type() const { return QScriptFunction::C; }

    virtual QString functionName() const;

private:
    QScriptFunctionSignature m_funPtr;
};

// internal API function
class C2Function: public QScriptFunction
{
public:
    C2Function(QScriptInternalFunctionSignature funPtr, int length,
               QScriptClassInfo *classInfo, const QString &name)
        : QScriptFunction(length), m_funPtr(funPtr),
          m_classInfo(classInfo), m_name(name)
        { }

    virtual ~C2Function() {}

    virtual void execute(QScriptContextPrivate *context);

    virtual Type type() const { return QScriptFunction::C2; }

    virtual QString functionName() const;

private:
    QScriptInternalFunctionSignature m_funPtr;
    QScriptClassInfo *m_classInfo;
    QString m_name;
};

class C3Function: public QScriptFunction
{
public:
    C3Function(QScriptFunctionWithArgSignature funPtr, void *arg, int length)
        : QScriptFunction(length), m_funPtr(funPtr), m_arg(arg)
        { }

    virtual ~C3Function() { }

    virtual void execute(QScriptContextPrivate *context);

    virtual Type type() const { return QScriptFunction::C3; }

private:
    QScriptFunctionWithArgSignature m_funPtr;
    void *m_arg;
};

namespace AST {
    class FunctionExpression;
}

// implemented in qscriptcontext_p.cpp
class ScriptFunction: public QScriptFunction
{
public:
    ScriptFunction(AST::FunctionExpression *definition, NodePool *astPool):
        m_definition(definition), m_astPool(astPool), m_compiledCode(0) {}

    virtual ~ScriptFunction() {}

    virtual void execute(QScriptContextPrivate *context);

    virtual QString toString(QScriptContextPrivate *context) const;

    virtual Type type() const
    { return QScriptFunction::Script; }

    virtual QString fileName() const;

    virtual QString functionName() const;

    virtual int startLineNumber() const;

    virtual int endLineNumber() const;

private:
    AST::FunctionExpression *m_definition;
    QExplicitlySharedDataPointer<NodePool> m_astPool;
    Code *m_compiledCode;
};

} // namespace QScript

QT_END_NAMESPACE


#endif // QSCRIPTFUNCTION_P_H
