#include "timerwidget.h"
#include "ui_timerwidget.h"

#include "toggl_api.h"

TimerWidget::TimerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimerWidget),
    timer(0),
    duration(0)
{
    ui->setupUi(this);

    connect(TogglApi::instance, SIGNAL(displayStoppedTimerState()),
            this, SLOT(displayStoppedTimerState()));
    connect(TogglApi::instance, SIGNAL(displayRunningTimerState(TimeEntryView*)),
            this, SLOT(displayRunningTimerState(TimeEntryView*)));
    connect(TogglApi::instance, SIGNAL(displayTimeEntryAutocomplete(QVector<AutocompleteView*>)),
            this, SLOT(displayTimeEntryAutocomplete(QVector<AutocompleteView*>)));
    connect(TogglApi::instance, SIGNAL(displayProjectAutocomplete(QVector<AutocompleteView*>)),
            this, SLOT(displayProjectAutocomplete(QVector<AutocompleteView*>)));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start(1000);
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
    ui->project->setCurrentText(te->ProjectAndTaskLabel);

    duration = te->DurationInSeconds;

    disconnect(this, SLOT(start()));
    disconnect(this, SLOT(stop()));

    connect(this, SIGNAL(buttonClicked()), this, SLOT(stop()));
}

void TimerWidget::displayStoppedTimerState()
{
    ui->start->setText("Start");

    ui->description->setText("");
    ui->duration->setText("");
    ui->project->setCurrentText("");

    duration = 0;

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

void TimerWidget::displayTimeEntryAutocomplete(
    QVector<AutocompleteView *> list)
{
}

void TimerWidget::displayProjectAutocomplete(
    QVector<AutocompleteView *> list)
{
    ui->project->clear();
    ui->project->addItem("");
    foreach(AutocompleteView *view, list)
    {
        ui->project->addItem(view->Text, QVariant::fromValue(view));
    }
}

void TimerWidget::timeout()
{
    if (duration >= 0)
    {
        return;
    }
    ui->duration->setText(TogglApi::formatDurationInSecondsHHMMSS(duration));
}
