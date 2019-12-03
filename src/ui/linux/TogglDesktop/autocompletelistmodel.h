#ifndef AUTOCOMPLETELISTMODEL_H
#define AUTOCOMPLETELISTMODEL_H

#include "autocompleteview.h"

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

class AutocompleteListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    AutocompleteListModel(QObject *parent = nullptr, QVector<AutocompleteView*> list = {}, uint64_t displayItem = AutocompleteView::AC_TIME_ENTRY);
    void setList(QVector<AutocompleteView *> autocompletelist);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QVector<AutocompleteView *> list;
    uint64_t displayItem;
};

class AutocompleteProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
    friend class AutocompleteComboBox;
public:
    AutocompleteProxyModel(QObject *parent = nullptr);

    Q_INVOKABLE int count();

    Q_INVOKABLE void setFilter(const QString &filter);
    Q_INVOKABLE AutocompleteView *get(int idx);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};


#endif // AUTOCOMPLETELISTMODEL_H
