// Copyright 2014 Toggl Desktop developers.

#include "./timeentryeditorwidget.h"
#include "./ui_timeentryeditorwidget.h"

#include <QMessageBox>  // NOLINT
#include <QDebug>  // NOLINT

#include "./toggl.h"

TimeEntryEditorWidget::TimeEntryEditorWidget(QWidget *parent) : QWidget(parent),
ui(new Ui::TimeEntryEditorWidget),
guid(""),
timeEntryAutocompleteNeedsUpdate(false),
projectAutocompleteNeedsUpdate(false),
workspaceSelectNeedsUpdate(false),
clientSelectNeedsUpdate(false),
timer(new QTimer(this)),
duration(0) {
    ui->setupUi(this);

    setVisible(false);

    ui->addNewProject->setText(
        "<a href=\"#add_new_project\">Add new project</a>");

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTimeEntryList(bool,QVector<TimeEntryView*>)),  // NOLINT
            this, SLOT(displayTimeEntryList(bool,QVector<TimeEntryView*>)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTimeEntryEditor(bool,TimeEntryView*,QString)),  // NOLINT
            this, SLOT(displayTimeEntryEditor(bool,TimeEntryView*,QString)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTags(QVector<GenericView*>)),  // NOLINT
            this, SLOT(displayTags(QVector<GenericView*>)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayWorkspaceSelect(QVector<GenericView*>)),  // NOLINT
            this, SLOT(displayWorkspaceSelect(QVector<GenericView*>)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayProjectAutocomplete(QVector<AutocompleteView*>)),  // NOLINT
            this, SLOT(displayProjectAutocomplete(QVector<AutocompleteView*>)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTimeEntryAutocomplete(QVector<AutocompleteView*>)),  // NOLINT
            this, SLOT(displayTimeEntryAutocomplete(QVector<AutocompleteView*>)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayClientSelect(QVector<GenericView*>)),  // NOLINT
            this, SLOT(displayClientSelect(QVector<GenericView*>)));  // NOLINT

    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

TimeEntryEditorWidget::~TimeEntryEditorWidget() {
    delete ui;
}

void TimeEntryEditorWidget::displayClientSelect(
    QVector<GenericView *> list) {
    clientSelectUpdate = list;
    clientSelectNeedsUpdate = true;
    if (ui->newProjectClient->hasFocus()) {
        return;
    }

    uint64_t workspaceID = 0;
    QVariant data = ui->newProjectWorkspace->currentData();
    if (data.canConvert<GenericView *>()) {
        GenericView *view = data.value<GenericView *>();
        workspaceID = view->ID;
    }
    ui->newProjectClient->clear();
    ui->newProjectClient->addItem("");
    foreach(GenericView *view, clientSelectUpdate) {
        if (workspaceID && workspaceID != view->WID) {
            continue;
        }
        ui->newProjectClient->addItem(view->Name, QVariant::fromValue(view));
    }
    clientSelectNeedsUpdate = false;
}

void TimeEntryEditorWidget::displayTimeEntryAutocomplete(
    QVector<AutocompleteView *> list) {
    timeEntryAutocompleteUpdate = list;
    timeEntryAutocompleteNeedsUpdate = true;
    if (ui->description->hasFocus()) {
        return;
    }
    ui->description->clear();
    ui->description->addItem("");
    foreach(AutocompleteView *view, timeEntryAutocompleteUpdate) {
        ui->description->addItem(view->Text, QVariant::fromValue(view));
    }
    timeEntryAutocompleteNeedsUpdate = false;
}

void TimeEntryEditorWidget::displayProjectAutocomplete(
    QVector<AutocompleteView *> list) {
    projectAutocompleteUpdate = list;
    projectAutocompleteNeedsUpdate = true;
    if (ui->project->hasFocus()) {
        return;
    }
    ui->project->clear();
    ui->project->addItem("");
    foreach(AutocompleteView *view, projectAutocompleteUpdate) {
        ui->project->addItem(view->Text, QVariant::fromValue(view));
    }
    projectAutocompleteNeedsUpdate = false;
}

void TimeEntryEditorWidget::displayWorkspaceSelect(
    QVector<GenericView *> list) {
    workspaceSelectUpdate = list;
    workspaceSelectNeedsUpdate = true;
    if (ui->newProjectWorkspace->hasFocus()) {
        return;
    }
    ui->newProjectWorkspace->clear();
    foreach(GenericView *view, workspaceSelectUpdate) {
        ui->newProjectWorkspace->addItem(view->Name, QVariant::fromValue(view));
    }
    workspaceSelectNeedsUpdate = false;
}

void TimeEntryEditorWidget::displayLogin(
    const bool open,
    const uint64_t user_id) {
    if (open || !user_id) {
        setVisible(false);
        timer->stop();
    }
}

void TimeEntryEditorWidget::displayTimeEntryList(
    const bool open,
    QVector<TimeEntryView *> list) {
    if (open) {
        setVisible(false);
        timer->stop();
    }
}

void TimeEntryEditorWidget::displayTimeEntryEditor(
    const bool open,
    TimeEntryView *view,
    const QString focused_field_name) {
    if (open) {
        ui->timeDetails->setVisible(false);
        ui->timeOverview->setVisible(true);

        ui->newProject->setVisible(false);
        ui->addNewProject->setVisible(true);

        setVisible(true);

        if (focused_field_name == TogglApi::Duration) {
            ui->duration->setFocus();
        } else if (focused_field_name == TogglApi::Description) {
            ui->description->setFocus();
        } else if (focused_field_name == TogglApi::Project) {
            ui->project->setFocus();
        }
    }

    guid = view->GUID;
    duration = view->DurationInSeconds;

    if (duration < 0) {
        timer->start(1000);
    }

    if (!ui->description->hasFocus()) {
        ui->description->setEditText(view->Description);
    }
    if (!ui->project->hasFocus()) {
        ui->project->setEditText(view->ProjectAndTaskLabel);
    }
    if (!ui->start->hasFocus()) {
        ui->start->setText(view->StartTimeString);
    }
    if (!ui->stop->hasFocus()) {
        ui->stop->setText(view->EndTimeString);
    }
    if (!ui->dateEdit->hasFocus()) {
        ui->dateEdit->setDateTime(QDateTime::fromTime_t(view->Started));
    }
    if (!ui->duration->hasFocus()) {
        ui->duration->setText(view->Duration);
    }
    ui->billable->setChecked(view->Billable);
    ui->timeOverview->setText(
        "<a href=\"#view_time_details\">" + view->timeOverview() + "</a>");

    ui->lastUpdate->setVisible(view->UpdatedAt);
    ui->lastUpdate->setText(view->lastUpdate());

    ui->billable->setVisible(view->CanSeeBillable);
    if (!view->CanAddProjects) {
        ui->newProject->setVisible(false);
    }

    for (int i = 0; i < ui->tags->count(); i++) {
        QListWidgetItem *item = ui->tags->item(i);
        if (view->Tags.contains(item->text())) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }
}

void TimeEntryEditorWidget::on_doneButton_clicked() {
    if (applyNewProject()) {
        TogglApi::instance->viewTimeEntryList();
    }
}

bool TimeEntryEditorWidget::applyNewProject() {
    if (!ui->newProject->isVisible()) {
        return true;
    }

    if (ui->newProjectName->text().isEmpty()) {
        ui->newProjectName->setFocus();
        return false;
    }

    QVariant workspace = ui->newProjectWorkspace->currentData();
    if (!workspace.canConvert<GenericView *>()) {
        return false;
    }
    uint64_t workspaceID = workspace.value<GenericView *>()->ID;

    uint64_t clientID = 0;
    QVariant client = ui->newProjectClient->currentData();
    if (client.canConvert<GenericView *>()) {
        clientID = client.value<GenericView *>()->ID;
    }

    return TogglApi::instance->addProject(guid,
                                          workspaceID,
                                          clientID,
                                          ui->newProjectName->text(),
                                          !ui->publicProject->isChecked());
}

void TimeEntryEditorWidget::on_deleteButton_clicked() {
    if (QMessageBox::Ok == QMessageBox(
        QMessageBox::Question,
        "Delete this time entry?",
        "Deleted time entries cannot be restored.",
        QMessageBox::Ok|QMessageBox::Cancel).exec()) {
        TogglApi::instance->deleteTimeEntry(guid);
    }
}

void TimeEntryEditorWidget::on_addNewProject_linkActivated(
    const QString &link) {
    ui->addNewProject->setVisible(false);
    ui->newProject->setVisible(true);
    ui->newProjectName->setFocus();
}

void TimeEntryEditorWidget::on_timeOverview_linkActivated(
    const QString &link) {
    ui->timeOverview->setVisible(false);
    ui->timeDetails->setVisible(true);
    ui->duration->setFocus();
}

void TimeEntryEditorWidget::on_newProjectWorkspace_currentIndexChanged(
    int index) {
    displayClientSelect(clientSelectUpdate);
}

void TimeEntryEditorWidget::on_description_currentIndexChanged(int index) {
    QVariant data = ui->description->currentData();
    if (data.canConvert<AutocompleteView *>()) {
        AutocompleteView *view = data.value<AutocompleteView *>();
        TogglApi::instance->setTimeEntryProject(guid,
                                                view->TaskID,
                                                view->ProjectID,
                                                "");
        ui->description->setEditText(view->Description);
    }
}

void TimeEntryEditorWidget::on_description_activated(const QString &arg1) {
    TogglApi::instance->setTimeEntryDescription(guid, arg1);
}

void TimeEntryEditorWidget::on_project_activated(int index) {
    QVariant data = ui->project->currentData();
    if (data.canConvert<AutocompleteView *>()) {
        AutocompleteView *view = data.value<AutocompleteView *>();
        TogglApi::instance->setTimeEntryProject(guid,
                                                view->TaskID,
                                                view->ProjectID,
                                                "");
    }
}

void TimeEntryEditorWidget::on_duration_editingFinished() {
    TogglApi::instance->setTimeEntryDuration(guid,
            ui->duration->text());
}

void TimeEntryEditorWidget::on_start_editingFinished() {
}

void TimeEntryEditorWidget::on_stop_editingFinished() {
}

void TimeEntryEditorWidget::on_dateEdit_editingFinished() {
}

void TimeEntryEditorWidget::displayTags(
    QVector<GenericView*> tags) {
    ui->tags->clear();
    foreach(GenericView *view, tags) {
        QListWidgetItem *item = new QListWidgetItem(view->Name, ui->tags);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
}

void TimeEntryEditorWidget::on_billable_clicked(bool checked) {
    TogglApi::instance->setTimeEntryBillable(guid, checked);
}

void TimeEntryEditorWidget::timeout() {
    if (duration < 0 &&
            ui->duration->isVisible() &&
            !ui->duration->hasFocus()) {
        ui->duration->setText(
            TogglApi::formatDurationInSecondsHHMMSS(duration));
    }
}

void TimeEntryEditorWidget::on_tags_itemActivated(QListWidgetItem *item) {
    // FIXME: set tags
    if (item->checkState() == Qt::Checked) {
    } else if (item->checkState() == Qt::Unchecked) {
    }
}
