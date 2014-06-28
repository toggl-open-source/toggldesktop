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


#ifndef QSCRIPTCLASSINFO_P_H
#define QSCRIPTCLASSINFO_P_H

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

#if defined(Q_OS_VXWORKS) && defined(m_type)
#  undef m_type
#endif

#include "qscriptclassdata_p.h"


#include <qstring.h>

QT_BEGIN_NAMESPACE

class QScriptClassInfo
{
public:
    enum Type {
        FunctionBased   = 0x40000000,

        ObjectType      = 1,
        FunctionType    = 2 | FunctionBased,
        ArrayType       = 3,
        StringType      = 4,
        BooleanType     = 5,
        NumberType      = 6,
        DateType        = 7,
        RegExpType      = 8,
        ErrorType       = 9,

        VariantType     = 10,
        QObjectType     = 11,
        QMetaObjectType = 12 | FunctionBased,

        // Types used by the runtime
        ActivationType  = 100,
        EnumerationType = 101,

        CustomType      = 1000,

        TypeMask        = 0x0000FFFF
    };

    inline QScriptClassInfo(QScriptEnginePrivate *engine, Type type, const QString &name)
        : m_engine(engine), m_type(type), m_name(name), m_data(0) { }
    inline ~QScriptClassInfo() { delete m_data; }

    inline QScriptEnginePrivate *engine() const
        { return m_engine; }
    inline Type type() const
        { return m_type; }
    inline QString name() const
        { return m_name; }

    inline void setData(QScriptClassData *data)
        { m_data = data; }
    QScriptClassData *data() const
        { return m_data; }

private:
    QScriptEnginePrivate *m_engine;
    Type m_type;
    QString m_name;
    QScriptClassData *m_data;

private:
    Q_DISABLE_COPY(QScriptClassInfo)
};

QT_END_NAMESPACE

#endif // QSCRIPTCLASSINFO_P_H
