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
    qDebug() << "TRGIRRERED: " << currentText();

    keyPressEvent(e);
}

void AutocompleteCombobox::keyPressEvent(QKeyEvent *e)
{
    bool isShortcut = (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E;
    if (!isShortcut)
        QComboBox::keyPressEvent(e); // Don't send the shortcut (CTRL-E) to the text edit.

    qDebug() << "FILTER: " << currentText();

    list->filterItems(currentText());
    showPopup();

    if (e->key() == Qt::Key_Down) {
        qDebug() << "Open popup";
        showPopup();
    }
}
