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


#ifndef QSCRIPTVALUEIMPLFWD_P_H
#define QSCRIPTVALUEIMPLFWD_P_H

#include "qscriptglobals_p.h"


#include "qscriptvalue.h"

#include <qstring.h>
#include <qlist.h>

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

#if defined(Q_OS_VXWORKS) && defined(m_type)
#  undef m_type
#endif

class QScriptValueImpl;
typedef QList<QScriptValueImpl> QScriptValueImplList;

class QScriptClassInfo;
class QScriptObject;
class QScriptObjectData;
class QScriptNameIdImpl;
class QScriptFunction;
class QScriptEnginePrivate;

namespace QScript
{
    class Member;
}

class QScriptValueImpl
{
public:
    enum TypeHint {
        NoTypeHint,
        NumberTypeHint,
        StringTypeHint
    };

    inline QScriptValueImpl();
    inline QScriptValueImpl(QScriptValue::SpecialValue val);
    inline QScriptValueImpl(bool val);
    inline QScriptValueImpl(int val);
    inline QScriptValueImpl(uint val);
    inline QScriptValueImpl(qsreal val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, const QString &val);
    inline QScriptValueImpl(QScriptNameIdImpl *val);

    inline QScript::Type type() const;
    inline QScriptEnginePrivate *engine() const;
    inline QScriptClassInfo *classInfo() const;
    inline void setClassInfo(QScriptClassInfo *cls);
    inline QScriptNameIdImpl *stringValue() const;
    inline QScriptObject *objectValue() const;
    inline void incr();
    inline void decr();

    inline void invalidate();
    inline bool isValid() const;
    inline bool isBoolean() const;
    inline bool isNumber() const;
    inline bool isString() const;
    inline bool isFunction() const;
    inline bool isObject() const;
    inline bool isUndefined() const;
    inline bool isNull() const;
    inline bool isVariant() const;
    inline bool isQObject() const;
    inline bool isQMetaObject() const;
    inline bool isReference() const;

    inline bool isError() const;
    inline bool isArray() const;
    inline bool isDate() const;
    inline bool isRegExp() const;

    inline QString toString() const;
    inline qsreal toNumber() const;
    inline bool toBoolean() const;
    inline qsreal toInteger() const;
    inline qint32 toInt32() const;
    inline quint32 toUInt32() const;
    inline quint16 toUInt16() const;
    QVariant toVariant() const;
    inline QObject *toQObject() const;
    inline const QMetaObject *toQMetaObject() const;
    inline QDateTime toDateTime() const;
#ifndef QT_NO_REGEXP
    inline QRegExp toRegExp() const;
#endif

    inline QVariant &variantValue() const;
    inline void setVariantValue(const QVariant &v);

    bool instanceOf(const QScriptValueImpl &value) const;
    bool instanceOf_helper(const QScriptValueImpl &value) const;

    bool implementsHasInstance() const;
    bool hasInstance(const QScriptValueImpl &value) const;

    inline QScriptValueImpl prototype() const;
    inline void setPrototype(const QScriptValueImpl &prototype);

    inline QScriptValueImpl property(QScriptNameIdImpl *nameId,
                                     const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;
    void setProperty(QScriptNameIdImpl *nameId, const QScriptValueImpl &value,
                     const QScriptValue::PropertyFlags &flags = QScriptValue::KeepExistingFlags);

    inline QScriptValueImpl property(const QString &name,
                              const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;
    inline void setProperty(const QString &name, const QScriptValueImpl &value,
                     const QScriptValue::PropertyFlags &flags = QScriptValue::KeepExistingFlags);

    inline QScriptValueImpl property(quint32 arrayIndex,
                              const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;
    inline void setProperty(quint32 arrayIndex, const QScriptValueImpl &value,
                     const QScriptValue::PropertyFlags &flags = QScriptValue::KeepExistingFlags);

    inline QScriptValue::PropertyFlags propertyFlags(const QString &name,
                                                     const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;
    inline QScriptValue::PropertyFlags propertyFlags(QScriptNameIdImpl *nameId,
                                                     const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;

    inline bool deleteProperty(QScriptNameIdImpl *nameId,
                               const QScriptValue::ResolveFlags &mode = QScriptValue::ResolveLocal);

    inline QScriptValueImpl call(const QScriptValueImpl &thisObject = QScriptValueImpl(),
                                 const QScriptValueImplList &args = QScriptValueImplList());
    inline QScriptValueImpl call(const QScriptValueImpl &thisObject,
                                 const QScriptValueImpl &arguments);
    inline QScriptValueImpl construct(const QScriptValueImplList &args = QScriptValueImplList());
    inline QScriptValueImpl construct(const QScriptValueImpl &arguments);

    inline void mark(int) const;
    bool isMarked(int) const;

    inline QScriptValueImpl internalValue() const;
    inline void setInternalValue(const QScriptValueImpl &internalValue);

    inline void setQObjectValue(QObject *object);

    inline QScriptObjectData *objectData() const;
    inline void setObjectData(QScriptObjectData *data);
    void destroyObjectData();

    inline void createMember(QScriptNameIdImpl *nameId,
                      QScript::Member *member, uint flags); // ### remove me
    inline int memberCount() const;
    inline void member(int index, QScript::Member *member) const;

    inline bool resolve(QScriptNameIdImpl *nameId, QScript::Member *member,
                        QScriptValueImpl *object, QScriptValue::ResolveFlags mode,
                        QScript::AccessMode access) const;
    bool resolve_helper(QScriptNameIdImpl *nameId, QScript::Member *member,
                        QScriptValueImpl *object, QScriptValue::ResolveFlags mode,
                        QScript::AccessMode access) const;
    inline void get(const QScript::Member &member, QScriptValueImpl *out) const;
    inline void get_helper(const QScript::Member &member, QScriptValueImpl *out) const;
    inline void get(QScriptNameIdImpl *nameId, QScriptValueImpl *out);
    inline void put(const QScript::Member &member, const QScriptValueImpl &value);
    inline void removeMember(const QScript::Member &member);

    inline QScriptValueImpl scope() const;
    inline void setScope(const QScriptValueImpl &scope);

    inline QScriptFunction *toFunction() const;

    bool detectedCycle() const;

    QScript::Type m_type;
    union {
        bool m_bool_value;
        int m_int_value;
        qsreal m_number_value;
        void *m_ptr_value;
        QScriptObject *m_object_value;
        QScriptNameIdImpl *m_string_value;
        QString *m_lazy_string_value;
    };
};

QT_END_NAMESPACE


#endif // QSCRIPTVALUEIMPLFWD_P_H
