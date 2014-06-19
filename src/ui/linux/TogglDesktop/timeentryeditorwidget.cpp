#include "timeentryeditorwidget.h"
#include "ui_timeentryeditorwidget.h"

#include "toggl_api.h"

TimeEntryEditorWidget::TimeEntryEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeEntryEditorWidget),
    guid("")
{
    ui->setupUi(this);

    setVisible(false);

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),
            this, SLOT(displayLogin(bool,uint64_t)));

    connect(TogglApi::instance, SIGNAL(displayTimeEntryList(bool,QVector<TimeEntryView*>)),
            this, SLOT(displayTimeEntryList(bool,QVector<TimeEntryView*>)));

    connect(TogglApi::instance, SIGNAL(displayTimeEntryEditor(bool,TimeEntryView*,QString)),
            this, SLOT(displayTimeEntryEditor(bool,TimeEntryView*,QString)));
}

TimeEntryEditorWidget::~TimeEntryEditorWidget()
{
    delete ui;
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
    ui->duration->setText(view->Duration);
    ui->billable->setChecked(view->Billable);
    ui->timeOverview->setText(view->timeOverview());

    if (view->UpdatedAt)
    {
        ui->lastUpdate->setText(view->lastUpdate());
        ui->lastUpdate->setVisible(true);
    }
    else
    {
        ui->lastUpdate->setVisible(false);
        ui->lastUpdate->setText("");
    }
}

void TimeEntryEditorWidget::on_doneButton_clicked()
{
    TogglApi::instance->viewTimeEntryList();
}
