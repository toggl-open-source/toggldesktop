#ifndef AUTOCOMPLETECOMBOBOX_H
#define AUTOCOMPLETECOMBOBOX_H

#include <QComboBox>
#include <QLineEdit>
#include <QCompleter>

class AutocompleteComboBox : public QComboBox {
    friend class AutocompleteLineEdit;
    Q_OBJECT
public:
    AutocompleteComboBox(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onModelChanged();
};

class AutocompleteLineEdit : public QLineEdit {
    Q_OBJECT
public:
    AutocompleteLineEdit(AutocompleteComboBox *parent = nullptr);

    AutocompleteComboBox *comboBox();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

class AutocompleteCompleter : public QCompleter {
    Q_OBJECT
public:
    AutocompleteCompleter(QWidget *parent = nullptr);
};

#endif // AUTOCOMPLETECOMBOBOX_H
