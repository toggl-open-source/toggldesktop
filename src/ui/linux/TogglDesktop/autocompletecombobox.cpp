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
    lineEdit->installEventFilter(this);
    completer->installEventFilter(this);
    listView->installEventFilter(this);
    completer->setModel(proxyModel);
    setLineEdit(lineEdit);
    setCompleter(completer);
    connect(lineEdit, &QLineEdit::textEdited, proxyModel, &AutocompleteProxyModel::setFilterFixedString);
}

void AutocompleteComboBox::setModel(QAbstractItemModel *model) {
    proxyModel->setSourceModel(model);
}

bool AutocompleteComboBox::eventFilter(QObject *o, QEvent *e) {
    if (e->type() == QEvent::KeyPress) {
        auto ke = reinterpret_cast<QKeyEvent*>(e);
        switch (ke->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
            if (!listView->isVisible())
                completer->complete();
            else
                listView->keyPressEvent(ke);
            return true;
        default:
            return QComboBox::eventFilter(o, e);
        }
    }
    return QComboBox::eventFilter(o, e);
}

void AutocompleteComboBox::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
        QComboBox::keyPressEvent(event);
        break;
    case Qt::Key_Down:
    case Qt::Key_Up:
        if (!listView->isVisible())
            completer->complete();
        else
            QComboBox::keyPressEvent(event);
        break;
    default:
        QComboBox::keyPressEvent(event);
    }
}

void AutocompleteComboBox::onModelChanged() {
}

AutocompleteCompleter::AutocompleteCompleter(QWidget *parent)
    : QCompleter(parent)
{
    setPopup(new AutocompleteListView(parent));
    setCompletionMode(QCompleter::UnfilteredPopupCompletion);
}

AutocompleteLineEdit::AutocompleteLineEdit(AutocompleteComboBox *parent)
    : QLineEdit(parent)
{

}

AutocompleteComboBox *AutocompleteLineEdit::comboBox() {
    return qobject_cast<AutocompleteComboBox*>(parent());
}

void AutocompleteLineEdit::keyPressEvent(QKeyEvent *event) {
    qCritical() << "AHOY";
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape: {
        // remembering the edit string is a HACK, this should be done in a different way
        // not sure how to do it atm
        /*
        QString previous = text();
        QLineEdit::keyPressEvent(event);
        if (text().isEmpty() && !previous.isEmpty()) {
            setText(previous);
        }
        break;
        */
    }
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
