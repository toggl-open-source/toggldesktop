#include "timeentryeditorwidget.h"
#include "ui_timeentryeditorwidget.h"

#include <QMessageBox>

#include "toggl_api.h"

TimeEntryEditorWidget::TimeEntryEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeEntryEditorWidget),
    guid(""),
    timeEntryAutocompleteNeedsUpdate(false),
    projectAutocompleteNeedsUpdate(false),
    workspaceSelectNeedsUpdate(false),
    clientSelectNeedsUpdate(false)
{
    ui->setupUi(this);

    setVisible(false);

    ui->addNewProject->setText("<a href=\"#add_new_project\">Add new project</a>");

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),
            this, SLOT(displayLogin(bool,uint64_t)));

    connect(TogglApi::instance, SIGNAL(displayTimeEntryList(bool,QVector<TimeEntryView*>)),
            this, SLOT(displayTimeEntryList(bool,QVector<TimeEntryView*>)));

    connect(TogglApi::instance, SIGNAL(displayTimeEntryEditor(bool,TimeEntryView*,QString)),
            this, SLOT(displayTimeEntryEditor(bool,TimeEntryView*,QString)));

    connect(TogglApi::instance, SIGNAL(displayTags(QVector<GenericView*>)),
            this, SLOT(displayTags(QVector<GenericView*>)));

    connect(TogglApi::instance, SIGNAL(displayWorkspaceSelect(QVector<GenericView*>)),
            this, SLOT(displayWorkspaceSelect(QVector<GenericView*>)));

    connect(TogglApi::instance, SIGNAL(displayProjectAutocomplete(QVector<AutocompleteView*>)),
            this, SLOT(displayProjectAutocomplete(QVector<AutocompleteView*>)));

    connect(TogglApi::instance, SIGNAL(displayTimeEntryAutocomplete(QVector<AutocompleteView*>)),
            this, SLOT(displayTimeEntryAutocomplete(QVector<AutocompleteView*>)));

    connect(TogglApi::instance, SIGNAL(displayClientSelect(QVector<GenericView*>)),
            this, SLOT(displayClientSelect(QVector<GenericView*>)));
}

TimeEntryEditorWidget::~TimeEntryEditorWidget()
{
    delete ui;
}

void TimeEntryEditorWidget::displayTags(
    QVector<GenericView*> tags)
{

}

void TimeEntryEditorWidget::displayClientSelect(
    QVector<GenericView *> list)
{
    clientSelectUpdate = list;
    clientSelectNeedsUpdate = true;
    if (ui->newProjectClient->hasFocus())
    {
        return;
    }

    u_int64_t workspaceID = 0;
    QVariant data = ui->newProjectWorkspace->currentData();
    if (data.canConvert<GenericView *>())
    {
        GenericView *view = data.value<GenericView *>();
        workspaceID = view->ID;
    }
    ui->newProjectClient->clear();
    ui->newProjectClient->addItem("");
    foreach(GenericView *view, clientSelectUpdate)
    {
        if (workspaceID && workspaceID != view->WID)
        {
            continue;
        }
        ui->newProjectClient->addItem(view->Name, QVariant::fromValue(view));
    }
    clientSelectNeedsUpdate = false;
}

void TimeEntryEditorWidget::displayTimeEntryAutocomplete(
    QVector<AutocompleteView *> list)
{
    timeEntryAutocompleteUpdate = list;
    timeEntryAutocompleteNeedsUpdate = true;
    if (ui->description->hasFocus())
    {
        return;
    }
    ui->description->clear();
    ui->description->addItem("");
    foreach(AutocompleteView *view, timeEntryAutocompleteUpdate)
    {
        ui->description->addItem(view->Text, QVariant::fromValue(view));
    }
    timeEntryAutocompleteNeedsUpdate = false;
}

void TimeEntryEditorWidget::displayProjectAutocomplete(
    QVector<AutocompleteView *> list)
{
    projectAutocompleteUpdate = list;
    projectAutocompleteNeedsUpdate = true;
    if (ui->project->hasFocus())
    {
        return;
    }
    ui->project->clear();
    ui->project->addItem("");
    foreach(AutocompleteView *view, projectAutocompleteUpdate)
    {
        ui->project->addItem(view->Text, QVariant::fromValue(view));
    }
    projectAutocompleteNeedsUpdate = false;
}

void TimeEntryEditorWidget::displayWorkspaceSelect(
    QVector<GenericView *> list)
{
    workspaceSelectUpdate = list;
    workspaceSelectNeedsUpdate = true;
    if (ui->newProjectWorkspace->hasFocus())
    {
        return;
    }
    ui->newProjectWorkspace->clear();
    foreach(GenericView *view, workspaceSelectUpdate)
    {
        ui->newProjectWorkspace->addItem(view->Name, QVariant::fromValue(view));
    }
    workspaceSelectNeedsUpdate = false;
}

void TimeEntryEditorWidget::displayLogin(
    const bool open,
    const uint64_t user_id)
{
    if (open || !user_id)
    {
        setVisible(false);
    }
}

void TimeEntryEditorWidget::displayTimeEntryList(
    const bool open,
    QVector<TimeEntryView *> list)
{
    if (open)
    {
        setVisible(false);
    }
}

void TimeEntryEditorWidget::displayTimeEntryEditor(
    const bool open,
    TimeEntryView *view,
    const QString focused_field_name)
{
    if (open)
    {
        ui->timeDetails->setVisible(false);
        ui->timeOverview->setVisible(true);

        ui->newProject->setVisible(false);
        ui->addNewProject->setVisible(true);

        setVisible(true);
    }

    guid = view->GUID;

    ui->description->setEditText(view->Description);
    ui->project->setEditText(view->ProjectAndTaskLabel);
    ui->start->setText(view->StartTimeString);
    ui->stop->setText(view->EndTimeString);
    ui->dateEdit->setDateTime(QDateTime::fromTime_t(view->Started));
    ui->duration->setText(view->Duration);
    ui->billable->setChecked(view->Billable);
    ui->timeOverview->setText("<a href=\"#view_time_details\">" + view->timeOverview() + "</a>");

    ui->lastUpdate->setVisible(view->UpdatedAt);
    ui->lastUpdate->setText(view->lastUpdate());
}

void TimeEntryEditorWidget::on_doneButton_clicked()
{
    TogglApi::instance->viewTimeEntryList();
}

void TimeEntryEditorWidget::on_deleteButton_clicked()
{
    if (QMessageBox::Ok == QMessageBox(QMessageBox::Question,
                                        "Delete this time entry?",
                                        "Deleted time entries cannot be restored.",
                                        QMessageBox::Ok|QMessageBox::Cancel).exec())
    {
        TogglApi::instance->deleteTimeEntry(guid);
    }
}

void TimeEntryEditorWidget::on_addNewProject_linkActivated(const QString &link)
{
    ui->addNewProject->setVisible(false);
    ui->newProject->setVisible(true);
    ui->newProjectName->setFocus();
}

void TimeEntryEditorWidget::on_timeOverview_linkActivated(const QString &link)
{
    ui->timeOverview->setVisible(false);
    ui->timeDetails->setVisible(true);
    ui->duration->setFocus();
}

void TimeEntryEditorWidget::on_newProjectWorkspace_currentIndexChanged(int index)
{
    displayClientSelect(clientSelectUpdate);
}
