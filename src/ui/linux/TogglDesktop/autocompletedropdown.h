#ifndef AUTOCOMPLETEDROPDOWN_H
#define AUTOCOMPLETEDROPDOWN_H

#include <QDialog>
#include <QGraphicsDropShadowEffect>
#include "autocompleteview.h"
#include "autocompletecellwidget.h"

namespace Ui {
class AutocompleteDropdown;
}

class AutocompleteDropdown : public QDialog
{
    Q_OBJECT

public:
    explicit AutocompleteDropdown(QWidget *parent = 0);
    ~AutocompleteDropdown();
    void setData(QVector<AutocompleteView *> list);
    void reload(QVector<AutocompleteView *> list);
    void onKeyPressEvent(QKeyEvent* event);


private:
    Ui::AutocompleteDropdown *ui;
};

#endif // AUTOCOMPLETEDROPDOWN_H
