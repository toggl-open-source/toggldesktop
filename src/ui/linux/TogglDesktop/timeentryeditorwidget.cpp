#include "timeentryeditorwidget.h"
#include "ui_timeentryeditorwidget.h"

TimeEntryEditorWidget::TimeEntryEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeEntryEditorWidget)
{
    ui->setupUi(this);
}

TimeEntryEditorWidget::~TimeEntryEditorWidget()
{
    delete ui;
}
