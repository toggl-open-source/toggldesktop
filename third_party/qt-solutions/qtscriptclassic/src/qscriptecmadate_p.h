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


#ifndef QSCRIPTECMADATE_P_H
#define QSCRIPTECMADATE_P_H

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

class QDate;
class QDateTime;

namespace QScript { namespace Ecma {

class Date: public Core
{
public:
    Date(QScriptEnginePrivate *engine);
    virtual ~Date();

    virtual void execute(QScriptContextPrivate *context);

    void newDate(QScriptValueImpl *result, double t);
    void newDate(QScriptValueImpl *result, const QDateTime &dt);
    void newDate(QScriptValueImpl *result, const QDate &d);

    QDateTime toDateTime(const QScriptValueImpl &date) const;

protected:
    static QScriptValueImpl method_MakeTime(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValueImpl method_MakeDate(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValueImpl method_TimeClip(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValueImpl method_parse(QScriptContextPrivate *context,
                                         QScriptEnginePrivate *eng,
                                         QScriptClassInfo *classInfo);
    static QScriptValueImpl method_UTC(QScriptContextPrivate *context,
                                       QScriptEnginePrivate *eng,
                                       QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toString(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toDateString(QScriptContextPrivate *context,
                                                QScriptEnginePrivate *eng,
                                                QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toTimeString(QScriptContextPrivate *context,
                                                QScriptEnginePrivate *eng,
                                                QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toLocaleString(QScriptContextPrivate *context,
                                                  QScriptEnginePrivate *eng,
                                                  QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toLocaleDateString(QScriptContextPrivate *context,
                                                      QScriptEnginePrivate *eng,
                                                      QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toLocaleTimeString(QScriptContextPrivate *context,
                                                      QScriptEnginePrivate *eng,
                                                      QScriptClassInfo *classInfo);
    static QScriptValueImpl method_valueOf(QScriptContextPrivate *context,
                                           QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getTime(QScriptContextPrivate *context,
                                           QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getYear(QScriptContextPrivate *context,
                                           QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getFullYear(QScriptContextPrivate *context,
                                               QScriptEnginePrivate *eng,
                                               QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getUTCFullYear(QScriptContextPrivate *context,
                                                  QScriptEnginePrivate *eng,
                                                  QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getMonth(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getUTCMonth(QScriptContextPrivate *context,
                                               QScriptEnginePrivate *eng,
                                               QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getDate(QScriptContextPrivate *context,
                                           QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getUTCDate(QScriptContextPrivate *context,
                                              QScriptEnginePrivate *eng,
                                              QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getDay(QScriptContextPrivate *context,
                                          QScriptEnginePrivate *eng,
                                          QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getUTCDay(QScriptContextPrivate *context,
                                             QScriptEnginePrivate *eng,
                                             QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getHours(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getUTCHours(QScriptContextPrivate *context,
                                               QScriptEnginePrivate *eng,
                                               QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getMinutes(QScriptContextPrivate *context,
                                              QScriptEnginePrivate *eng,
                                              QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getUTCMinutes(QScriptContextPrivate *context,
                                                 QScriptEnginePrivate *eng,
                                                 QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getSeconds(QScriptContextPrivate *context,
                                              QScriptEnginePrivate *eng,
                                              QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getUTCSeconds(QScriptContextPrivate *context,
                                                 QScriptEnginePrivate *eng,
                                                 QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getMilliseconds(QScriptContextPrivate *context,
                                                   QScriptEnginePrivate *eng,
                                                   QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getUTCMilliseconds(QScriptContextPrivate *context,
                                                      QScriptEnginePrivate *eng,
                                                      QScriptClassInfo *classInfo);
    static QScriptValueImpl method_getTimezoneOffset(QScriptContextPrivate *context,
                                                     QScriptEnginePrivate *eng,
                                                     QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setTime(QScriptContextPrivate *context,
                                           QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setMilliseconds(QScriptContextPrivate *context,
                                                   QScriptEnginePrivate *eng,
                                                   QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setUTCMilliseconds(QScriptContextPrivate *context,
                                                      QScriptEnginePrivate *eng,
                                                      QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setSeconds(QScriptContextPrivate *context,
                                              QScriptEnginePrivate *eng,
                                              QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setUTCSeconds(QScriptContextPrivate *context,
                                                 QScriptEnginePrivate *eng,
                                                 QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setMinutes(QScriptContextPrivate *context,
                                              QScriptEnginePrivate *eng,
                                              QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setUTCMinutes(QScriptContextPrivate *context,
                                                 QScriptEnginePrivate *eng,
                                                 QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setHours(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setUTCHours(QScriptContextPrivate *context,
                                               QScriptEnginePrivate *eng,
                                               QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setDate(QScriptContextPrivate *context,
                                           QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setUTCDate(QScriptContextPrivate *context,
                                              QScriptEnginePrivate *eng,
                                              QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setMonth(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setUTCMonth(QScriptContextPrivate *context,
                                               QScriptEnginePrivate *eng,
                                               QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setYear(QScriptContextPrivate *context,
                                           QScriptEnginePrivate *eng,
                                           QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setFullYear(QScriptContextPrivate *context,
                                               QScriptEnginePrivate *eng,
                                               QScriptClassInfo *classInfo);
    static QScriptValueImpl method_setUTCFullYear(QScriptContextPrivate *context,
                                                  QScriptEnginePrivate *eng,
                                                  QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toUTCString(QScriptContextPrivate *context,
                                               QScriptEnginePrivate *eng,
                                               QScriptClassInfo *classInfo);
};

} } // namespace QScript::Ecma

QT_END_NAMESPACE


#endif
