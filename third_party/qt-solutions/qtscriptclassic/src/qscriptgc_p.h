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


#ifndef QSCRIPTGC_P_H
#define QSCRIPTGC_P_H

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

#if defined(Q_OS_VXWORKS) && defined(m_free)
#  undef m_free
#endif

#include <qglobal.h>


#include <QtDebug>
#include <new>

#include "qscriptmemorypool_p.h"

QT_BEGIN_NAMESPACE

namespace QScript {

class GCBlock
{
public:
    GCBlock *next;

    union {
        int generation;
        uint flags;
    };

public:
    inline GCBlock(GCBlock *n):
        next(n), flags(0) {}

    inline void *data()
    { return reinterpret_cast<char *>(this) + sizeof(GCBlock); }

    inline static GCBlock *get(void *ptr)
    {
        char *where = reinterpret_cast<char *>(ptr);
        return reinterpret_cast<GCBlock *>(where - sizeof(GCBlock));
    }
};

template <typename _Tp>
class GCAlloc
{
private:
    int m_new_allocated_blocks;
    int m_free_blocks;
    int m_new_allocated_extra_bytes;
    GCBlock *m_head;
    GCBlock *m_current;
    GCBlock *m_free;
    bool m_blocked_gc;
    bool m_force_gc;
    bool m_sweeping;
    MemoryPool pool;
    _Tp trivial;

public:
    enum { MaxNumberOfBlocks = 1 << 14 };
    enum { MaxNumberOfExtraBytes = 0x800000 };

public:
    inline GCAlloc():
        m_new_allocated_blocks(0),
        m_free_blocks(0),
        m_new_allocated_extra_bytes(0),
        m_head(0),
        m_current(0),
        m_free(0),
        m_blocked_gc(false),
        m_force_gc(false),
        m_sweeping(false) {
        trivial.reset();
    }

    inline ~GCAlloc() {
    }

    inline void destruct() {
        m_sweeping = true;
        GCBlock *blk = m_free;

        if (! blk) {
            blk = m_head;
            m_head = 0;
        }

        while (blk) {
            GCBlock *was = blk;
            blk = blk->next;

            Q_ASSERT(was->data());
            _Tp *data = reinterpret_cast<_Tp*>(was->data());
            data->~_Tp();
            blk->~GCBlock();

            if (! blk && m_head) {
                blk = m_head;
                m_head = 0;
            }
        }
        m_sweeping = false;
    }

    inline int newAllocatedBlocks() const { return m_new_allocated_blocks; }
    inline int freeBlocks() const { return m_free_blocks; }

    inline _Tp *operator()(int generation)
    {
        GCBlock *previous = m_current;
        void *where = 0;

        if (! m_free) {
            Q_ASSERT (m_free_blocks == 0);
            where = pool.allocate(sizeof(GCBlock) + sizeof(_Tp));
            ++m_new_allocated_blocks;
            (void) new (reinterpret_cast<char*>(where) + sizeof(GCBlock)) _Tp();
        } else {
            --m_free_blocks;
            where = m_free;
            m_free = m_free->next;

            if (! m_free)
                m_force_gc = true;
        }

        m_current = new (where) GCBlock(0);

        if (! previous) {
            Q_ASSERT(! m_head);
            m_head = m_current;
        } else {
            previous->next = m_current;
        }
        m_current->generation = generation;

        return reinterpret_cast<_Tp*> (m_current->data());
    }

    inline bool blocked() const
    {
        return m_blocked_gc;
    }

    inline bool sweeping() const
    {
        return m_sweeping;
    }

    inline bool blockGC(bool block)
    {
        bool was = m_blocked_gc;
        m_blocked_gc = block;
        return was;
    }

    inline void requestGC()
    {
        m_force_gc = true;
    }

    inline void adjustBytesAllocated(int bytes)
    { m_new_allocated_extra_bytes += bytes; }

    inline bool poll()
    {
        if (m_blocked_gc || ! m_head)
            return false;

        else if (m_force_gc) {
            m_force_gc = false;
            return true;
        }

        else if (m_free && ! m_free->next)
            return true;

        return (m_new_allocated_blocks >= MaxNumberOfBlocks)
            || ((m_new_allocated_extra_bytes >= MaxNumberOfExtraBytes)
                && (m_new_allocated_blocks > 0));
    }

    inline int generation(_Tp *ptr) const
    { return GCBlock::get(ptr)->generation; }

    inline GCBlock *head() const
    { return m_head; }

    void sweep(int generation)
    {
        m_sweeping = true;
        GCBlock *blk = m_head;
        m_current = 0;

        m_new_allocated_blocks = 0;
        m_new_allocated_extra_bytes = 0;

        while (blk != 0) {
            if (blk->generation != generation) {
                if (m_current)
                    m_current->next = blk->next;

                GCBlock *tmp = blk;
                blk = blk->next;    // advance the pointer

                tmp->next = m_free; // prepend the node to the free list...
                m_free = tmp;
                ++m_free_blocks;

                if (m_free == m_head)
                    m_head = blk;

                _Tp *data = reinterpret_cast<_Tp *>(tmp->data());
                data->finalize();
                tmp->~GCBlock();
            } else {
                m_current = blk;
                blk = blk->next;
            }
        }

        if (! m_current)
            m_head = m_current;
        m_sweeping = false;
    }

    class const_iterator
    {
    public:
        typedef _Tp value_type;
        typedef const _Tp *pointer;
        typedef const _Tp &reference;
        inline const_iterator() : i(0) { }
        inline const_iterator(GCBlock *block) : i(block) { }
        inline const_iterator(const const_iterator &o)
        { i = reinterpret_cast<const const_iterator &>(o).i; }

        inline const _Tp *data() const { return reinterpret_cast<_Tp*>(i->data()); }
        inline const _Tp &value() const { return *reinterpret_cast<_Tp*>(i->data()); }
        inline const _Tp &operator*() const { return *reinterpret_cast<_Tp*>(i->data()); }
        inline const _Tp *operator->() const { return reinterpret_cast<_Tp*>(i->data()); }
        inline bool operator==(const const_iterator &o) const { return i == o.i; }
        inline bool operator!=(const const_iterator &o) const { return i != o.i; }

        inline const_iterator &operator++() {
            i = i->next;
            return *this;
        }
    private:
        GCBlock *i;
    };
    friend class const_iterator;

    inline const_iterator constBegin() const { return const_iterator(m_head); }
    inline const_iterator constEnd() const { return const_iterator(0); }
    
private:
    Q_DISABLE_COPY(GCAlloc)
};

} // namespace QScript

QT_END_NAMESPACE

#endif // QSCRIPT_GC_H
