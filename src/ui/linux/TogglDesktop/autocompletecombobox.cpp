#include "autocompletecombobox.h"
#include "autocompletelistmodel.h"
#include "autocompletelistview.h"

#include <QDebug>

AutocompleteComboBox::AutocompleteComboBox(QWidget *parent)
    : QComboBox(parent)
    , lineEdit(new AutocompleteLineEdit(this))
    , completer(new AutocompleteCompleter(this))
    , proxyModel(new AutocompleteProxyModel(this))
    , listView(qobject_cast<AutocompleteListView*>(completer->popup()))
{
    setEditable(true);
    completer->installEventFilter(this);
    listView->installEventFilter(this);
    completer->setModel(proxyModel);
    setLineEdit(lineEdit);
    setCompleter(completer);
    connect(listView, &AutocompleteListView::visibleChanged, this, &AutocompleteComboBox::onDropdownVisibleChanged);
    connect(lineEdit, &QLineEdit::textEdited, proxyModel, &AutocompleteProxyModel::setFilterFixedString);
    connect(listView, &AutocompleteListView::selected, this, &AutocompleteComboBox::onDropdownSelected);
}

void AutocompleteComboBox::setModel(QAbstractItemModel *model) {
    proxyModel->setSourceModel(model);
}

bool AutocompleteComboBox::eventFilter(QObject *o, QEvent *e) {
    if (e->type() == QEvent::KeyPress) {
        auto ke = reinterpret_cast<QKeyEvent*>(e);
        switch (ke->key()) {
        case Qt::Key_Escape:
            setCurrentText(oldLabel);
            oldLabel = QString();
            listView->setVisible(false);
            return true;
        case Qt::Key_Enter:
        case Qt::Key_Return: {
            if (listView->currentIndex().isValid()) {
                listView->keyPressEvent(ke);
            }
            else {
                listView->hide();
                emit returnPressed();
            }
            return true;
        }
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageDown:
        case Qt::Key_PageUp:
        case Qt::Key_Up:
        case Qt::Key_Down:
            if (!listView->isVisible())
                completer->complete();
            else
                listView->keyPressEvent(ke);
            return true;
        default:
            lineEdit->keyPressEvent(ke);
            return true;
        }
    }
    return QComboBox::eventFilter(o, e);
}

AutocompleteView *AutocompleteComboBox::currentView() {
    return qvariant_cast<AutocompleteView*>(currentData());
}

void AutocompleteComboBox::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        emit returnPressed();
        break;
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
    case Qt::Key_Down:
    case Qt::Key_Up:
        if (!listView->isVisible())
            completer->complete();
        else
            listView->keyPressEvent(event);
        break;
    default:
        QComboBox::keyPressEvent(event);
    }
}

void AutocompleteComboBox::onDropdownVisibleChanged() {
    if (listView->isVisible()) {
        oldLabel = currentText();
    }
}

void AutocompleteComboBox::onDropdownSelected(AutocompleteView *item) {
    if (item) {

        switch (item->Type) {
        case 0:
            if (item->ProjectID && !item->ProjectLabel.isEmpty())
                emit projectSelected(item->ProjectLabel, item->ProjectID, item->ProjectColor);
            emit timeEntrySelected(item->Text);
            break;
        case 2:
            emit projectSelected(item->Text, item->ProjectID, item->ProjectColor);
            break;
        default:
            break;
        }
    }
}

AutocompleteCompleter::AutocompleteCompleter(QWidget *parent)
    : QCompleter(parent)
{
    setMaxVisibleItems(20);
    setPopup(new AutocompleteListView(parent));
    setCompletionMode(QCompleter::UnfilteredPopupCompletion);
}

bool AutocompleteCompleter::eventFilter(QObject *o, QEvent *e) {
    // completely ignore key events in the filter and let the combobox handle them
    if (e->type() == QEvent::KeyPress) {
        return false;
    }
    return QCompleter::eventFilter(o, e);
}

AutocompleteLineEdit::AutocompleteLineEdit(AutocompleteComboBox *parent)
    : QLineEdit(parent)
{

}

AutocompleteComboBox *AutocompleteLineEdit::comboBox() {
    return qobject_cast<AutocompleteComboBox*>(parent());
}

void AutocompleteLineEdit::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    default:
        QLineEdit::keyPressEvent(event);
    }
}

AutocompleteProxyModel::AutocompleteProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent) {
    setFilterRole(Qt::UserRole);
}

bool AutocompleteProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    QString input = filterRegExp().pattern();
    QStringList words = input.split(" ");

    auto view = qvariant_cast<AutocompleteView*>(sourceModel()->data(sourceModel()->index(source_row, 0), Qt::UserRole));
    for (auto word : words) {
        if (word.isEmpty() && words.count() > 1)
            continue;
        if (view->_Children.isEmpty()) {
            if (view->Description.contains(word, Qt::CaseInsensitive))
                    return true;
            if (view->Text.contains(word, Qt::CaseInsensitive))
                    return true;
        }
        for (auto v : view->_Children) {
            if (v->Description.contains(word, Qt::CaseInsensitive))
                    return true;
            if (v->Text.contains(word, Qt::CaseInsensitive))
                    return true;
        }
    }

    return false;
}
