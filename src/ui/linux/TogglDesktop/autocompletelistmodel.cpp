#include "autocompletelistmodel.h"

#include <QDebug>
#include <QQmlEngine>

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

QHash<int, QByteArray> AutocompleteListModel::roleNames() const {
    return { { Qt::UserRole, "modelData" } };
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


AutocompleteProxyModel::AutocompleteProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent) {
    setFilterRole(Qt::UserRole);
}

int AutocompleteProxyModel::count() {
    return rowCount(QModelIndex());
}

void AutocompleteProxyModel::setFilter(const QString &filter) {
    setFilterRegExp(filter);
}

AutocompleteView *AutocompleteProxyModel::get(int idx) {
    if (idx >= 0 && idx < rowCount()) {
        auto d = data(index(idx, 0), Qt::UserRole);
        if (!d.isValid())
            return nullptr;
        auto ptr = qvariant_cast<AutocompleteView*>(d);
        if (!ptr)
            return nullptr;
        QQmlEngine::setObjectOwnership(ptr, QQmlEngine::CppOwnership);
        return ptr;
    }
    return nullptr;
}

bool AutocompleteProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    QString input = filterRegExp().pattern();
    QStringList words = input.split(" ");

    auto variant = sourceModel()->data(sourceModel()->index(source_row, 0), Qt::UserRole);
    if (!variant.isValid())
        return false;
    auto view = qvariant_cast<AutocompleteView*>(variant);
    if (!view)
        return false;
    for (auto word : words) {
        if (word.isEmpty() && words.count() > 1)
            continue;
        if (view->_Children.isEmpty()) {
            if (view->Description.contains(word, Qt::CaseInsensitive))
                    return true;
            if (view->Text.contains(word, Qt::CaseInsensitive))
                    return true;
        }
        for (auto v : view->_Children) {
            if (v->Description.contains(word, Qt::CaseInsensitive))
                    return true;
            if (v->Text.contains(word, Qt::CaseInsensitive))
                    return true;
        }
    }

    return false;
}
