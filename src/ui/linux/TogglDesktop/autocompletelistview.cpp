#include "autocompletelistview.h"

#include <QDebug>
#include <QPainter>
#include <QTextDocument>

AutocompleteListView::AutocompleteListView(QWidget *parent) :
    QListView(parent)
{
    setFixedWidth(320);
    setViewMode(QListView::ListMode);
    setUniformItemSizes(true);
    setItemDelegate(new AutoCompleteItemDelegate(this));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void AutocompleteListView::paintEvent(QPaintEvent *e) {
    QListView::paintEvent(e);
}

void AutocompleteListView::keyPressEvent(QKeyEvent *e)
{
    bool modifiers = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (modifiers) {
        emit keyPress(e);
        return;
    }

    switch (e->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
        QListView::keyPressEvent(e);
        return;
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
    case Qt::Key_Down:
    case Qt::Key_Up:
        QListView::keyPressEvent(e);
        return;
    }

    emit keyPress(e);
}

AutoCompleteItemDelegate::AutoCompleteItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{

}

void AutoCompleteItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();

    QTextDocument doc;
    doc.setTextWidth(319);
    auto view = getCurrentView(index);
    doc.setHtml(format(view));

    option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

    painter->translate(option.rect.left(), option.rect.top());
    QRect clip(0, 0, option.rect.width(), option.rect.height());
    doc.drawContents(painter, clip);

    painter->restore();
}

QSize AutoCompleteItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    auto view = getCurrentView(index);
    QSize size = QItemDelegate::sizeHint(option, index);
    size.setHeight(24);
    size.setWidth(320);
    return size;
}

QString AutoCompleteItemDelegate::format(const AutocompleteView *view) const {
    QString label;
    QString transparent = "background-color: transparent;";

    // Format is: Description - TaskName · ProjectName - ClientName

    switch (view->Type) {
    case 13: // Workspace row
        return "<div width=320px style='width:320px;font-size:13px;color:#9e9e9e;text-align:center;background-color:transparent;font-weight:bold;margin:2px;padding:2px;'>" + view->Description + "</div><div width=320px height=1px style='background-color:#ececec'/>";
    case 11: // Category row
        return "<div width=320px style='width:320px;font-size:12px;color:#9e9e9e;background-color:transparent;font-weight:500'>" + view->Description + "</div>";
    case 12: { // Client row / no project row
        QString style = transparent + "padding-top:5px;padding-left:10px;font-weight:";
        if (view->Type == 2) {
            style.append("normal;");
        } else {
            style.append("800;");
        }
        //ui->label->setStyleSheet(style);
        label = "<span style='" + style + "'>" + view->Description + "</span>";
        return label;
    }
    case 1: // Task row
        return "<span style='background-color:transparent;padding-top:8px;padding-left:30px;'>" "- " + view->Description + "</span>";
    case 0: { // Item rows (projects/time entries)
        QString table;
        if (!view->Description.isEmpty())
            table.append("<span style='font-size:14px;'>" + view->Description + "</span>");
        if (view->TaskID)
            table.append("<span style='font-size:10px;'> - " + view->TaskLabel + "</span>");
        //table.append("<br>");
        if (view->ProjectID)
            table.append("<span style='font-size:12px;color:" + view->ProjectColor + ";'> •" + view->ProjectLabel + "</span>");
        if (view->ClientID)
            table.append("<span style='font-size:8px;top-padding:1px;bottom-padding:1px'>" + view->ClientLabel + "</span>");
        return table;
    }
    default:
        //ui->label->setStyleSheet(transparent + "padding-top:7px;padding-left:15px;font-size:9pt;");
        return "<span style='background-color: transparent;font-size:13px;'>" + view->ProjectLabel + view->TaskLabel + view->ClientLabel + view->Description + "</span>";
    }
}

AutocompleteView *AutoCompleteItemDelegate::getCurrentView(const QModelIndex &index) const {
    return qvariant_cast<AutocompleteView*>(index.data(Qt::UserRole));
}
