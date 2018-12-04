#include "autocompletelistmodel.h"
#include <QDebug>

AutocompleteListModel::AutocompleteListModel(QObject *parent, QVector<AutocompleteView*> list) :
    QAbstractListModel(parent)
{
    setList(list);
}

void AutocompleteListModel::setList(QVector<AutocompleteView *> autocompletelist)
{
    if (list == autocompletelist)
        return;
    beginResetModel();
    list = autocompletelist;
    endResetModel();
}

Qt::ItemFlags AutocompleteListModel::flags(const QModelIndex &index) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int AutocompleteListModel::rowCount(const QModelIndex &parent) const {
    return list.count();
}

QVariant AutocompleteListModel::data(const QModelIndex &index, int role) const {
    if (index.row() >= list.count())
        return QVariant();
    auto view = list.at(index.row());
    if (!view)
        return QVariant();
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return view->Description;
    }
    if (role == Qt::UserRole)
        return QVariant::fromValue(list.at(index.row()));
    return QVariant();
}
