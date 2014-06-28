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


#ifndef QSCRIPTMEMBERFWD_P_H
#define QSCRIPTMEMBERFWD_P_H

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

#if defined(Q_OS_VXWORKS) && defined(m_flags)
#  undef m_flags
#endif

#include <qglobal.h>

QT_BEGIN_NAMESPACE


class QScriptNameIdImpl;

namespace QScript {

    class Member
    {
    public:
        enum PropertyFlag {
            ObjectProperty      = 0x00000100, // Stored in the member table
            NativeProperty      = 0x00000200,

            UninitializedConst  = 0x00000800, // NB: shared with QScriptValue::KeepExistingFlags

            InternalRange       = 0x0000ff00  // Not user-accessible (read as 0, don't change on write)
        };

        inline Member() : m_nameId(0), m_id(0), m_flags(0) {}

        inline void resetFlags(uint flags);
        inline void setFlags(uint flags);
        inline void unsetFlags(uint flags);
        inline uint flags() const;
        inline bool testFlags(uint mask) const;

        inline bool isValid() const;

        inline bool isWritable() const;
        inline bool isDeletable() const;

        inline bool dontEnum() const;

        inline bool isObjectProperty() const;
        inline bool isNativeProperty() const;

        inline bool isUninitializedConst() const;

        inline bool isGetter() const;
        inline bool isSetter() const;
        inline bool isGetterOrSetter() const;

        inline int id() const;
        inline QScriptNameIdImpl *nameId() const;

        inline bool operator==(const Member &other) const;
        inline bool operator!=(const Member &other) const;

        inline static Member invalid();
        inline void invalidate();

        inline void native(QScriptNameIdImpl *nameId, int id, uint flags);
        inline void object(QScriptNameIdImpl *nameId, int id, uint flags);

    private:
        QScriptNameIdImpl *m_nameId;
        int m_id;
        uint m_flags;
    };

} // namespace QScript

QT_END_NAMESPACE


#endif
