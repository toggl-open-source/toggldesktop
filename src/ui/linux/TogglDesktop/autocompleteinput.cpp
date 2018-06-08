#include "autocompleteinput.h"
#include <QDebug>
#include <QKeyEvent>

AutocompleteInput::AutocompleteInput(QWidget *parent) : QLineEdit(parent)
{
    autocompleteDropdown = new AutocompleteDropdown(this);
    connect(this, SIGNAL(textEdited(const QString &)), this, SLOT(onTextEdited(const QString &)));
}

void AutocompleteInput::onTextEdited(const QString &text){
    filter(text);
    reloadDropdown();
    updateAndShow();
}

void AutocompleteInput::updateAndShow() {
    int newX = window()->pos().x() + this->pos().x() - 5; // minus dropshadow size
    int newY = window()->pos().y() + this->pos().y() + 74;
    autocompleteDropdown->move(newX, newY);
    autocompleteDropdown->show();
}

void AutocompleteInput::setData(QVector<AutocompleteView *> list){
    data = list;
    filteredData = list;
}

void AutocompleteInput::filter(const QString &input){
    QVector<AutocompleteView *> filtered;
    foreach(AutocompleteView *view, data) {
        if(view->Text.contains(input, Qt::CaseInsensitive)){
            filtered.push_back(view);
        }
    }
    filteredData = filtered;
}

void AutocompleteInput::reloadDropdown(){
    if (filteredData.size() == 0) {
        autocompleteDropdown->close();
        return;
    }
    autocompleteDropdown->reload(filteredData);
}

void AutocompleteInput::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        //select item
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        autocompleteDropdown->close();
        return;
    }
    if (!autocompleteDropdown->isVisible()) {
        if (event->key() == Qt::Key_Down) {
            reloadDropdown();
            updateAndShow();
        }
        return;
    } else {
        if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up) {
            autocompleteDropdown->onKeyPressEvent(event);
            return;
        }
    }
    QLineEdit::keyPressEvent(event);
}



