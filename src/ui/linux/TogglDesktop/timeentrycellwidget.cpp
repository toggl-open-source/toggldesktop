// Copyright 2014 Toggl Desktop developers.

#include "./timeentrycellwidget.h"
#include "./ui_timeentrycellwidget.h"

#include "./toggl.h"

TimeEntryCellWidget::TimeEntryCellWidget() : QWidget(0),
ui(new Ui::TimeEntryCellWidget),
guid(""),
description(""),
project(""),
groupName(""),
group(false) {
    ui->setupUi(this);
}

void TimeEntryCellWidget::display(TimeEntryView *view) {
    setLoadMore(false);
    guid = view->GUID;
    groupName = view->GroupName;
    description =
        (view->Description.length() > 0) ?
        view->Description : "(no description)";
    project = view->ProjectAndTaskLabel;

    setEllipsisTextToLabel(ui->description, description);
    setEllipsisTextToLabel(ui->project, project);
    ui->project->setStyleSheet("color: '" + getProjectColor(view->Color) + "'");
    ui->duration->setText(view->Duration);

    ui->billable->setVisible(view->Billable);
    ui->tags->setVisible(!view->Tags.isEmpty());

    ui->headerFrame->setVisible(view->IsHeader);
    ui->date->setText(view->DateHeader);
    ui->dateDuration->setText(view->DateDuration);

    ui->unsyncedicon->setVisible(view->Unsynced);

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
    setupGroupedMode(view);
}

void TimeEntryCellWidget::setLoadMore(bool load_more) {
    ui->headerFrame->setVisible(false);
    ui->loadMoreButton->setVisible(load_more);
    ui->descProjFrame->setVisible(!load_more);
    ui->groupFrame->setVisible(!load_more);
    ui->frame->setVisible(!load_more);
    if (load_more) {
        ui->dataFrame->setStyleSheet("background-color: rgb(229,229,229);");
        ui->unsyncedicon->setVisible(false);
    }
}

void TimeEntryCellWidget::setupGroupedMode(TimeEntryView *view) {
    // Grouped Mode Setup
    group = view->Group;
    QString style = "border-bottom:1px solid #cacaca;background-color: #FAFAFA;";
    QString count = "";
    QString continueIcon = ":/images/continue_light.svg";
    QString descriptionStyle = "border:none;";
    int left = 0;
    if (view->GroupItemCount && view->GroupOpen && !view->Group) {
        style = "border-bottom:1px solid #cacaca;background-color: #EFEFEF;";
        left = 10;
        descriptionStyle = "border:none;color:#878787";
    }
    ui->description->setStyleSheet(descriptionStyle);
    ui->descProjFrame->layout()->setContentsMargins(left, 9, 9, 9);
    ui->dataFrame->setStyleSheet(style);

    if (view->Group) {
        QString buttonStyle = "outline:none;border:none;font-size:11px;color:#a4a4a4;background: url(:/images/group_icon_open.svg) no-repeat;";

        if (!view->GroupOpen) {
            count = QString::number(view->GroupItemCount);
            buttonStyle = "outline:none;border:none;font-size:11px;color:#a4a4a4;background: url(:/images/group_icon_closed.svg) no-repeat;";
        }
        ui->groupButton->setStyleSheet(buttonStyle);
        continueIcon = ":/images/continue_regular.svg";
    }
    ui->groupButton->setText(count);
    ui->continueButton->setIcon(QIcon(continueIcon));
    ui->groupFrame->setVisible(view->Group);
}

void TimeEntryCellWidget::setEllipsisTextToLabel(ClickableLabel *label, QString text)
{
    QFontMetrics metrix(label->font());
    int width = label->width() - 2;
    QString clippedText = metrix.elidedText(text, Qt::ElideRight, width);
    label->setText(clippedText);
}

void TimeEntryCellWidget::labelClicked(QString field_name) {
    if (group) {
        on_groupButton_clicked();
        return;
    }
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
    if (group) {
        on_groupButton_clicked();
        return;
    }
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

void TimeEntryCellWidget::on_groupButton_clicked()
{
    TogglApi::instance->toggleEntriesGroup(groupName);
}

void TimeEntryCellWidget::resizeEvent(QResizeEvent* event)
{
    setEllipsisTextToLabel(ui->description, description);
    setEllipsisTextToLabel(ui->project, project);
    QWidget::resizeEvent(event);
}

void TimeEntryCellWidget::on_loadMoreButton_clicked()
{
    TogglApi::instance->loadMore();
    ui->loadMoreButton->setStyleSheet("color:#8c8c8c;");
    ui->loadMoreButton->setText("Loading ...");
}
