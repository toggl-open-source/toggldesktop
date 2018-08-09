#ifndef AUTOCOMPLETECOMBOBOX_H
#define AUTOCOMPLETECOMBOBOX_H

#include <QComboBox>
#include <QKeyEvent>
#include <QDebug>
#include "./autocompletedropdownlist.h"

class AutocompleteCombobox : public QComboBox
{
    Q_OBJECT
    public:
        AutocompleteCombobox(QWidget* parent);
        ~AutocompleteCombobox();
        void setView(QAbstractItemView *itemView);
        AutocompleteDropdownList *list;

    protected:
        void keyPressEvent(QKeyEvent *e);

    private slots:
        void keyPress(QKeyEvent *e);
};

#endif // AUTOCOMPLETECOMBOBOX_H
