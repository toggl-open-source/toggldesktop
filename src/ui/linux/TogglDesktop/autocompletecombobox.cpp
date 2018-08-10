#include "autocompletecombobox.h"

AutocompleteCombobox::AutocompleteCombobox(QWidget* parent) : QComboBox(parent)
{

}

AutocompleteCombobox::~AutocompleteCombobox()
{
}

void AutocompleteCombobox::setView(QAbstractItemView *itemView)
{
    list = static_cast<AutocompleteDropdownList *>(itemView);
    QComboBox::setView(itemView);

    connect(list, SIGNAL(keyPress(QKeyEvent *)),  // NOLINT
                this, SLOT(keyPress(QKeyEvent *)));  // NOLINT

}

void AutocompleteCombobox::keyPress(QKeyEvent *e)
{
    keyPressEvent(e);
}

void AutocompleteCombobox::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Down && list->count() > 0) {
        qDebug() << "Open popup";
        showPopup();
        return;
    }

    QComboBox::keyPressEvent(e);
    QString lastText = currentText();
    qDebug() << "FILTER: " << currentText();

    if (list->filterItems(currentText())) {
        showPopup();
    } else {
        hidePopup();
    }
    setEditText(lastText);

}
