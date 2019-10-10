#include "autocompletelistmodel.h"
#include <QDebug>

AutocompleteListModel::AutocompleteListModel(QObject *parent, QVector<AutocompleteView*> list, uint64_t displayItem) :
    QAbstractListModel(parent),
    displayItem(displayItem)
{
    setList(list);
}

void AutocompleteListModel::setList(QVector<AutocompleteView *> autocompletelist)
{
    if (list == autocompletelist)
        return;
    beginResetModel();
    for (auto i : list)
        i->deleteLater();
    list = autocompletelist;
    endResetModel();
}

Qt::ItemFlags AutocompleteListModel::flags(const QModelIndex &index) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= list.count())
        return Qt::NoItemFlags;
    auto view = list.at(index.row());
    switch (view->Type) {
    case 0:
    case 1:
    case 2:
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    default:
            return Qt::ItemIsSelectable;
    }
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
        if (view->Type == displayItem) {
            switch (displayItem) {
            case 2:
                return view->ProjectLabel;
            case 1:
                return view->TaskLabel;
            default:
                return view->Description;
            }
        }
        else if (view->Type == 1 && displayItem == 2){
            return QString("%1. %2").arg(view->TaskLabel).arg(view->ProjectLabel);
        }
        else {
            return QString();
        }
    }
    if (role == Qt::UserRole)
        return QVariant::fromValue(list.at(index.row()));
    return QVariant();
}
