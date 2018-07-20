// Copyright 2014 Toggl Desktop developers.

#include "./timerwidget.h"
#include "./ui_timerwidget.h"

#include <QApplication>  // NOLINT
#include <QCompleter>  // NOLINT

#include "./autocompleteview.h"
#include "./timeentryview.h"
#include "./toggl.h"
#include "./autocompletecellwidget.h"

TimerWidget::TimerWidget(QWidget *parent) : QWidget(parent),
ui(new Ui::TimerWidget),
timer(new QTimer(this)),
duration(0),
timeEntryAutocompleteNeedsUpdate(false),
tagsHolder(""),
project(""),
dropdown(new AutocompleteDropdown(this)),
types(QStringList()) {
    ui->setupUi(this);

    connect(TogglApi::instance, SIGNAL(displayStoppedTimerState()),
            this, SLOT(displayStoppedTimerState()));

    connect(TogglApi::instance, SIGNAL(displayRunningTimerState(TimeEntryView*)),  // NOLINT
            this, SLOT(displayRunningTimerState(TimeEntryView*)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayMinitimerAutocomplete(QVector<AutocompleteView*>)),  // NOLINT
            this, SLOT(displayMinitimerAutocomplete(QVector<AutocompleteView*>)));  // NOLINT

    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)),
            this, SLOT(focusChanged(QWidget*, QWidget*)));

    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));

    connect(ui->description->lineEdit(), SIGNAL(returnPressed()),
            this, SLOT(descriptionReturnPressed()));

    ui->description->setModel(dropdown->model());
    ui->description->setView(dropdown);

    ui->description->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    ui->description->completer()->setCompletionMode(
        QCompleter::PopupCompletion);
    ui->description->completer()->setMaxVisibleItems(20);
    ui->description->completer()->setFilterMode(Qt::MatchContains);

    ui->billable->setVisible(false);
    ui->tags->setVisible(false);

    descriptionPlaceholder = "What are you doing?";
    tagsHolder = "";
    types << "TIME ENTRIES" << "TASKS" << "PROJECTS" << "WORKSPACES";
}

TimerWidget::~TimerWidget() {
    timer->stop();

    delete ui;
}

void TimerWidget::descriptionReturnPressed() {
    start();
}

void TimerWidget::focusChanged(QWidget *old, QWidget *now) {
    if (old == ui->description) {
        if (ui->description->currentText().length() == 0) {
            ui->description->setEditText(descriptionPlaceholder);
        }
        if (timeEntryAutocompleteNeedsUpdate) {
            displayMinitimerAutocomplete(timeEntryAutocompleteUpdate);
        }
    }
    if (now == ui->description &&
            ui->description->currentText() == descriptionPlaceholder) {
        ui->description->setEditText("");
    }
}

void TimerWidget::displayRunningTimerState(
    TimeEntryView *te) {
    ui->start->setText("Stop");
    ui->start->setStyleSheet(
        "background-color: #e20000; color:'white'; font-weight: bold;");

    QString description = (te->Description.length() > 0) ?
                          te->Description : "(no description)";

    ui->description->setEditText(description);
    ui->description->setEnabled(false);

    ui->duration->setText(te->Duration);
    ui->duration->setEnabled(false);
    ui->duration->setToolTip(
        QString("<p style='color:white;background-color:black;'>Started: " +
                te->StartTimeString+"</p>"));

    project = te->ProjectAndTaskLabel;
    setEllipsisTextToLabel(ui->project, project);

    ui->billable->setVisible(te->Billable);
    ui->tags->setVisible(!te->Tags.isEmpty());

    duration = te->DurationInSeconds;

    if (te->Description.length() > 0) {
        ui->description->setToolTip(
            QString("<p style='color:white;background-color:black;'>" +
                    te->Description+"</p>"));
    }
    if (te->ProjectAndTaskLabel.length() > 0) {
        ui->project->setToolTip(
            QString("<p style='color:white;background-color:black;'>" +
                    te->ProjectAndTaskLabel+"</p>"));
    }

    disconnect(this, SLOT(start()));
    disconnect(this, SLOT(stop()));

    connect(this, SIGNAL(buttonClicked()), this, SLOT(stop()));

    timer->start(1000);
}

void TimerWidget::displayStoppedTimerState() {
    ui->start->setText("Start");
    ui->start->setStyleSheet(
        "background-color: #47bc00; color:'white'; font-weight: bold;");

    if (!ui->description->hasFocus()) {
        ui->description->setEditText(descriptionPlaceholder);
    }
    ui->description->setEnabled(true);
    ui->description->setToolTip(QString(""));

    if (!ui->duration->hasFocus()) {
        ui->duration->setText("");
    }
    ui->duration->setEnabled(true);

    ui->project->setText("");

    ui->billable->setVisible(false);
    ui->tags->setVisible(false);

    tagsHolder = "";

    duration = 0;

    disconnect(this, SLOT(start()));
    disconnect(this, SLOT(stop()));

    connect(this, SIGNAL(buttonClicked()), this, SLOT(start()));

    timer->stop();
}

void TimerWidget::on_start_clicked() {
    emit buttonClicked();
}

void TimerWidget::start() {
    uint64_t task_id(0);
    uint64_t project_id(0);

    QVariant data = ui->description->currentData();
    if (data.canConvert<AutocompleteView *>()) {
        AutocompleteView *view = data.value<AutocompleteView *>();
        task_id = view->TaskID;
        project_id = view->ProjectID;
    }

    QString description = ui->description->currentText();
    if (description == descriptionPlaceholder) {
        description = "";
    }

    TogglApi::instance->start(description,
                              ui->duration->text(),
                              task_id,
                              project_id,
                              tagsHolder.toStdString().c_str(),
                              ui->billable->isVisible());
    tagsHolder = "";
}

void TimerWidget::stop() {
    TogglApi::instance->stop();
}

void TimerWidget::displayMinitimerAutocomplete(
    QVector<AutocompleteView *> list) {

    uint64_t lastWID = 0;
    uint64_t lastType;
    QString lastClientLabel;

    timeEntryAutocompleteUpdate = list;
    timeEntryAutocompleteNeedsUpdate = true;
    if (ui->description->hasFocus()) {
        return;
    }
    /*
    QString currentText = ui->description->currentText();
    ui->description->clear();
    ui->description->addItem("");
    foreach(AutocompleteView *view, timeEntryAutocompleteUpdate) {
        ui->description->addItem(view->Text, QVariant::fromValue(view));
    }
    timeEntryAutocompleteNeedsUpdate = false;
    ui->description->setEditText(currentText);
    */
    int size = list.size();
    for (int i = 0; i < size; i++) {
        AutocompleteView *view = list.at(i);
        /*
        QListWidgetItem *item = new QListWidgetItem(lw);
        QLabel *ql = new QLabel(view->Text);
        lw->setItemWidget(item, ql);
        item->setSizeHint(QSize(item->sizeHint().width(), 100));
*/
        /*
        QListWidgetItem *item = 0;
        AutocompleteCellWidget *cell = 0;

        if (dropdown->count() > i) {
            item = dropdown->item(i);
            cell = static_cast<AutocompleteCellWidget *>(
                dropdown->itemWidget(item));
        }

        if (!item) {
            item = new QListWidgetItem(dropdown);
            cell = new AutocompleteCellWidget();

            dropdown->addItem(item);
            dropdown->setItemWidget(item, cell);
        }
*/

        // Add workspace title
        if (view->WorkspaceID != lastWID) {
            QListWidgetItem *it = new QListWidgetItem(dropdown);
            AutocompleteCellWidget *cl = new AutocompleteCellWidget();
            it->setFlags(it->flags() & ~Qt::ItemIsSelectable);
            dropdown->addItem(it);
            dropdown->setItemWidget(it, cl);

            AutocompleteView *v = new AutocompleteView();
            v->Type = 13;
            v->Text = view->WorkspaceName;
            cl->display(v);
            it->setSizeHint(QSize(it->sizeHint().width(), 50));

            lastWID = view->WorkspaceID;
            lastClientLabel = "";
            lastType = 99;
        }

        // Add category title
        if (view->Type != lastType && view->Type != 1) {
            QListWidgetItem *it = new QListWidgetItem(dropdown);
            AutocompleteCellWidget *cl = new AutocompleteCellWidget();
            it->setFlags(it->flags() & ~Qt::ItemIsSelectable);
            dropdown->addItem(it);
            dropdown->setItemWidget(it, cl);

            AutocompleteView *v = new AutocompleteView();
            v->Type = 11;
            v->Text = types[view->Type];
            cl->display(v);
            it->setSizeHint(QSize(it->sizeHint().width(), 50));

            lastType = view->Type;

            // Add 'No project' item
            if (view->Type == 2)
            {
                QListWidgetItem *it = new QListWidgetItem(dropdown);
                AutocompleteCellWidget *cl = new AutocompleteCellWidget();
                dropdown->addItem(it);
                dropdown->setItemWidget(it, cl);

                AutocompleteView *v = new AutocompleteView();
                v->Type = 2;
                v->Text = "No project";
                v->ProjectAndTaskLabel = "";
                cl->display(v);
                it->setSizeHint(QSize(it->sizeHint().width(), 50));
            }
        }

        // Add Client name
        if (view->Type == 2 && view->ClientLabel != lastClientLabel)
        {
            QListWidgetItem *it = new QListWidgetItem(dropdown);
            AutocompleteCellWidget *cl = new AutocompleteCellWidget();
            dropdown->addItem(it);
            dropdown->setItemWidget(it, cl);

            AutocompleteView *v = new AutocompleteView();
            v->Type = 12;
            v->Text = view->ClientLabel;
            if (v->Text.count() == 0)
            {
                v->Text = "No Client";
            }
            cl->display(v);
            it->setSizeHint(QSize(it->sizeHint().width(), 50));
            lastClientLabel = v->ClientLabel;
        }

        QListWidgetItem *item = new QListWidgetItem(dropdown);
        AutocompleteCellWidget *cell = new AutocompleteCellWidget();
        dropdown->addItem(item);
        dropdown->setItemWidget(item, cell);
        cell->display(view);
        item->setSizeHint(QSize(item->sizeHint().width(), 50));
    }

    while (dropdown->count() > size) {
        dropdown->model()->removeRow(size);
    }
}

void TimerWidget::timeout() {
    if (!isVisible()) {
        return;
    }
    if (duration >= 0) {
        return;
    }
    ui->duration->setText(TogglApi::formatDurationInSecondsHHMMSS(duration));
}

void TimerWidget::on_description_currentIndexChanged(int index) {
    QVariant data = ui->description->currentData();
    if (data.canConvert<AutocompleteView *>()) {
        AutocompleteView *view = data.value<AutocompleteView *>();
        ui->description->setEditText(view->Description);
        ui->project->setText(view->ProjectAndTaskLabel);
        ui->billable->setVisible(view->Billable);
        ui->tags->setVisible(!view->Tags.isEmpty());
        if (!view->Tags.isEmpty()) {
            tagsHolder = view->Tags;
        } else {
            tagsHolder = "";
        }

    }
}

void TimerWidget::mousePressEvent(QMouseEvent *event) {
    TogglApi::instance->editRunningTimeEntry("");
    QWidget::mousePressEvent(event);
}

void TimerWidget::on_duration_returnPressed() {
    start();
}

void TimerWidget::resizeEvent(QResizeEvent* event)
{
    setEllipsisTextToLabel(ui->project, project);
    QWidget::resizeEvent(event);
}

void TimerWidget::setEllipsisTextToLabel(QLabel *label, QString text)
{
    QFontMetrics metrix(label->font());
    int width = label->width() - 4;
    QString clippedText = metrix.elidedText(text, Qt::ElideRight, width);
    label->setText(clippedText);
}
