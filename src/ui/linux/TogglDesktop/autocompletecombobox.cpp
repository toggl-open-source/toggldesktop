#include "autocompletecombobox.h"
#include "autocompletelistmodel.h"
#include "autocompletelistview.h"

#include <QDebug>

AutocompleteComboBox::AutocompleteComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setEditable(true);
    setView(new AutocompleteListView(this));
    setLineEdit(new AutocompleteLineEdit(this));
}

void AutocompleteComboBox::setModel(QAbstractItemModel *model) {
    QComboBox::setModel(model);
    setCompleter(new AutocompleteCompleter(this));
    completer()->setModel(model);
    completer()->popup()->setItemDelegate(new AutoCompleteItemDelegate(this));
}

void AutocompleteComboBox::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
        QComboBox::keyPressEvent(event);
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
    setCaseSensitivity(Qt::CaseInsensitive);
    setFilterMode(Qt::MatchContains);
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
        QString previous = text();
        QLineEdit::keyPressEvent(event);
        if (text().isEmpty() && !previous.isEmpty()) {
            setText(previous);
        }
        break;
    }
    default:
        QLineEdit::keyPressEvent(event);
    }
}
