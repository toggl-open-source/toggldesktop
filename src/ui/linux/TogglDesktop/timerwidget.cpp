#include "timerwidget.h"
#include "ui_timerwidget.h"

#include "toggl_api.h"

TimerWidget::TimerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimerWidget)
{
    ui->setupUi(this);

    connect(TogglApi::instance, SIGNAL(displayStoppedTimerState()),
            this, SLOT(displayStoppedTimerState()));
    connect(TogglApi::instance, SIGNAL(displayRunningTimerState(TimeEntryView*)),
            this, SLOT(displayRunningTimerState(TimeEntryView*)));
}

TimerWidget::~TimerWidget()
{
    delete ui;
}

void TimerWidget::displayRunningTimerState(
    TimeEntryView *te)
{
    ui->start->setText("Stop");

    ui->description->setText(te->Description);
    ui->duration->setText(te->Duration);

    disconnect(this, SLOT(start()));
    disconnect(this, SLOT(stop()));

    connect(this, SIGNAL(buttonClicked()), this, SLOT(stop()));
}

void TimerWidget::displayStoppedTimerState()
{
    ui->start->setText("Start");

    ui->description->setText("");
    ui->duration->setText("");

    disconnect(this, SLOT(start()));
    disconnect(this, SLOT(stop()));

    connect(this, SIGNAL(buttonClicked()), this, SLOT(start()));
}

void TimerWidget::on_start_clicked()
{
    emit buttonClicked();
}

void TimerWidget::start()
{
    TogglApi::instance->start(ui->description->text(),
                              ui->duration->text(),
                              0, // FIXME: task_id
                              0); // FIXME: project_id
}

void TimerWidget::stop()
{
    TogglApi::instance->stop();
}
