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


#ifndef QSCRIPTECMAERROR_P_H
#define QSCRIPTECMAERROR_P_H

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

#include "qscriptecmacore_p.h"


QT_BEGIN_NAMESPACE

namespace QScript { namespace Ecma {

class Error: public Core
{
public:
    Error(QScriptEnginePrivate *engine);
    virtual ~Error();

    virtual void execute(QScriptContextPrivate *context);

    virtual void mark(QScriptEnginePrivate *eng, int generation);

    void newError(QScriptValueImpl *result, const QString &message = QString());
    void newEvalError(QScriptValueImpl *result, const QString &message = QString());
    void newRangeError(QScriptValueImpl *result, const QString &message = QString());
    void newReferenceError(QScriptValueImpl *result, const QString &message = QString());
    void newSyntaxError(QScriptValueImpl *result, const QString &message = QString());
    void newTypeError(QScriptValueImpl *result, const QString &message = QString());
    void newURIError(QScriptValueImpl *result, const QString &message = QString());

    bool isEvalError(const QScriptValueImpl &value) const;
    bool isRangeError(const QScriptValueImpl &value) const;
    bool isReferenceError(const QScriptValueImpl &value) const;
    bool isSyntaxError(const QScriptValueImpl &value) const;
    bool isTypeError(const QScriptValueImpl &value) const;
    bool isURIError(const QScriptValueImpl &value) const;

    static QStringList backtrace(const QScriptValueImpl &error);

    QScriptValueImpl evalErrorCtor;
    QScriptValueImpl rangeErrorCtor;
    QScriptValueImpl referenceErrorCtor;
    QScriptValueImpl syntaxErrorCtor;
    QScriptValueImpl typeErrorCtor;
    QScriptValueImpl uriErrorCtor;

    QScriptValueImpl evalErrorPrototype;
    QScriptValueImpl rangeErrorPrototype;
    QScriptValueImpl referenceErrorPrototype;
    QScriptValueImpl syntaxErrorPrototype;
    QScriptValueImpl typeErrorPrototype;
    QScriptValueImpl uriErrorPrototype;

protected:
    static QScriptValueImpl method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo);
    static QScriptValueImpl method_backtrace(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo);

private:
    void newError(QScriptValueImpl *result, const QScriptValueImpl &proto,
                  const QString &message = QString());
    void newErrorPrototype(QScriptValueImpl *result, const QScriptValueImpl &proto,
                           QScriptValueImpl &ztor, const QString &name);
};

} } // namespace QScript::Ecma

QT_END_NAMESPACE

#endif // QSCRIPTECMAERROR_P_H
