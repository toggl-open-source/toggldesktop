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

    AutocompleteView *view_item;

private:
    Ui::AutocompleteCellWidget *ui;
};

#endif // AUTOCOMPLETECELLWIDGET_H
