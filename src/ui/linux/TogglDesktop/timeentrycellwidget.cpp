#include "timeentrycellwidget.h"
#include "ui_timeentrycellwidget.h"

#include "toggl_api.h"

TimeEntryCellWidget::TimeEntryCellWidget(TimeEntryView *view) :
    QWidget(0),
    ui(new Ui::TimeEntryCellWidget),
    guid("")
{
    ui->setupUi(this);

    guid = view->GUID;

    ui->description->setText(view->Description);
    ui->project->setText(view->ProjectAndTaskLabel);
    ui->duration->setText(view->Duration);

    ui->billable->setVisible(view->Billable);
    ui->tags->setVisible(!view->Tags.isEmpty());
}

TimeEntryCellWidget::~TimeEntryCellWidget()
{
    delete ui;
}

void TimeEntryCellWidget::on_continueButton_clicked()
{
    TogglApi::instance->continueTimeEntry(guid);

}
