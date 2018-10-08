#include "autocompletedropdownlist.h"
#include <QDebug>

AutocompleteDropdownList::AutocompleteDropdownList(QWidget *parent) :
    QListWidget(parent),
    types(QStringList()),
    loadedOnce(false)
{
    types << "TIME ENTRIES" << "TASKS" << "PROJECTS" << "WORKSPACES";
    connect(this, SIGNAL(itemPressed(QListWidgetItem*)),
            this, SLOT(onListItemClicked(QListWidgetItem*)));
    setUniformItemSizes(true);
}

void AutocompleteDropdownList::onListItemClicked(QListWidgetItem* item)
{
    AutocompleteCellWidget *cl = 0;
    cl = static_cast<AutocompleteCellWidget *>(
            itemWidget(item));
    fillData(cl->view_item);
}

void AutocompleteDropdownList::keyPressEvent(QKeyEvent *e)
{
    bool modifiers = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (modifiers) {
        keyPress(e);
        return;
    }

    switch (e->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        returnPressed();
    case Qt::Key_Escape:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
    case Qt::Key_Down:
    case Qt::Key_Up:
        QListWidget::keyPressEvent(e);
        return;
    }

    keyPress(e);
}

bool AutocompleteDropdownList::filterItems(QString filter) {
    uint64_t lastWID = 0;
    int64_t lastCID = -1;
    int64_t lastPID = -1;
    uint64_t lastType;
    int h = 35;
    int itemCount = 0;
    int size = list.size();
    QString itemText;
    QStringList stringList = filter.split(" ", QString::SkipEmptyParts);
    int matchCount = 0;
    QString currentFilter;
    bool fullList = false;
    bool noProjectAdded = false;

    if (filter.length() == 0){
        stringList << "";
        fullList = true;
    }

    setUpdatesEnabled(false);
    render_m_.lock();

    if (fullList && loadedOnce) {
        for (int i = 0; i < size; i++) {
            QListWidgetItem *it = 0;
            it = item(i);
            it->setHidden(false);
        }
    } else {
        for (int i = 0; i < size; i++) {
            AutocompleteView *view = list.at(i);
            itemText = (view->Type == 1) ? view->ProjectAndTaskLabel.toLower(): view->Text.toLower();
            matchCount = 0;

            for (int j = 0; j < stringList.size(); j++) {
                currentFilter = stringList.at(j).toLower();
                if (currentFilter.length() > 0
                        && itemText.indexOf(currentFilter) == -1) {
                    break;
                }
                matchCount++;
                if (matchCount < stringList.size() && !fullList) {
                    continue;
                }
                // Add workspace title
                if (view->WorkspaceID != lastWID) {

                    QListWidgetItem *it = 0;
                    AutocompleteCellWidget *cl = 0;

                    if (count() > itemCount) {
                        it = item(itemCount);
                        cl = static_cast<AutocompleteCellWidget *>(
                            itemWidget(it));
                    }

                    if (!it) {
                        it = new QListWidgetItem();
                        cl = new AutocompleteCellWidget();

                        addItem(it);
                        setItemWidget(it, cl);
                    }
                    it->setHidden(false);
                    it->setFlags(it->flags() & ~Qt::ItemIsEnabled);

                    AutocompleteView *v = new AutocompleteView();
                    v->Type = 13;
                    v->Text = view->WorkspaceName;
                    cl->display(v);
                    it->setSizeHint(QSize(it->sizeHint().width(), h));

                    lastWID = view->WorkspaceID;
                    lastCID = -1;
                    lastType = 99;

                    itemCount++;
                }

                // Add category title
                if (view->Type != lastType && view->Type != 1) {
                    QListWidgetItem *it = 0;
                    AutocompleteCellWidget *cl = 0;

                    if (count() > itemCount) {
                        it = item(itemCount);
                        cl = static_cast<AutocompleteCellWidget *>(
                            itemWidget(it));
                    }

                    if (!it) {
                        it = new QListWidgetItem();
                        cl = new AutocompleteCellWidget();

                        addItem(it);
                        setItemWidget(it, cl);
                    }
                    it->setHidden(false);
                    it->setFlags(it->flags() & ~Qt::ItemIsEnabled);

                    AutocompleteView *v = new AutocompleteView();
                    v->Type = 11;
                    v->Text = types[view->Type];
                    cl->display(v);
                    it->setSizeHint(QSize(it->sizeHint().width(), h));

                    lastType = view->Type;

                    itemCount++;

                    // Add 'No project' item
                    if (view->Type == 2 && currentFilter.length() == 0
                            && !noProjectAdded)
                    {
                        QListWidgetItem *it = 0;
                        AutocompleteCellWidget *cl = 0;

                        if (count() > itemCount) {
                            it = item(itemCount);
                            cl = static_cast<AutocompleteCellWidget *>(
                                itemWidget(it));
                        }

                        if (!it) {
                            it = new QListWidgetItem();
                            cl = new AutocompleteCellWidget();

                            addItem(it);
                            setItemWidget(it, cl);
                        }
                        it->setHidden(false);
                        it->setFlags(it->flags() | Qt::ItemIsEnabled);

                        AutocompleteView *v = new AutocompleteView();
                        v->Type = 2;
                        v->Text = "No project";
                        v->ProjectAndTaskLabel = "";
                        v->TaskID = 0;
                        v->ProjectID = 0;
                        cl->display(v);
                        it->setSizeHint(QSize(it->sizeHint().width(), h));

                        noProjectAdded = true;
                        itemCount++;
                    }
                }

                // Add Client name
                if (view->Type == 2 && view->ClientID != lastCID)
                {
                    QListWidgetItem *it = 0;
                    AutocompleteCellWidget *cl = 0;

                    if (count() > itemCount) {
                        it = item(itemCount);
                        cl = static_cast<AutocompleteCellWidget *>(
                            itemWidget(it));
                    }

                    if (!it) {
                        it = new QListWidgetItem();
                        cl = new AutocompleteCellWidget();

                        addItem(it);
                        setItemWidget(it, cl);
                    }
                    it->setHidden(false);
                    it->setFlags(it->flags() & ~Qt::ItemIsEnabled);

                    AutocompleteView *v = new AutocompleteView();
                    v->Type = 12;
                    v->Text = view->ClientLabel.count() > 0 ? view->ClientLabel : "No client";
                    cl->display(v);
                    it->setSizeHint(QSize(it->sizeHint().width(), h));
                    lastCID = view->ClientID;

                    itemCount++;
                }

                // In case we filter task and project is not filtered
                if (currentFilter.length() > 0 && view->Type == 1
                        && view->ProjectID != lastPID) {

                    // Also add Client name if needed
                    if (view->ClientID != lastCID) {
                        QListWidgetItem *it = 0;
                        AutocompleteCellWidget *cl = 0;

                        if (count() > itemCount) {
                            it = item(itemCount);
                            cl = static_cast<AutocompleteCellWidget *>(
                                itemWidget(it));
                        }

                        if (!it) {
                            it = new QListWidgetItem();
                            cl = new AutocompleteCellWidget();

                            addItem(it);
                            setItemWidget(it, cl);
                        }
                        it->setHidden(false);
                        it->setFlags(it->flags() & ~Qt::ItemIsEnabled);

                        AutocompleteView *v = new AutocompleteView();
                        v->Type = 12;
                        v->Text = view->ClientLabel.count() > 0 ? view->ClientLabel : "No client";
                        cl->display(v);
                        it->setSizeHint(QSize(it->sizeHint().width(), h));
                        lastCID = view->ClientID;

                        itemCount++;
                    }

                    QListWidgetItem *it = 0;
                    AutocompleteCellWidget *cl = 0;

                    if (count() > itemCount) {
                        it = item(itemCount);
                        cl = static_cast<AutocompleteCellWidget *>(
                            itemWidget(it));
                    }

                    if (!it) {
                        it = new QListWidgetItem();
                        cl = new AutocompleteCellWidget();

                        addItem(it);
                        setItemWidget(it, cl);
                    }
                    it->setHidden(false);
                    it->setFlags(it->flags() | Qt::ItemIsEnabled);

                    AutocompleteView *v = new AutocompleteView();
                    v->Type = 2;
                    v->Text = view->ProjectLabel;
                    v->ProjectLabel = view->ProjectLabel;
                    v->ProjectColor = view->ProjectColor;
                    v->ProjectID = view->ProjectID;
                    v->Description = view->Description;
                    v->ClientLabel = view->ClientLabel;
                    v->ProjectAndTaskLabel = view->ProjectAndTaskLabel;
                    v->TaskID = 0;
                    cl->display(v);
                    it->setSizeHint(QSize(it->sizeHint().width(), h));
                    lastPID = view->ProjectID;

                    itemCount++;
                }


                QListWidgetItem *it = 0;
                AutocompleteCellWidget *cl = 0;

                if (count() > itemCount) {
                    it = item(itemCount);
                    cl = static_cast<AutocompleteCellWidget *>(
                        itemWidget(it));
                }

                if (!it) {
                    it = new QListWidgetItem();
                    cl = new AutocompleteCellWidget();

                    addItem(it);
                    setItemWidget(it, cl);
                }
                it->setHidden(false);
                it->setFlags(it->flags() | Qt::ItemIsEnabled);

                cl->display(view);
                it->setSizeHint(QSize(it->sizeHint().width(), h));

                if (view->Type == 2) {
                    lastPID = view->ProjectID;
                }

                itemCount++;
            }
        }
        int c = count();
        while (c > itemCount) {
            QListWidgetItem *it = 0;
            it = item(c-1);
            it->setHidden(true);
            c--;
        }
    }

    render_m_.unlock();
    setUpdatesEnabled(true);

    if (filter.length() == 0 && !loadedOnce) {
        loadedOnce = true;
    }

    return (itemCount != 0);
}

void AutocompleteDropdownList::setList(QVector<AutocompleteView *> autocompletelist,
                                       QString filter)
{
    list = autocompletelist;
    filterItems(filter);
}

