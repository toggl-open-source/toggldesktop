#ifndef AUTOCOMPLETEINPUT_H
#define AUTOCOMPLETEINPUT_H

#include <QLineEdit>
#include "autocompletedropdown.h"
#include "autocompleteview.h"

class AutocompleteInput : public QLineEdit
{
    Q_OBJECT
public:
    explicit AutocompleteInput(QWidget *parent = 0);
    void setData(QVector<AutocompleteView *> list);

signals:

public slots:
    void onTextEdited(const QString &text);

private:
    AutocompleteDropdown *autocompleteDropdown;
    QVector<AutocompleteView *> data;
    QVector<AutocompleteView *> filteredData;

    void filter(const QString &input);
    void reloadDropdown();

};

#endif // AUTOCOMPLETEINPUT_H
