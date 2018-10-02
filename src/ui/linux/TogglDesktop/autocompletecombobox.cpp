#include "autocompletecombobox.h"

AutocompleteCombobox::AutocompleteCombobox(QWidget* parent) :
    QComboBox(parent),
    timer(new QTimer(this))
{
    timer->setSingleShot(true);
    // setup timer signal and slot
    connect(timer, SIGNAL(timeout()),
        this, SLOT(triggerFilter()));
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
    bool modifiers = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (modifiers) {
        QComboBox::keyPressEvent(e);
        return;
    }

    if (e->key() == Qt::Key_Down && list->count() > 0) {
        //qDebug() << "Open popup";
        showPopup();
        return;
    }

    QComboBox::keyPressEvent(e);
    //qDebug() << "FILTER: " << currentText();

    if (timer->isActive()) {
        timer->stop();
    }

    timer->start(200);
}

void AutocompleteCombobox::triggerFilter()
{
    QString lastText = currentText();
    if (list->filterItems(currentText())) {
        showPopup();
    } else {
        hidePopup();
    }
    setEditText(lastText);
}
