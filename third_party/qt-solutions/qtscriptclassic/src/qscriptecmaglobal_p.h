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


#ifndef QSCRIPTECMAGLOBAL_P_H
#define QSCRIPTECMAGLOBAL_P_H

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
#include "qscriptfunction_p.h"
#include "qscriptvalue.h"

QT_BEGIN_NAMESPACE

class QScriptEnginePrivate;
class QScriptContextPrivate;
class QScriptClassInfo;
class QScriptValueImpl;


namespace QScript { namespace Ecma {

class Global: public QScriptObjectData
{
protected:
    Global(QScriptEnginePrivate *engine, QScriptClassInfo *classInfo);

public:
    virtual ~Global();

    inline QScriptEnginePrivate *engine() const;

    static void construct(QScriptValueImpl *object, QScriptEnginePrivate *eng);
    static void initialize(QScriptValueImpl *object, QScriptEnginePrivate *eng);

protected:
    static QScriptValueImpl method_parseInt(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValueImpl method_parseFloat(QScriptContextPrivate *context,
                                              QScriptEnginePrivate *eng,
                                              QScriptClassInfo *classInfo);
    static QScriptValueImpl method_isNaN(QScriptContextPrivate *context,
                                         QScriptEnginePrivate *eng,
                                         QScriptClassInfo *classInfo);
    static QScriptValueImpl method_isFinite(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValueImpl method_decodeURI(QScriptContextPrivate *context,
                                             QScriptEnginePrivate *eng,
                                             QScriptClassInfo *classInfo);
    static QScriptValueImpl method_decodeURIComponent(QScriptContextPrivate *context,
                                                      QScriptEnginePrivate *eng,
                                                      QScriptClassInfo *classInfo);
    static QScriptValueImpl method_encodeURI(QScriptContextPrivate *context,
                                             QScriptEnginePrivate *eng,
                                             QScriptClassInfo *classInfo);
    static QScriptValueImpl method_encodeURIComponent(QScriptContextPrivate *context,
                                                      QScriptEnginePrivate *eng,
                                                      QScriptClassInfo *classInfo);
    static QScriptValueImpl method_escape(QScriptContextPrivate *context,
                                          QScriptEnginePrivate *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValueImpl method_unescape(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValueImpl method_version(QScriptContextPrivate *context,
                                           QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_gc(QScriptContextPrivate *context,
                                      QScriptEnginePrivate *eng,
                                      QScriptClassInfo *classInfo);

private:
    static void addFunction(QScriptValueImpl &object, const QString &name,
                            QScriptInternalFunctionSignature fun, int length,
                            const QScriptValue::PropertyFlags flags);

    QScriptEnginePrivate *m_engine;
    QScriptClassInfo *m_classInfo;
};

inline QScriptEnginePrivate *Global::engine() const
{ return m_engine; }


} } // namespace QScript::Ecma


QT_END_NAMESPACE

#endif
