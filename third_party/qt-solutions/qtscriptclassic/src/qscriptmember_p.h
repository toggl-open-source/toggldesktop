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


#ifndef QSCRIPTMEMBER_P_H
#define QSCRIPTMEMBER_P_H

#include "qscriptmemberfwd_p.h"


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

inline void QScript::Member::resetFlags(uint flags)
{
    m_flags = flags;
}

inline void QScript::Member::setFlags(uint flags)
{
    m_flags |= flags;
}

inline void QScript::Member::unsetFlags(uint flags)
{
    m_flags &= ~flags;
}

inline uint QScript::Member::flags() const
{
    return m_flags;
}

inline bool QScript::Member::testFlags(uint mask) const
{
    return m_flags & mask;
}

inline bool QScript::Member::isValid() const
{
    return m_flags & 0x00000300;
}

inline bool QScript::Member::isWritable() const
{
    return !(m_flags & QScriptValue::ReadOnly);
}

inline bool QScript::Member::isDeletable() const
{
    return !(m_flags & QScriptValue::Undeletable);
}

inline bool QScript::Member::dontEnum() const
{
    return m_flags & QScriptValue::SkipInEnumeration;
}

inline bool QScript::Member::isObjectProperty() const
{
    return m_flags & ObjectProperty;
}

inline bool QScript::Member::isNativeProperty() const
{
    return m_flags & NativeProperty;
}

inline bool QScript::Member::isUninitializedConst() const
{
    return m_flags & UninitializedConst;
}

inline bool QScript::Member::isGetter() const
{
    return m_flags & QScriptValue::PropertyGetter;
}

inline bool QScript::Member::isSetter() const
{
    return m_flags & QScriptValue::PropertySetter;
}

inline bool QScript::Member::isGetterOrSetter() const
{
    return m_flags & (QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
}

inline int QScript::Member::id() const
{
    return m_id;
}

inline QScriptNameIdImpl *QScript::Member::nameId() const
{
    return m_nameId;
}

inline QScript::Member QScript::Member::invalid()
{
    Member m;
    m.m_flags = 0;
    return m;
}

inline void QScript::Member::invalidate()
{
    m_flags = 0;
}

inline void QScript::Member::native(QScriptNameIdImpl *nameId, int id, uint flags)
{
    Q_ASSERT(! (flags & ObjectProperty));

    m_nameId = nameId;
    m_id = id;
    m_flags = flags | NativeProperty;
}

inline void QScript::Member::object(QScriptNameIdImpl *nameId, int id, uint flags)
{
    Q_ASSERT(! (flags & NativeProperty));

    m_nameId = nameId;
    m_id = id;
    m_flags = flags | ObjectProperty;
}

inline bool QScript::Member::operator==(const QScript::Member &other) const
{
    return m_nameId == other.m_nameId;
}

inline bool QScript::Member::operator!=(const QScript::Member &other) const
{
    return m_nameId != other.m_nameId;
}

QT_END_NAMESPACE


#endif
