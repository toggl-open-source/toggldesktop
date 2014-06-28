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


#include "qscriptvalueimpl_p.h"


#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

QT_BEGIN_NAMESPACE

static void dfs(QScriptObject *instance, QHash<QScriptObject*, int> &dfn, int n)
{
    bool found = dfn.contains(instance);
    dfn[instance] = n;

    if (found)
        return;

    if (instance->m_prototype.isObject())
        dfs (instance->m_prototype.m_object_value, dfn, n + 1);

    if (instance->m_scope.isObject())
        dfs (instance->m_scope.m_object_value, dfn, n + 1);
}


static bool checkCycle(QScriptObject *instance, const QHash<QScriptObject*, int> &dfn)
{
    int n = dfn.value(instance);

    if (instance->m_prototype.isObject()) {
        if (n >= dfn.value(instance->m_prototype.m_object_value))
            return true;
    }

    if (instance->m_scope.isObject()) {
        if (n >= dfn.value(instance->m_scope.m_object_value))
            return true;
    }

    return false;
}

bool QScriptValueImpl::detectedCycle() const
{
    QHash<QScriptObject*, int> dfn;
    dfs(m_object_value, dfn, 0);
    return checkCycle(m_object_value, dfn);
}

bool QScriptValueImpl::instanceOf(const QScriptValueImpl &value) const
{
    if (! isObject() || ! value.isObject() || !value.implementsHasInstance())
        return false;
    return value.hasInstance(*this);
}

bool QScriptValueImpl::implementsHasInstance() const
{
    Q_ASSERT(isObject());
    if (isFunction())
        return true;
    if (QScriptClassData *odata = classInfo()->data()) {
        return odata->implementsHasInstance(*this);
    }
    return false;
}

bool QScriptValueImpl::hasInstance(const QScriptValueImpl &value) const
{
    Q_ASSERT(isObject());

    if (QScriptClassData *odata = classInfo()->data()) {
        if (odata->implementsHasInstance(*this))
            return odata->hasInstance(*this, value);
    }
    if (!isFunction())
        return false;

    // [[HasInstance] for function objects

    if (!value.isObject())
        return false;

    QScriptEnginePrivate *eng = engine();
    QScriptValueImpl proto = property(eng->idTable()->id_prototype);
    if (!proto.isObject()) {
        QScriptContextPrivate *ctx = eng->currentContext();
        ctx->throwTypeError(QLatin1String("instanceof: 'prototype' property is not an object"));
        return false;
    }

    QScriptObject *target = proto.m_object_value;
    QScriptValueImpl v = value;
    while (true) {
        v = v.prototype();
        if (!v.isObject())
            break;
        if (target == v.m_object_value)
            return true;
    }
    return false;
}

bool QScriptValueImpl::resolve_helper(QScriptNameIdImpl *nameId, QScript::Member *member,
                                      QScriptValueImpl *object, QScriptValue::ResolveFlags mode,
                                      QScript::AccessMode access) const
{
    QScriptObject *object_data = m_object_value;

    QScriptEnginePrivate *eng_p = engine();

    if (nameId == eng_p->idTable()->id___proto__) {
        member->native(nameId, /*id=*/0, QScriptValue::Undeletable);
        *object = *this;
        return true;
    }

    // If not found anywhere else, search in the extra members.
    if (QScriptClassData *odata = classInfo()->data()) {
        *object = *this;

        if (odata->resolve(*this, nameId, member, object, access))
            return true;
    }

    if (mode & QScriptValue::ResolvePrototype) {
        // For values and other non object based types, search in class's prototype
        const QScriptValueImpl &proto = object_data->m_prototype;

        if (proto.isObject()
            && proto.resolve(nameId, member, object, mode, access)) {
            return true;
        }
    }

    if ((mode & QScriptValue::ResolveScope) && object_data->m_scope.isValid())
        return object_data->m_scope.resolve(nameId, member, object, mode, access);

    return false;
}

void QScriptValueImpl::setProperty(QScriptNameIdImpl *nameId,
                                   const QScriptValueImpl &value,
                                   const QScriptValue::PropertyFlags &flags)
{
    if (!isObject())
        return;

    QScriptValueImpl base;
    QScript::Member member;

    QScriptValue::ResolveFlags mode = QScriptValue::ResolveLocal;
    // if we are not setting a setter or getter, look in prototype too
    if (!(flags & (QScriptValue::PropertyGetter | QScriptValue::PropertySetter)))
        mode |= QScriptValue::ResolvePrototype;

    if (resolve(nameId, &member, &base, mode, QScript::ReadWrite)) {
        // we resolved an existing property with that name
        if (flags & (QScriptValue::PropertyGetter | QScriptValue::PropertySetter)) {
            // setting the getter or setter of a property in this object
            if (member.isNativeProperty()) {
                if (value.isValid()) {
                    qWarning("QScriptValue::setProperty() failed: "
                             "cannot set getter or setter of native property `%s'",
                             qPrintable(nameId->s));
                }
                return;
            }
            if (member.isSetter()) {
                // the property we resolved is a setter
                if (!(flags & QScriptValue::PropertySetter) && !member.isGetter()) {
                    // find the getter, if not, create one
                    if (!m_object_value->findGetter(&member)) {
                        if (!value.isValid())
                            return; // don't create property for invalid value
                        createMember(nameId, &member, flags);
                    }
                }
            } else if (member.isGetter()) {
                // the property we resolved is a getter
                if (!(flags & QScriptValue::PropertyGetter)) {
                    // find the setter, if not, create one
                    if (!m_object_value->findSetter(&member)) {
                        if (!value.isValid())
                            return; // don't create property for invalid value
                        createMember(nameId, &member, flags);
                    }
                }
            } else {
                // the property is a normal property -- change the flags
                uint newFlags = flags & ~QScript::Member::InternalRange;
                newFlags |= QScript::Member::ObjectProperty;
                member.resetFlags(newFlags);
                base.m_object_value->m_members[member.id()].resetFlags(newFlags);
            }
            Q_ASSERT(member.isValid());
            if (!value.isValid()) {
                // remove the property
                removeMember(member);
                return;
            }
        } else {
            // setting the value
            if (member.isGetterOrSetter()) {
                // call the setter
                QScriptValueImpl setter;
                if (member.isObjectProperty() && !member.isSetter()) {
                    if (!base.m_object_value->findSetter(&member)) {
                        qWarning("QScriptValue::setProperty() failed: "
                                 "property '%s' has a getter but no setter",
                                 qPrintable(nameId->s));
                        return;
                    }
                }
                base.get(member, &setter);
                setter.call(*this, QScriptValueImplList() << value);
                return;
            } else {
                if (base.m_object_value != m_object_value) {
                    if (!value.isValid())
                        return; // don't create property for invalid value
                    createMember(nameId, &member, flags);
                    base = *this;
                } else {
                    if (!value.isValid()) {
                        // remove the property
                        removeMember(member);
                        return;
                    }
                }
                if (flags != QScriptValue::KeepExistingFlags) {
                    // change flags
                    if (member.isNativeProperty()) {
                        qWarning("QScriptValue::setProperty(%s): "
                                 "cannot change flags of a native property",
                                 qPrintable(nameId->s));
                    } else {
                        uint newFlags = member.flags() & QScript::Member::InternalRange;
                        newFlags |= flags & ~QScript::Member::InternalRange;
                        base.m_object_value->m_members[member.id()].resetFlags(newFlags);
                    }
                }
            }
        }
    } else {
        // property does not exist
        if (!value.isValid())
            return; // don't create property for invalid value
        createMember(nameId, &member, flags & ~QScript::Member::InternalRange);
        base = *this;
    }

    base.put(member, value);
}

QVariant QScriptValueImpl::toVariant() const
{
    switch (m_type) {
    case QScript::InvalidType:
        return QVariant();

    case QScript::UndefinedType:
    case QScript::NullType:
    case QScript::PointerType:
    case QScript::ReferenceType:
        break;

    case QScript::BooleanType:
        return QVariant(m_bool_value);

    case QScript::IntegerType:
        return QVariant(m_int_value);

    case QScript::NumberType:
        return QVariant(m_number_value);

    case QScript::StringType:
        return QVariant(m_string_value->s);

    case QScript::LazyStringType:
        return QVariant(*m_lazy_string_value);

    case QScript::ObjectType:
        if (isDate())
            return QVariant(toDateTime());

#ifndef QT_NO_REGEXP
        if (isRegExp())
            return QVariant(toRegExp());
#endif
        if (isVariant())
            return variantValue();

#ifndef QT_NO_QOBJECT
        if (isQObject())        
            return qVariantFromValue(toQObject());
#endif
        if (isArray())
            return QScriptEnginePrivate::variantListFromArray(*this);

        QScriptValueImpl v = engine()->toPrimitive(*this);
        if (!v.isObject())
            return v.toVariant();
        break;
    } // switch
    return QVariant();
}

QDebug &operator<<(QDebug &d, const QScriptValueImpl &object)
{
    d.nospace() << "QScriptValue(";

    switch (object.type()) {
    case QScript::InvalidType:
        d.nospace() << "Invalid)";
        return d;

    case QScript::BooleanType:
        d.nospace() << "bool=" << object.toBoolean();
        break;

    case QScript::IntegerType:
        d.nospace() << "int=" << object.toInt32();
        break;

    case QScript::NumberType:
        d.nospace() << "qsreal=" << object.toNumber();
        break;

    case QScript::LazyStringType:
    case QScript::StringType:
        d.nospace() << "string=" << object.toString();
        break;

    case QScript::ReferenceType:
        d.nospace() << "reference";
        break;

    case QScript::NullType:
        d.nospace() << "null";
        break;

    case QScript::UndefinedType:
        d.nospace() << "undefined";
        break;

    case QScript::PointerType:
        d.nospace() << "pointer";
        break;

    case QScript::ObjectType:
        d.nospace() << object.classInfo()->name() << ",{";
        QScriptObject *od = object.objectValue();
        for (int i=0; i<od->memberCount(); ++i) {
            if (i != 0)
                d << ',';

            QScript::Member m;
            od->member(i, &m);

            if (m.isValid() && m.isObjectProperty()) {
                d << object.engine()->toString(m.nameId());
                QScriptValueImpl o;
                od->get(m, &o);
                d.nospace() << QLatin1Char(':')
                            << (o.classInfo()
                                ? o.classInfo()->name()
                                : QLatin1String("?"));
            }
        }

        d.nospace() << "} scope={";
        QScriptValueImpl scope = object.scope();
        while (scope.isValid()) {
            Q_ASSERT(scope.isObject());
            d.nospace() << ' ' << scope.objectValue();
            scope = scope.scope();
        }
        d.nospace() << '}';
        break;
    }

    d << ')';
    return d;
}

void QScriptValueImpl::destroyObjectData()
{
    Q_ASSERT(isObject());
    m_object_value->finalizeData();
}

bool QScriptValueImpl::isMarked(int generation) const
{
    if (isString())
        return (m_string_value->used != 0);
    else if (isObject()) {
        QScript::GCBlock *block = QScript::GCBlock::get(m_object_value);
        return (block->generation == generation);
    }
    return false;
}

QT_END_NAMESPACE

