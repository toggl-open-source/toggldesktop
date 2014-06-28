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


#ifndef QSCRIPTGLOBALS_P_H
#define QSCRIPTGLOBALS_P_H

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


QT_BEGIN_NAMESPACE

class QScriptValue;
class QScriptValueImpl;
class QScriptClassInfo;
class QScriptEngine;
class QScriptEnginePrivate;
class QScriptContext;
class QScriptContextPrivate;

typedef QScriptValueImpl (*QScriptInternalFunctionSignature)(QScriptContextPrivate *, QScriptEnginePrivate *, QScriptClassInfo *);
typedef QScriptValue (*QScriptFunctionSignature)(QScriptContext *, QScriptEngine *);
typedef QScriptValue (*QScriptFunctionWithArgSignature)(QScriptContext *, QScriptEngine *, void *);

namespace QScript {

enum Type {
    InvalidType,
    // standard types
    UndefinedType,
    NullType,
    BooleanType,
    StringType,
    NumberType,
    ObjectType,
    // internal types
    IntegerType,
    ReferenceType,
    PointerType,
    LazyStringType
};

enum AccessMode {
    Read = 0x01,
    Write = 0x02,
    ReadWrite = 0x03
};

} // namespace QScript

QT_END_NAMESPACE


#endif // QSCRIPTGLOBALS_P_H
