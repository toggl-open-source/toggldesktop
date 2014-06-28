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


#ifndef QSCRIPTECMAREGEXP_P_H
#define QSCRIPTECMAREGEXP_P_H

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

#include <QRegExp>


#include "qscriptecmacore_p.h"

QT_BEGIN_NAMESPACE

namespace QScript { namespace Ecma {

class RegExp: public Core
{
public:
    enum RegExpFlag {
        Global     = 0x01,
        IgnoreCase = 0x02,
        Multiline  = 0x04
    };

    RegExp(QScriptEnginePrivate *engine);
    virtual ~RegExp();

    virtual void execute(QScriptContextPrivate *context);

    class Instance: public QScriptObjectData {
    public:
        Instance() : flags(0) {}
        virtual ~Instance() {}

        static Instance *get(const QScriptValueImpl &object,
                             QScriptClassInfo *klass);

    public: // attributes
#ifndef QT_NO_REGEXP
        QRegExp value;
#else
        QString pattern;
#endif
        int flags;
    };

    inline Instance *get(const QScriptValueImpl &object) const
        { return Instance::get(object, classInfo()); }

    void newRegExp(QScriptValueImpl *result, const QString &pattern,
                   int flags);
#ifndef QT_NO_REGEXP
    void newRegExp(QScriptValueImpl *result, const QRegExp &rx,
                   int flags = 0);
    QRegExp toRegExp(const QScriptValueImpl &value) const;
    static QRegExp toRegExp(const QString &pattern, int flags);
#endif

    static int flagFromChar(const QChar &ch);
    static QString flagsToString(int flags);

protected:
    static QScriptValueImpl method_exec(QScriptContextPrivate *context,
                                        QScriptEnginePrivate *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValueImpl method_test(QScriptContextPrivate *context,
                                        QScriptEnginePrivate *eng,
                                        QScriptClassInfo *classInfo);
    static QScriptValueImpl method_toString(QScriptContextPrivate *context,
                                            QScriptEnginePrivate *eng,
                                            QScriptClassInfo *classInfo);

private:
#ifndef QT_NO_REGEXP
    void newRegExp_helper(QScriptValueImpl *result, const QRegExp &rx,
                          int flags);
#endif
    void initRegExp(QScriptValueImpl *result,
#ifndef QT_NO_REGEXP
                    const QRegExp &rx,
#else
                    const QString &pattern,
#endif
                    int flags);
};

} } // namespace QScript::Ecma

QT_END_NAMESPACE

#endif // QSCRIPTECMAREGEXP_P_H
