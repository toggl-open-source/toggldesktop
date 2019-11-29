#include "qmlobjectlist.h"
#include <QMetaProperty>
#include <QQmlEngine>
#include <QDebug>

#define ValidateIndex(m_i) (m_i < 0 || m_i >= rowCount())

void QmlObjectList::append(const QVariantMap& properties)
{
    QObject* newObj = mMetaObject.newInstance();
    if(newObj == Q_NULLPTR) {
        qWarning("invalid constructor");
        return;
    }
    for(const QString& key : properties.keys()) {
        if(!newObj->setProperty(key.toUtf8().data(), properties.value(key)))
            qWarning()<<"append object with invalid property"<<key;
    }
    append(newObj);
}

void QmlObjectList::prepend(QObject *object) {
    Q_ASSERT(object->metaObject() == &mMetaObject);
    beginInsertRows(QModelIndex(), 0, 0);
    mData.prepend(QObjectPointer(object));
    endInsertRows();
}

void QmlObjectList::append(QObject *object)
{
    insert(rowCount(), object);
}

bool QmlObjectList::insert(const int& i, QObject* object)
{
    Q_ASSERT(object->metaObject() == &mMetaObject);
    if(i < 0 || i > rowCount())
        return false;
    beginInsertRows(QModelIndex(), i, i);
    mData.insert(i, QObjectPointer(object));
    endInsertRows();
    return true;
}

int QmlObjectList::count() {
    return rowCount();
}

bool QmlObjectList::removeRow(int row, const QModelIndex &parent)
{
    const int first = row;
    const int last = row;
    if(ValidateIndex(first) || ValidateIndex(last))
        return false;
    beginRemoveRows(parent, first, last);
    mData.removeAt(row);
    endRemoveRows();
    return true;
}

QVariant QmlObjectList::data(const QModelIndex &index, int role) const
{
    if(ValidateIndex(index.row()))
        return QVariant();
    const QObjectPointer& data = mData[index.row()];
    if(data.isNull()) {
        qWarning()<<__FUNCTION__<<"data is null";
        return QVariant();
    }
    return QVariant::fromValue(data.data());
}

QHash<int, QByteArray> QmlObjectList::roleNames() const
{
    return { { Qt::UserRole, "modelData" } };
}

int QmlObjectList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mData.count();
}

QmlObjectList::QmlObjectList(const QMetaObject &m, QObject *parent) :
    QAbstractListModel(parent),
    mMetaObject(m)
{ }

void QmlObjectList::clear() {
    beginResetModel();
    mData.clear();
    endResetModel();
}
