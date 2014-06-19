#include "timerwidget.h"
#include "ui_timerwidget.h"

#include <QApplication>

#include "toggl_api.h"

TimerWidget::TimerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimerWidget),
    timer(0),
    duration(0),
    projectAutocompleteNeedsUpdate(false),
    timeEntryAutocompleteNeedsUpdate(false)
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

    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(focusChanged(QWidget*,QWidget*)));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

TimerWidget::~TimerWidget()
{
    timer->stop();

    delete ui;
}

void TimerWidget::focusChanged(QWidget *old, QWidget *now)
{
    if (!old)
    {
        return;
    }
    if (old == ui->description && timeEntryAutocompleteNeedsUpdate)
    {
        displayTimeEntryAutocomplete(timeEntryAutocompleteUpdate);
    }
    if (old == ui->project && projectAutocompleteNeedsUpdate)
    {
        displayProjectAutocomplete(projectAutocompleteUpdate);
    }
}

void TimerWidget::displayRunningTimerState(
    TimeEntryView *te)
{
    ui->start->setText("Stop");

    ui->description->setCurrentText(te->Description);
    ui->duration->setText(te->Duration);
    ui->project->setCurrentText(te->ProjectAndTaskLabel);

    duration = te->DurationInSeconds;

    disconnect(this, SLOT(start()));
    disconnect(this, SLOT(stop()));

    connect(this, SIGNAL(buttonClicked()), this, SLOT(stop()));

    timer->start(1000);
}

void TimerWidget::displayStoppedTimerState()
{
    ui->start->setText("Start");

    if (!ui->description->hasFocus())
    {
        ui->description->setCurrentText("");
    }
    if (!ui->duration->hasFocus())
    {
        ui->duration->setText("");

    }
    if (!ui->project->hasFocus())
    {
        ui->project->setCurrentText("");
    }

    duration = 0;

    disconnect(this, SLOT(start()));
    disconnect(this, SLOT(stop()));

    connect(this, SIGNAL(buttonClicked()), this, SLOT(start()));

    timer->stop();
}

void TimerWidget::on_start_clicked()
{
    emit buttonClicked();
}

void TimerWidget::start()
{
    TogglApi::instance->start(ui->description->currentText(),
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

void TimerWidget::displayProjectAutocomplete(
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
    foreach(AutocompleteView *view, list)
    {
        ui->project->addItem(view->Text, QVariant::fromValue(view));
    }
    projectAutocompleteNeedsUpdate = false;
}

void TimerWidget::timeout()
{
    if (duration >= 0)
    {
        return;
    }
    ui->duration->setText(TogglApi::formatDurationInSecondsHHMMSS(duration));
}
