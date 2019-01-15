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
            [[clang::fallthrough]];
        case Qt::Key_Enter:
        case Qt::Key_Return:
            listView->setVisible(false);
            return true;
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

void AutocompleteComboBox::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
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

void AutocompleteComboBox::onModelChanged() {
}

void AutocompleteComboBox::onDropdownVisibleChanged() {
    if (listView->isVisible()) {
        oldLabel = currentText();
    }
}

AutocompleteCompleter::AutocompleteCompleter(QWidget *parent)
    : QCompleter(parent)
{
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
        if (view->Description.contains(word, Qt::CaseInsensitive))
                return true;
        if (view->Text.contains(word, Qt::CaseInsensitive))
                return true;
    }

    return false;
}
