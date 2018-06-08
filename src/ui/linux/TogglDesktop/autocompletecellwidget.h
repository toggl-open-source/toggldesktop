#ifndef AUTOCOMPLETECELLWIDGET_H
#define AUTOCOMPLETECELLWIDGET_H

#include <QWidget>
#include <QLabel>
#include "autocompleteview.h"

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

private:
    Ui::AutocompleteCellWidget *ui;
    void setEllipsisTextToLabel(QLabel *label, QString text);


 protected:
    //virtual void mousePressEvent(QMouseEvent *event);
};

#endif // AUTOCOMPLETECELLWIDGET_H
