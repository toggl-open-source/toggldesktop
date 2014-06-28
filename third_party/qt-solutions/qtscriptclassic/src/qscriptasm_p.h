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


#ifndef QSCRIPTASM_P_H
#define QSCRIPTASM_P_H

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

#include <qglobal.h>


#include <qvector.h>

#include "qscriptvalueimplfwd_p.h"

QT_BEGIN_NAMESPACE

class QTextStream;

class QScriptInstruction
{
public:
    enum Operator {
#define Q_SCRIPT_DEFINE_OPERATOR(op) OP_##op,
#include "instruction.table"
#undef Q_SCRIPT_DEFINE_OPERATOR
        OP_Dummy
    };

public:
    Operator op;
    QScriptValueImpl operand[2];
#if defined(Q_SCRIPT_DIRECT_CODE)
    void *code;
#endif

    void print(QTextStream &out) const;

    static const char *opcode[];
};

namespace QScript {

class NodePool;

class ExceptionHandlerDescriptor
{
public:
    ExceptionHandlerDescriptor()
        : m_startInstruction(0),
          m_endInstruction(0),
          m_handlerInstruction(0) {}

    ExceptionHandlerDescriptor(
        int startInstruction,
        int endInstruction,
        int handlerInstruction)
        : m_startInstruction(startInstruction),
          m_endInstruction(endInstruction),
          m_handlerInstruction(handlerInstruction) {}

    inline int startInstruction() const { return m_startInstruction; }
    inline int endInstruction() const { return m_endInstruction; }
    inline int handlerInstruction() const { return m_handlerInstruction; }

private:
    int m_startInstruction;
    int m_endInstruction;
    int m_handlerInstruction;
};

class CompilationUnit
{
public:
    CompilationUnit(): m_valid(true),
        m_errorLineNumber(-1) {}

    bool isValid() const { return m_valid; }

    void setError(const QString &message, int lineNumber)
    {
        m_errorMessage = message;
        m_errorLineNumber = lineNumber;
        m_valid = false;
    }

    QString errorMessage() const
        { return m_errorMessage; }
    int errorLineNumber() const
        { return m_errorLineNumber; }

    QVector<QScriptInstruction> instructions() const
        { return m_instructions; }
    void setInstructions(const QVector<QScriptInstruction> &instructions)
        { m_instructions = instructions; }

    QVector<ExceptionHandlerDescriptor> exceptionHandlers() const
        { return m_exceptionHandlers; }
    void setExceptionHandlers(const QVector<ExceptionHandlerDescriptor> &exceptionHandlers)
        { m_exceptionHandlers = exceptionHandlers; }

private:
    bool m_valid;
    QString m_errorMessage;
    int m_errorLineNumber;
    QVector<QScriptInstruction> m_instructions;
    QVector<ExceptionHandlerDescriptor> m_exceptionHandlers;
};

class Code
{
public:
    Code();
    ~Code();

    void init(const CompilationUnit &compilation, NodePool *astPool);

public: // attributes
    bool optimized;
    QScriptInstruction *firstInstruction;
    QScriptInstruction *lastInstruction;
    QVector<ExceptionHandlerDescriptor> exceptionHandlers;
    NodePool *astPool;

private:
    Q_DISABLE_COPY(Code)
};


} // namespace QScript

QT_END_NAMESPACE

#endif // QSCRIPTASM_P_H
