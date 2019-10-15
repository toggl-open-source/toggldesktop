#ifndef QMLOBJECTLIST_H
#define QMLOBJECTLIST_H

#include <QMetaObject>
#include <QAbstractListModel>
#include <QSharedPointer>

typedef QSharedPointer<QObject> QObjectPointer;

class QmlObjectList : public QAbstractListModel
{
    Q_OBJECT
public:
    Q_DISABLE_COPY(QmlObjectList)
    virtual ~QmlObjectList() {
        mData.clear();
    }

    template <typename T>
    inline static QmlObjectList* create(QObject *parent = Q_NULLPTR) {
        return new QmlObjectList(T::staticMetaObject, parent);
    }

    void prepend(QObject* object);
    void append(QObject *object);

    bool insert(const int& i, QObject *object);

    int count();

    void clear();

    Q_INVOKABLE
    /**
     * @brief append
     * append an object to list and assign given properties.
     * @param properties
     * @warning if append new Object from QML is needed
     * have to assign the default value for the parameters,
     * and adding Q_INVOKABLE to constructor.
     */
    void append(const QVariantMap& properties);

    Q_INVOKABLE bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    Q_INVOKABLE inline void removeFirst() {
        if(!mData.isEmpty())
            removeRow(0);
    }

    Q_INVOKABLE inline void removeLast() {
        if(!mData.isEmpty())
            removeRow(rowCount() - 1);
    }

    Q_INVOKABLE inline QVariant at(const int& i) {
        return QVariant::fromValue(mData.at(i));
    }

protected:
    QVariant data(const QModelIndex &index, int role) const;

    QHash<int, QByteArray> roleNames() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
private:
    QmlObjectList(const QMetaObject& m, QObject *parent = Q_NULLPTR);

    const QMetaObject&              mMetaObject;
    QList<QObjectPointer>         mData;
};

#endif // QMLOBJECTLIST_H
