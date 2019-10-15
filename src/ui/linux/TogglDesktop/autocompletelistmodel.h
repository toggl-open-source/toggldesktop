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

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QVector<AutocompleteView *> list;
    uint64_t displayItem;
};

#endif // AUTOCOMPLETELISTMODEL_H
