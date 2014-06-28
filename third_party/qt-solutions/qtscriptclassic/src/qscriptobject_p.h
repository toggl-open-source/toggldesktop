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


#ifndef QSCRIPTOBJECT_P_H
#define QSCRIPTOBJECT_P_H

#include "qscriptobjectfwd_p.h"


QT_BEGIN_NAMESPACE

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

inline bool QScriptObject::findMember(QScriptNameIdImpl *nameId,
                       QScript::Member *m) const
{
    const QScript::Member *members = m_members.constData();
    const int size = m_members.size();

    const QScript::Member *first = &members[-1];
    const QScript::Member *last = &members[size - 1];

    for (const QScript::Member *it = last; it != first; --it) {
        if (it->nameId() == nameId && it->isValid()) {
            *m = *it;
            return true;
        }
    }

    return false;
}

// assumes that m already points to the setter
inline bool QScriptObject::findGetter(QScript::Member *m) const
{
    const QScript::Member *members = m_members.constData();
    const QScript::Member *first = &members[-1];
    const QScript::Member *last = &members[m->id() - 1];

    for (const QScript::Member *it = last; it != first; --it) {
        if (it->nameId() == m->nameId() && it->isValid() && it->isGetter()) {
            *m = *it;
            return true;
        }
    }

    return false;
}

// assumes that m already points to the getter
inline bool QScriptObject::findSetter(QScript::Member *m) const
{
    const QScript::Member *members = m_members.constData();
    const QScript::Member *first = &members[-1];
    const QScript::Member *last = &members[m->id() - 1];

    for (const QScript::Member *it = last; it != first; --it) {
        if (it->nameId() == m->nameId() && it->isValid() && it->isSetter()) {
            *m = *it;
            return true;
        }
    }

    return false;
}

inline int QScriptObject::memberCount() const
{
    return m_members.size();
}

inline void QScriptObject::createMember(QScriptNameIdImpl *nameId,
                         QScript::Member *member, uint flags)
{
    member->object(nameId, m_values.size(), flags);
    m_members.append(*member);
    m_values.append(QScriptValueImpl());
}

inline void QScriptObject::member(int index, QScript::Member *member)
{
    *member = m_members[index];
}

inline void QScriptObject::put(const QScript::Member &m, const QScriptValueImpl &v)
{
    m_values[m.id()] = v;
}

inline QScriptValueImpl &QScriptObject::reference(const QScript::Member &m)
{
    return m_values[m.id()];
}

inline void QScriptObject::get(const QScript::Member &m, QScriptValueImpl *v)
{
    Q_ASSERT(m.isObjectProperty());
    *v = m_values[m.id()];
}

inline void QScriptObject::removeMember(const QScript::Member &member)
{
    m_members[member.id()].invalidate();
    m_values[member.id()].invalidate();
}

inline QScriptObject::~QScriptObject()
{
    finalize();
}

inline void QScriptObject::finalize()
{
    finalizeData();
}

inline void QScriptObject::finalizeData()
{
    if (m_data) {
        m_data->finalize(m_class->engine());
        delete m_data;
        m_data = 0;
    }
}

inline void QScriptObject::reset()
{
    m_prototype.invalidate();
    m_scope.invalidate();
    m_internalValue.invalidate();
    m_members.resize(0);
    m_values.resize(0);
    m_data = 0;
}

QT_END_NAMESPACE


#endif
