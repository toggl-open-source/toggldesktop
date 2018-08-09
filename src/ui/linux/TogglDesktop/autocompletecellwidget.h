#ifndef AUTOCOMPLETECELLWIDGET_H
#define AUTOCOMPLETECELLWIDGET_H

#include <QWidget>
#include "./autocompleteview.h"

namespace Ui {
class AutocompleteCellWidget;
}

class AutocompleteCellWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AutocompleteCellWidget(QWidget *parent = 0);
    ~AutocompleteCellWidget();

    void display(AutocompleteView *view);
    void labelClicked(QString field_name);

    AutocompleteView *view_item;
    bool filter(QString filter);

private:
    Ui::AutocompleteCellWidget *ui;
};

#endif // AUTOCOMPLETECELLWIDGET_H
