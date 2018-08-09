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

    //AutocompleteDropdown *d = static_cast<AutocompleteDropdown *>(view());
    list->filterItems(currentText());
    showPopup();


/*
    if (c && c->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // Let the completer do default behavior
        }
    }

    bool isShortcut = (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E;
    if (!isShortcut)
        QComboBox::keyPressEvent(e); // Don't send the shortcut (CTRL-E) to the text edit.

    if (!c)
        return;

    bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!isShortcut && !ctrlOrShift && e->modifiers() != Qt::NoModifier)
    {
        c->popup()->hide();
        return;
    }

    if (e->key() == Qt::Key_Down) {
        c->popup()->show();
    }
    c->update(text());
    c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    */


    if (e->key() == Qt::Key_Down) {
        qDebug() << "Open popup";
        showPopup();
    }
}
