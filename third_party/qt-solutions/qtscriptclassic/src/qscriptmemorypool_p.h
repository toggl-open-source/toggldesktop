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


#ifndef QSCRIPTMEMORYPOOL_P_H
#define QSCRIPTMEMORYPOOL_P_H

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
#include <qshareddata.h>
#include <string.h>

QT_BEGIN_NAMESPACE

namespace QScript {

class MemoryPool : public QSharedData
{
public:
    enum { maxBlockCount = -1 };
    enum { defaultBlockSize = 1 << 12 };

    MemoryPool() {
        m_blockIndex = maxBlockCount;
        m_currentIndex = 0;
        m_storage = 0;
        m_currentBlock = 0;
        m_currentBlockSize = 0;
    }

    virtual ~MemoryPool() {
        for (int index = 0; index < m_blockIndex + 1; ++index)
            qFree(m_storage[index]);

        qFree(m_storage);
    }

    char *allocate(int bytes) {
        bytes += (8 - bytes) & 7; // ensure multiple of 8 bytes (maintain alignment)
        if (m_currentBlock == 0 || m_currentBlockSize < m_currentIndex + bytes) {
            ++m_blockIndex;
            m_currentBlockSize = defaultBlockSize << m_blockIndex;

            m_storage = reinterpret_cast<char**>(qRealloc(m_storage, sizeof(char*) * (1 + m_blockIndex)));
            m_currentBlock = m_storage[m_blockIndex] = reinterpret_cast<char*>(qMalloc(m_currentBlockSize));
            ::memset(m_currentBlock, 0, m_currentBlockSize);

            m_currentIndex = (8 - quintptr(m_currentBlock)) & 7; // ensure first chunk is 64-bit aligned
            Q_ASSERT(m_currentIndex + bytes <= m_currentBlockSize);
        }

        char *p = reinterpret_cast<char *>
            (m_currentBlock + m_currentIndex);

        m_currentIndex += bytes;

        return p;
    }

    int bytesAllocated() const {
        int bytes = 0;
        for (int index = 0; index < m_blockIndex; ++index)
            bytes += (defaultBlockSize << index);
        bytes += m_currentIndex;
        return bytes;
    }

private:
    int m_blockIndex;
    int m_currentIndex;
    char *m_currentBlock;
    int m_currentBlockSize;
    char **m_storage;

private:
    Q_DISABLE_COPY(MemoryPool)
};

} // namespace QScript

QT_END_NAMESPACE

#endif
