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


#ifndef QSCRIPTBUFFER_P_H
#define QSCRIPTBUFFER_P_H

#include <qglobal.h>

#if defined(Q_OS_VXWORKS) && defined(m_data)
#  undef m_data
#endif


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

namespace QScript
{
    template <typename T> class Buffer
    {
    public:
        typedef T *iterator;
        typedef const T *const_iterator;

        Buffer() : m_data(0), m_capacity(0), m_size(0) { }
        ~Buffer() { delete [] m_data; }

        inline void reserve(int num);
        inline void reset();
        inline void clear();
        inline void append(const T &t);

        inline iterator begin();
        inline iterator end();

        inline const_iterator begin() const;
        inline const_iterator end() const;

        inline int size() const;
        inline void resize(int s);

        inline int capacity() const;
        inline T *data();
        inline const T *constData() const;

        inline T &last();
        inline T &takeLast();

        inline T &at(int i) { return (*this)[i]; }
        inline const T &at(int i) const { return (*this)[i]; }

        inline T &operator[](int i);
        inline const T &operator[](int i) const;

        inline bool isEmpty() const;

    private:
        T *m_data;
        int m_capacity;
        int m_size;

    private:
        Q_DISABLE_COPY(Buffer)
    };

} // namespace QScript

template <typename T> T *QScript::Buffer<T>::data() { return m_data; }
template <typename T> const T *QScript::Buffer<T>::constData() const { return m_data; }
template <typename T> void QScript::Buffer<T>::reset() { m_size = 0; }
template <typename T> int QScript::Buffer<T>::capacity() const { return m_capacity; }

template <typename T> int QScript::Buffer<T>::size() const { return m_size; }
template <typename T> void QScript::Buffer<T>::resize(int s)
{
    if (m_capacity < s)
        reserve (s << 1);

    m_size = s;
}

template <typename T> void QScript::Buffer<T>::clear()
{
    delete [] m_data;
    m_data = 0;
    m_size = 0;
    m_capacity = 0;
}

template <typename T> void QScript::Buffer<T>::reserve(int x)
{
    // its an ever expanding buffer so it never gets smaller..
    if (x < m_capacity)
        return;
    m_capacity = x;
    T *new_data = new T[m_capacity];
    for (int i=0; i<m_size; ++i)
        new_data[i] = m_data[i];
    delete [] m_data;
    m_data = new_data;
}

template <typename T> void QScript::Buffer<T>::append(const T &t)
{
    if (m_size == m_capacity)
        reserve(m_capacity + 32);
    m_data[m_size++] = t;
}

template <typename T> T &QScript::Buffer<T>::operator[](int i)
{
    Q_ASSERT(i >= 0);
    Q_ASSERT(i < m_size);
    return m_data[i];
}

template <typename T> const T &QScript::Buffer<T>::operator[](int i) const
{
    Q_ASSERT(i >= 0);
    Q_ASSERT(i < m_size);
    return m_data[i];
}

template <typename T> bool QScript::Buffer<T>::isEmpty() const
{
    return m_size == 0;
}

template <typename T> T &QScript::Buffer<T>::takeLast()
{
    Q_ASSERT(!isEmpty());
    --m_size;
    return m_data[m_size];
}

template <typename T> T &QScript::Buffer<T>::last()
{
    return m_data[m_size - 1];
}

template <typename T> typename QScript::Buffer<T>::iterator QScript::Buffer<T>::begin()
{
    return m_data;
}

template <typename T> typename QScript::Buffer<T>::iterator QScript::Buffer<T>::end()
{
    return m_data + m_size;
}

template <typename T> typename QScript::Buffer<T>::const_iterator QScript::Buffer<T>::begin() const
{
    return m_data;
}

template <typename T> typename QScript::Buffer<T>::const_iterator QScript::Buffer<T>::end() const
{
    return m_data + m_size;
}

QT_END_NAMESPACE

#endif
