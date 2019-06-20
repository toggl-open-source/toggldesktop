#include "autocompletelistview.h"

#include <QDebug>
#include <QPainter>
#include <QTextLayout>

AutocompleteListView::AutocompleteListView(QWidget *parent) :
    QListView(parent)
{
    setFixedWidth(320);
    setViewMode(QListView::ListMode);
    setUniformItemSizes(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setItemDelegateForColumn(0, new AutoCompleteItemDelegate(this));

    connect(this, &QListView::activated, [=](const QModelIndex &index){
        emit selected(qvariant_cast<AutocompleteView*>(index.data(Qt::UserRole)));
        setVisible(false);
    });
    connect(this, &QListView::clicked, [=](const QModelIndex &index){
        emit selected(qvariant_cast<AutocompleteView*>(index.data(Qt::UserRole)));
    });
}

void AutocompleteListView::paintEvent(QPaintEvent *e) {
    QListView::paintEvent(e);
}

QModelIndex AutocompleteListView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) {
    Q_UNUSED(modifiers);
    switch(cursorAction) {
        case MoveDown:
        case MoveRight:
        case MoveNext:
            for (int i = currentIndex().row() + 1; i < model()->rowCount(); i++) {
                auto index = model()->index(i, 0);
                if (index.isValid() && index.flags() & Qt::ItemIsEnabled) {
                    return model()->index(i, 0);
                }
            }
            return QModelIndex();
        case MovePageDown: {
            QModelIndex lastValid;
            int remaining = 19;
            for (int i = currentIndex().row() + 1; i < model()->rowCount() && remaining > 0; i++, remaining--) {
                auto index = model()->index(i, 0);
                if (index.isValid() && index.flags() & Qt::ItemIsEnabled) {
                    lastValid = model()->index(i, 0);
                }
            }
            return lastValid;
        }
        case MoveEnd: {
            for (int i = model()->rowCount() - 1; i >= 0; i--) {
                auto index = model()->index(i, 0);
                if (index.isValid() && index.flags() & Qt::ItemIsEnabled) {
                    return index;
                }
            }
            return QModelIndex();
        }
        case MoveUp:
        case MoveLeft:
        case MovePrevious: {
            for (int i = currentIndex().row() - 1; i >= 0; i--) {
                auto index = model()->index(i, 0);
                if (index.isValid() && index.flags() & Qt::ItemIsEnabled) {
                    return model()->index(i, 0);
                }
            }
            return QModelIndex();
        }
        case MovePageUp: {
            QModelIndex lastValid;
            int remaining = 19;
            for (int i = currentIndex().row() - 1; i >= 0 && remaining > 0; i--, remaining--) {
                auto index = model()->index(i, 0);
                if (index.isValid() && index.flags() & Qt::ItemIsEnabled) {
                    lastValid = model()->index(i, 0);
                }
            }
            return lastValid;
        }
        case MoveHome: {
            for (int i = 0; i < model()->rowCount(); i++) {
                auto index = model()->index(i, 0);
                if (index.isValid() && index.flags() & Qt::ItemIsEnabled) {
                    return index;
                }
            }
            return QModelIndex();
        }
    }
    return QModelIndex();
}

void AutocompleteListView::showEvent(QShowEvent *event) {
    emit visibleChanged();
    QListView::showEvent(event);
}

void AutocompleteListView::hideEvent(QHideEvent *event) {
    emit visibleChanged();
    QListView::hideEvent(event);
}

void AutocompleteListView::keyPressEvent(QKeyEvent *e)
{
    bool modifiers = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (modifiers) {
        //emit keyPress(e);
        return;
    }

    QListView::keyPressEvent(e);
    //emit keyPress(e);
}

AutoCompleteItemDelegate::AutoCompleteItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{

}

void AutoCompleteItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();

    auto view = getCurrentView(index);
    /*
     * Because QTextDocument doesn't support <table> very well, we need to paint time entries ourselves by hand
     */
    switch (view->Type) {
    case 0:
        layoutTimeEntry(painter, option, view);
        break;
    case 1:
        layoutTask(painter, option, view);
        break;
    case 2:
        layoutProject(painter, option, view);
        break;
    default: {
        QTextDocument doc;
        doc.setTextWidth(option.rect.width());
        doc.setHtml(format(view));

        option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

        painter->translate(option.rect.left(), option.rect.top());
        QRect clip(0, 0, option.rect.width(), option.rect.height());
        doc.drawContents(painter, clip);
    }}

    painter->restore();
}

void AutoCompleteItemDelegate::layoutProject(QPainter *painter, const QStyleOptionViewItem &option, AutocompleteView *view) const {
    // set up the basic stuff, fonts, colors and font metrics to measure text sizes
    int lineWidth = option.rect.width();
    QFont font;
    font.setPixelSize(12);
    QFontMetrics metrics(font);
    QPen colorPen;
    if (!view->ProjectColor.empty())
        colorPen.setColor(QColor(QString::fromStdString(view->ProjectColor)));

    // center the string vertically
    int topMargin = (24 - metrics.height()) / 2;
    // some margins to please the eye
    int leftMargin = 18;
    int rightMargin = 12;

    // then lay out the project
    QTextLayout projectLayout;
    int projectWidth = lineWidth - leftMargin - rightMargin - 1;
    QString elidedProject = metrics.elidedText(QString::fromStdString(view->ProjectLabel), option.textElideMode, projectWidth);
    projectLayout.setText(" • " + elidedProject);
    projectLayout.setFont(font);
    projectWidth = metrics.width(" • " + elidedProject) + 1;

    projectLayout.beginLayout();
    auto projectLine = projectLayout.createLine();
    projectLine.setLineWidth(projectWidth + 1);
    projectLine.setPosition({qreal(leftMargin), qreal(topMargin)});
    projectLayout.endLayout();

    // and this is the actual painting part
    // this asks Qt to paint the item background (for the highlighted items, especially)
    option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);
    // set the pen to match the project color and render project name
    painter->setPen(colorPen);
    projectLayout.draw(painter, QPointF(option.rect.left(), option.rect.top()));
}

void AutoCompleteItemDelegate::layoutTask(QPainter *painter, const QStyleOptionViewItem &option, AutocompleteView *view) const {
    // set up the basic stuff, fonts, colors and font metrics to measure text sizes
    int lineWidth = option.rect.width();
    QFont font;
    font.setPixelSize(12);
    QFontMetrics metrics(font);
    QPen grayPen;
    grayPen.setColor(Qt::darkGray);

    // center the string vertically
    int topMargin = (24 - metrics.height()) / 2;
    // some margins to please the eye
    int leftMargin = 30;
    int rightMargin = 12;

    // do line trimming
    QTextLayout taskLayout;
    int taskWidth = lineWidth - leftMargin - rightMargin - 1;
    QString elidedTask = metrics.elidedText(QString::fromStdString(view->TaskLabel), option.textElideMode, taskWidth);
    taskLayout.setText(elidedTask);
    taskLayout.setFont(font);
    taskWidth = metrics.width(elidedTask) + 1;

    // here we do the actual layout of the line
    taskLayout.beginLayout();
    auto taskLine = taskLayout.createLine();
    taskLine.setLineWidth(taskWidth + 1);
    taskLine.setPosition({qreal(leftMargin), qreal(topMargin)});
    taskLayout.endLayout();

    // and this is the actual painting part
    // this asks Qt to paint the item background (for the highlighted items, especially)
    option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);
    // set the pen to gray
    painter->setPen(grayPen);
    // render the task label
    taskLayout.draw(painter, QPointF(option.rect.left(), option.rect.top()));
}

void AutoCompleteItemDelegate::layoutTimeEntry(QPainter *painter, const QStyleOptionViewItem &option, AutocompleteView *view) const {
    // set up the basic stuff, fonts, colors and font metrics to measure text sizes
    int lineWidth = option.rect.width();
    QFont font;
    font.setPixelSize(12);
    QFontMetrics metrics(font);
    QPen defaultPen(painter->pen());
    QPen colorPen(defaultPen);
    if (!view->ProjectColor.empty())
        colorPen.setColor(QColor(QString::fromStdString(view->ProjectColor)));
    QPen grayPen(defaultPen);
    grayPen.setColor(Qt::darkGray);

    // center the string vertically
    int topMargin = (24 - metrics.height()) / 2;
    // some margins to please the eye
    int leftMargin = 18;
    int rightMargin = 6;

    // first lay out the client (rightmost string)
    QTextLayout clientLayout;
    int clientWidth = 0;
    if (!view->ClientLabel.empty()) {
        QString elidedClient = metrics.elidedText(QString::fromStdString(view->ClientLabel), option.textElideMode, lineWidth / 3);
        clientLayout.setText(elidedClient);
        clientLayout.setFont(font);
        clientWidth = metrics.width(elidedClient) + 1;
    }

    // then lay out the project
    QTextLayout projectLayout;
    int projectWidth = 0;
    if (!view->ProjectLabel.empty()) {
        QString elidedProject = metrics.elidedText(QString::fromStdString(view->ProjectLabel), option.textElideMode, lineWidth / 2);
        projectLayout.setText(" • " + elidedProject);
        projectLayout.setFont(font);
        projectWidth = metrics.width(" • " + elidedProject) + 1;
    }

    // and finally the description, since it can be cropped by the project and client
    QTextLayout descriptionLayout;
    // crop the potential length by the margins and client/project widths
    int descriptionWidth = lineWidth - projectWidth - clientWidth - leftMargin - rightMargin + 1;
    // metrics will do the ellipsis for us
    QString elidedDescription = metrics.elidedText(QString::fromStdString(view->Description), option.textElideMode, descriptionWidth);
    // and now we measure the actual length of the ellided string so everything aligns nicely
    descriptionWidth = metrics.width(elidedDescription);
    descriptionLayout.setText(elidedDescription);
    descriptionLayout.setFont(font);

    // here we do the actual layout of the line
    descriptionLayout.beginLayout();
    auto descriptionLine = descriptionLayout.createLine();
    descriptionLine.setLineWidth(descriptionWidth + 1);
    descriptionLine.setPosition({qreal(leftMargin), qreal(topMargin)});
    descriptionLayout.endLayout();

    projectLayout.beginLayout();
    auto projectLine = projectLayout.createLine();
    projectLine.setLineWidth(projectWidth + 1);
    projectLine.setPosition({qreal(leftMargin + descriptionWidth + 1), qreal(topMargin)});
    projectLayout.endLayout();

    clientLayout.beginLayout();
    auto clientLine = clientLayout.createLine();
    clientLine.setLineWidth(clientWidth + 1);
    clientLine.setPosition({qreal(leftMargin + descriptionWidth + projectWidth + 2), qreal(topMargin)});
    clientLayout.endLayout();

    // and this is the actual painting part
    // this asks Qt to paint the item background (for the highlighted items, especially)
    option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);
    // paint the description with the default pen first
    descriptionLayout.draw(painter, QPointF(option.rect.left(), option.rect.top()));
    // set the pen to match the project color and render project name
    painter->setPen(colorPen);
    projectLayout.draw(painter, QPointF(option.rect.left(), option.rect.top()));
    // and finally, paint the client in gray
    painter->setPen(grayPen);
    clientLayout.draw(painter, QPointF(option.rect.left(), option.rect.top()));
}

QSize AutoCompleteItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return { 320, 24 };
}

QString AutoCompleteItemDelegate::format(const AutocompleteView *view) const {
    QString label;
    QString transparent = "background-color: transparent;";

    // Format is: Description - TaskName · ProjectName - ClientName
    QString description = QString::fromStdString(view->Description);
    QString clientLabel = QString::fromStdString(view->ClientLabel);
    QString taskLabel = QString::fromStdString(view->TaskLabel);
    QString projectLabel = QString::fromStdString(view->ProjectLabel);
    QString projectColor = QString::fromStdString(view->ProjectColor);

    switch (view->Type) {
    case 13: // Workspace row
        return "<div width=320px style='width:320px;font-size:13px;color:#9e9e9e;text-align:center;background-color:transparent;font-weight:bold;margin:2px;padding:2px;'>" + description + "</div><div width=320px height=1px style='background-color:#ececec'/>";
    case 11: // Category row
        return "<div width=320px style='width:320px;font-size:12px;color:#9e9e9e;background-color:transparent;font-weight:500'>" + description + "</div>";
    case 12: { // Client row / no project row
        return "<div width=320px style='width:320px;font-size:12px;color:#9e9e9e;background-color:transparent;font-weight:500;margin-left:9px;'>" + description + "</div>";
    }
    case 2: { // Project items rows
        label.append("<div style='font-size:12px;margin-left:18px;color:" + projectColor + ";'>• " + projectLabel + "</div>");
        return label;
    }
    case 1: { // Task row
        QString row = QString("<div style='font-size:12px;margin-left:27px;background-color:transparent;color:gray'>");
        row += taskLabel;
        row += "</div>";
        return row;
    }
    case 0: { // Item rows (projects/time entries)
        // !!! UNUSED
        QString table("<div style='margin-left:9px;font-size:12px;'>");
        if (!description.isEmpty())
            table.append(description + " ");
        if (view->TaskID)
            table.append(taskLabel + " ");
        //table.append("<br>");
        if (view->ProjectID)
            table.append("<span style='color:" + projectColor + ";'> • " + projectLabel + " </span>");
        if (view->ClientID)
            table.append("<span style='color:#9e9e9e;'> " + clientLabel + "</span>");
        table.append("</div>");
        return table;
    }
    default:
        //ui->label->setStyleSheet(transparent + "padding-top:7px;padding-left:15px;font-size:9pt;");
        return "<span style='background-color: transparent;font-size:13px;'>" + projectLabel + taskLabel + clientLabel + description + "</span>";
    }
}

AutocompleteView *AutoCompleteItemDelegate::getCurrentView(const QModelIndex &index) const {
    return qvariant_cast<AutocompleteView*>(index.data(Qt::UserRole));
}
