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
    QString description =
        (view->Description.length() > 0) ?
        view->Description : "(no description)";
    ui->description->setText(description);
    ui->project->setText(view->ProjectAndTaskLabel);
    ui->project->setStyleSheet("color: '" + getProjectColor(view->Color) + "'");
    ui->duration->setText(view->Duration);

    ui->billable->setVisible(view->Billable);
    ui->tags->setVisible(!view->Tags.isEmpty());

    ui->headerFrame->setVisible(view->IsHeader);
    ui->date->setText(view->DateHeader);
    ui->dateDuration->setText(view->DateDuration);

    if (view->StartTimeString.length() > 0 &&
            view->EndTimeString.length() > 0) {
        ui->duration->setToolTip(
            QString("<p style='color:black;background-color:white;'>" +
                    view->StartTimeString + " - " +
                    view->EndTimeString+"</p>"));
    }

    ui->tags->setToolTip(
        QString("<p style='color:black;background-color:white;'>" +
                (view->Tags).replace(QString("\t"), QString(", ")) + "</p>"));
    if (view->Description.length() > 0) {
        ui->description->setToolTip(
            QString("<p style='color:white;background-color:black;'>" +
                    view->Description + "</p>"));
    }
    if (view->ProjectAndTaskLabel.length() > 0) {
        ui->project->setToolTip(
            QString("<p style='color:white;background-color:black;'>" +
                    view->ProjectAndTaskLabel + "</p>"));
    }
}

void TimeEntryCellWidget::labelClicked(QString field_name) {
    TogglApi::instance->editTimeEntry(guid, field_name);
}

TimeEntryCellWidget::~TimeEntryCellWidget() {
    delete ui;
}

QSize TimeEntryCellWidget::getSizeHint(bool is_header) {
    if (is_header) {
        return QSize(minimumWidth(), sizeHint().height());
    }
    return QSize(minimumWidth(), ui->dataFrame->height());
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
