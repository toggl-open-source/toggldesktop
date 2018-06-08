#include "autocompleteinput.h"
#include <QDebug>

AutocompleteInput::AutocompleteInput(QWidget *parent) : QLineEdit(parent)
{
    autocompleteDropdown = new AutocompleteDropdown(this);

    connect(this, SIGNAL(textEdited(const QString &)), this, SLOT(onTextEdited(const QString &)));
}

void AutocompleteInput::onTextEdited(const QString &text){

    int newX = window()->pos().x() + this->pos().x() - 10; // minus dropshadow size
    int newY = window()->pos().y() + this->pos().y() + 80;

    qDebug() << "Changed: " << text << " [" << newX << ", " << newY << "]";
    filter(text);
    reloadDropdown();

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
            qDebug() << view->Text << " | " << view->Type;
        }
    }
    filteredData = filtered;
    qDebug() << "==========================";
}

void AutocompleteInput::reloadDropdown(){
    qDebug() << "reloadDropdown";

    autocompleteDropdown->reload(filteredData);
}



