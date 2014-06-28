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


#include <QTextStream>


#include "qscriptasm_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptengine_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

QT_BEGIN_NAMESPACE

const char *QScriptInstruction::opcode[] = {
#define STR(a) #a
#define Q_SCRIPT_DEFINE_OPERATOR(op) STR(i##op) ,
#include "instruction.table"
#undef Q_SCRIPT_DEFINE_OPERATOR
#undef STR
};

void QScriptInstruction::print(QTextStream &out) const
{
    out << opcode[op];

    if (! operand[0].isValid())
        return;

    out << '(' << operand[0].toString();

    if (operand[1].isValid())
        out << ", " << operand[1].toString();

    out << ')';
}

namespace QScript {

Code::Code():
    optimized(false),
    firstInstruction(0),
    lastInstruction(0),
    astPool(0)
{
}

Code::~Code()
{
    delete[] firstInstruction;
}

void Code::init(const CompilationUnit &compilation, NodePool *pool)
{
    optimized = false;
    const QVector<QScriptInstruction> ilist = compilation.instructions();
    firstInstruction = new QScriptInstruction[ilist.count()];
    lastInstruction = firstInstruction + ilist.count();
    qCopy(ilist.begin(), ilist.end(), firstInstruction);
    exceptionHandlers = compilation.exceptionHandlers();
    astPool = pool;
}

} // namespace QScript

QT_END_NAMESPACE

