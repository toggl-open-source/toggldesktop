#include "autocompletecombobox.h"
#include "autocompletelistmodel.h"
#include "autocompleteitemview.h"

#include <QDebug>

AutocompleteComboBox::AutocompleteComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setEditable(true);
    setView(new AutocompleteDropdownList(this));
}

void AutocompleteComboBox::setModel(QAbstractItemModel *model) {
    QComboBox::setModel(model);
    setCompleter(new AutocompleteCompleter(this));
    completer()->setModel(model);
    completer()->popup()->setItemDelegate(new AutoCompleteItemDelegate(this));
}

void AutocompleteComboBox::onModelChanged() {
}

AutocompleteCompleter::AutocompleteCompleter(QWidget *parent)
    : QCompleter(parent)
{
    setCaseSensitivity(Qt::CaseInsensitive);
    setFilterMode(Qt::MatchContains);
    setPopup(new AutocompleteDropdownList(parent));
}
