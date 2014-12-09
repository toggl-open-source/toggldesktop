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
    ui->project->setStyleSheet("color: '" + getProjectColor(view->Color) + "'");
    ui->duration->setText(view->Duration);

    ui->billable->setVisible(view->Billable);
    ui->tags->setVisible(!view->Tags.isEmpty());

    ui->headerFrame->setVisible(view->IsHeader);
    ui->date->setText(view->DateHeader);
    ui->dateDuration->setText(view->DateDuration);

    ui->tags->setToolTip(
        QString("<p style='color:black;'>"+
                (view->Tags).replace(QString("\t"), QString(", "))+"</p>"));
    ui->description->setToolTip(
        QString("<p style='color:black;'>"+view->Description+"</p>"));
    ui->project->setToolTip(
        QString("<p style='color:black;'>"+view->ProjectAndTaskLabel+"</p>"));
}

void TimeEntryCellWidget::labelClicked(QString field_name) {
    TogglApi::instance->editTimeEntry(guid, field_name);
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

QString TimeEntryCellWidget::getProjectColor(QString color) {
    if (color.length() == 0) {
        return QString("#9d9d9d");
    }
    return color;
}
