#ifndef AUTOCOMPLETECOMBOBOX_H
#define AUTOCOMPLETECOMBOBOX_H

#include <QComboBox>
#include <QCompleter>

class AutocompleteComboBox : public QComboBox {
    Q_OBJECT
public:
    AutocompleteComboBox(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);

private slots:
    void onModelChanged();
};

class AutocompleteCompleter : public QCompleter {
    Q_OBJECT
public:
    AutocompleteCompleter(QWidget *parent = nullptr);
};

#endif // AUTOCOMPLETECOMBOBOX_H
