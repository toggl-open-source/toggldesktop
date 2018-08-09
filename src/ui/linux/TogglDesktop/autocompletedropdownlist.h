#ifndef AUTOCOMPLETEDROPDOWN_H
#define AUTOCOMPLETEDROPDOWN_H

#include <QListWidget>
#include <QKeyEvent>

#include "./autocompletecellwidget.h"

class AutocompleteDropdownList : public QListWidget
{
    Q_OBJECT
public:
    explicit AutocompleteDropdownList(QWidget *parent = 0);
    void filterItems(QString filter);
    void setList(QVector<AutocompleteView *> autocompletelist, QString filter);

private:
    QVector<AutocompleteView *> list;
    QStringList types;

signals:
    void keyPress(QKeyEvent *e);

public slots:

protected:
    void keyPressEvent(QKeyEvent *e);

};

#endif // AUTOCOMPLETEDROPDOWN_H
