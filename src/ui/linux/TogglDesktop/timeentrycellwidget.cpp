// Copyright 2014 Toggl Desktop developers.

#include "./timeentrycellwidget.h"
#include "./ui_timeentrycellwidget.h"

#include "./toggl.h"

TimeEntryCellWidget::TimeEntryCellWidget() : QWidget(0),
ui(new Ui::TimeEntryCellWidget),
guid("") {
    ui->setupUi(this);
}

void TimeEntryCellWidget::display(TimeEntryView *view) {
    guid = view->GUID;

    ui->description->setText(view->Description);
    ui->project->setText(view->ProjectAndTaskLabel);
    ui->duration->setText(view->Duration);

    ui->billable->setVisible(view->Billable);
    ui->tags->setVisible(!view->Tags.isEmpty());

    ui->headerFrame->setVisible(view->IsHeader);
    ui->date->setText(view->DateHeader);
    ui->dateDuration->setText(view->DateDuration);
}

TimeEntryCellWidget::~TimeEntryCellWidget() {
    delete ui;
}

QSize TimeEntryCellWidget::getSizeHint(bool is_header) {
    if (is_header) {
        return sizeHint();
    }
    return QSize(width(), ui->dataFrame->height());
}

void TimeEntryCellWidget::mousePressEvent(QMouseEvent *event) {
    TogglApi::instance->editTimeEntry(guid, "");
    QWidget::mousePressEvent(event);
}

void TimeEntryCellWidget::on_continueButton_clicked() {
    TogglApi::instance->continueTimeEntry(guid);
}
