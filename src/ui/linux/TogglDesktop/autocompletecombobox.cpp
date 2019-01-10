#include "autocompletecombobox.h"
#include "autocompletelistmodel.h"
#include "autocompletelistview.h"

#include <QDebug>

AutocompleteComboBox::AutocompleteComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setEditable(true);
    setView(new AutocompleteListView(this));
    auto lineEdit = new AutocompleteLineEdit(this);
    auto proxyModel = new AutocompleteProxyModel(this);
    connect(lineEdit, &QLineEdit::textChanged, proxyModel, &AutocompleteProxyModel::setFilterFixedString);
    setLineEdit(lineEdit);
    //QComboBox::setModel(proxyModel);
    setCompleter(new AutocompleteCompleter(this));
    completer()->setModel(proxyModel);
    completer()->popup()->setItemDelegate(new AutoCompleteItemDelegate(this));
}

void AutocompleteComboBox::setModel(QAbstractItemModel *model) {
    qobject_cast<AutocompleteProxyModel*>(this->completer()->model())->setSourceModel(model);
    //QComboBox::setModel(model);
}

void AutocompleteComboBox::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
        QComboBox::keyPressEvent(event);
        break;
    case Qt::Key_Down:
    case Qt::Key_Up:
        showPopup();
        break;
    default:
        QComboBox::keyPressEvent(event);
    }
}

void AutocompleteComboBox::onModelChanged() {
}

AutocompleteCompleter::AutocompleteCompleter(QWidget *parent)
    : QCompleter(parent)
{
    //setCaseSensitivity(Qt::CaseInsensitive);
    //setFilterMode(Qt::MatchContains);
    setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    setPopup(new AutocompleteListView(parent));
}

AutocompleteLineEdit::AutocompleteLineEdit(AutocompleteComboBox *parent)
    : QLineEdit(parent)
{

}

AutocompleteComboBox *AutocompleteLineEdit::comboBox() {
    return qobject_cast<AutocompleteComboBox*>(parent());
}

void AutocompleteLineEdit::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape: {
        // remembering the edit string is a HACK, this should be done in a different way
        // not sure how to do it atm
        /*
        QString previous = text();
        QLineEdit::keyPressEvent(event);
        if (text().isEmpty() && !previous.isEmpty()) {
            setText(previous);
        }
        break;
        */
    }
    default:
        QLineEdit::keyPressEvent(event);
    }
}

AutocompleteProxyModel::AutocompleteProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent) {
    setFilterRole(Qt::UserRole);
}

bool AutocompleteProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    QString input = filterRegExp().pattern();
    QStringList words = input.split(" ");

    auto view = qvariant_cast<AutocompleteView*>(sourceModel()->data(sourceModel()->index(source_row, 0), Qt::UserRole));
    for (auto word : words) {
        if (view->Description.contains(word, Qt::CaseInsensitive))
                return true;
        if (view->Text.contains(word, Qt::CaseInsensitive))
                return true;
    }

    return false;
}
