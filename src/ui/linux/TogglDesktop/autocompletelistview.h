#ifndef AUTOCOMPLETELISTVIEW_H
#define AUTOCOMPLETELISTVIEW_H

#include <QListView>
#include <QListWidget>
#include <QKeyEvent>
#include <QMutex>
#include <QItemDelegate>

#include "./autocompleteview.h"

class AutocompleteListView : public QListView
{
    Q_OBJECT
    friend class AutocompleteComboBox;
public:
    explicit AutocompleteListView(QWidget *parent = 0);

    void paintEvent(QPaintEvent *e) override;

    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

signals:
    void visibleChanged();
    void selected(AutocompleteView *item);

protected:
    void keyPressEvent(QKeyEvent *e);

};

class AutoCompleteItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    AutoCompleteItemDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QString format(const AutocompleteView *view) const;
    AutocompleteView *getCurrentView(const QModelIndex &index) const;
};

#endif // AUTOCOMPLETELISTVIEW_H
