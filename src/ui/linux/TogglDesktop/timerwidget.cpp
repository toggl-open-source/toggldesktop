// Copyright 2014 Toggl Desktop developers.

#include "./timerwidget.h"
#include "./ui_timerwidget.h"

#include <QApplication>  // NOLINT
#include <QCompleter>  // NOLINT

#include "./autocompletelistmodel.h"
#include "./autocompleteview.h"
#include "./timeentryview.h"
#include "./toggl.h"

TimerWidget::TimerWidget(QWidget *parent) : QWidget(parent),
ui(new Ui::TimerWidget),
timer(new QTimer(this)),
duration(0),
project(""),
tagsHolder(""),
timeEntryAutocompleteNeedsUpdate(false),
descriptionModel(new AutocompleteListModel(this)) {
    ui->setupUi(this);

    connect(TogglApi::instance, SIGNAL(displayStoppedTimerState()),
            this, SLOT(displayStoppedTimerState()));

    connect(TogglApi::instance, SIGNAL(displayRunningTimerState(TimeEntryView*)),  // NOLINT
            this, SLOT(displayRunningTimerState(TimeEntryView*)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayMinitimerAutocomplete(QVector<AutocompleteView*>)),  // NOLINT
            this, SLOT(displayMinitimerAutocomplete(QVector<AutocompleteView*>)));  // NOLINT

    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)),
            this, SLOT(focusChanged(QWidget*, QWidget*)));

    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));

    connect(ui->description->lineEdit(), SIGNAL(returnPressed()),
            this, SLOT(descriptionReturnPressed()));

    ui->description->setModel(descriptionModel);

    ui->billable->setVisible(false);
    ui->tags->setVisible(false);

    descriptionPlaceholder = "What are you doing?";
    tagsHolder = "";
}

TimerWidget::~TimerWidget() {
    timer->stop();

    delete ui;
}

void TimerWidget::descriptionReturnPressed() {
    start();
}

void TimerWidget::focusChanged(QWidget *old, QWidget *now) {
    if (old == ui->description) {
        if (ui->description->currentText().length() == 0) {
            ui->description->setEditText(descriptionPlaceholder);
        }
        if (timeEntryAutocompleteNeedsUpdate) {
            displayMinitimerAutocomplete(timeEntryAutocompleteUpdate);
        }
    }
    if (now == ui->description &&
            ui->description->currentText() == descriptionPlaceholder) {
        ui->description->setEditText("");
    }
}

void TimerWidget::displayRunningTimerState(
    TimeEntryView *te) {
    ui->start->setText("Stop");
    ui->start->setStyleSheet(
        "background-color: #e20000; color:'white'; font-weight: bold;");

    QString description = (te->Description.length() > 0) ?
                          te->Description : "(no description)";

    ui->description->setEditText(description);
    ui->description->setEnabled(false);

    ui->duration->setText(te->Duration);
    ui->duration->setEnabled(false);
    ui->duration->setToolTip(
        QString("<p style='color:white;background-color:black;'>Started: " +
                te->StartTimeString+"</p>"));

    project = te->ProjectAndTaskLabel;
    setEllipsisTextToLabel(ui->project, project);

    ui->billable->setVisible(te->Billable);
    ui->tags->setVisible(!te->Tags.isEmpty());
    ui->tags->setToolTip(QString("<p style='color:white;background-color:black;'>" +
                                 te->Tags + "</p>"));

    duration = te->DurationInSeconds;

    if (te->Description.length() > 0) {
        ui->description->setToolTip(
            QString("<p style='color:white;background-color:black;'>" +
                    te->Description + "</p>"));
    }
    if (te->ProjectAndTaskLabel.length() > 0) {
        ui->project->setToolTip(
            QString("<p style='color:white;background-color:black;'>" +
                    te->ProjectAndTaskLabel+"</p>"));
    } else {
        ui->project->setToolTip(QString(""));
    }

    disconnect(this, SLOT(start()));
    disconnect(this, SLOT(stop()));

    connect(this, SIGNAL(buttonClicked()), this, SLOT(stop()));

    timer->start(1000);
}

void TimerWidget::displayStoppedTimerState() {
    ui->start->setText("Start");
    ui->start->setStyleSheet(
        "background-color: #47bc00; color:'white'; font-weight: bold;");

    if (!ui->description->hasFocus()) {
        ui->description->setEditText(descriptionPlaceholder);
    }
    ui->description->setEnabled(true);
    ui->description->setToolTip(QString(""));

    if (!ui->duration->hasFocus()) {
        ui->duration->setText("");
    }
    ui->duration->setEnabled(true);
    ui->duration->setToolTip(QString(""));

    ui->project->setText("");
    ui->project->setToolTip(QString(""));

    ui->billable->setVisible(false);
    ui->tags->setVisible(false);

    tagsHolder = "";

    duration = 0;

    disconnect(this, SLOT(start()));
    disconnect(this, SLOT(stop()));

    connect(this, SIGNAL(buttonClicked()), this, SLOT(start()));

    timer->stop();
}

void TimerWidget::on_start_clicked() {
    emit buttonClicked();
}

void TimerWidget::start() {
    uint64_t task_id(0);
    uint64_t project_id(0);

    QVariant data = ui->description->currentData();
    if (data.canConvert<AutocompleteView *>()) {
        AutocompleteView *view = data.value<AutocompleteView *>();
        task_id = view->TaskID;
        project_id = view->ProjectID;
    }

    QString description = ui->description->currentText();
    if (description == descriptionPlaceholder) {
        description = "";
    }

    TogglApi::instance->start(description,
                              ui->duration->text(),
                              task_id,
                              project_id,
                              tagsHolder.toStdString().c_str(),
                              ui->billable->isVisible());
    tagsHolder = "";
}

void TimerWidget::stop() {
    TogglApi::instance->stop();
}

void TimerWidget::displayMinitimerAutocomplete(
    QVector<AutocompleteView *> list) {
    timeEntryAutocompleteUpdate = list;
    timeEntryAutocompleteNeedsUpdate = true;
    if (ui->description->hasFocus()) {
        return;
    }
    QString currentText = ui->description->currentText();
    ui->description->clear();
    descriptionModel->setList(list);
    /*
    ui->description->addItem("");
    foreach(AutocompleteView *view, timeEntryAutocompleteUpdate) {
        ui->description->addItem(view->Text, QVariant::fromValue(view));
    }
    */
    timeEntryAutocompleteNeedsUpdate = false;
    ui->description->setEditText(currentText);
}

void TimerWidget::timeout() {
    if (!isVisible()) {
        return;
    }
    if (duration >= 0) {
        return;
    }
    ui->duration->setText(TogglApi::formatDurationInSecondsHHMMSS(duration));
}

void TimerWidget::on_description_currentIndexChanged(int index) {
    Q_UNUSED(index);
    QVariant data = ui->description->currentData();
    if (data.canConvert<AutocompleteView *>()) {
        AutocompleteView *view = data.value<AutocompleteView *>();
        ui->description->setEditText(view->Description);
        ui->project->setText(view->ProjectAndTaskLabel);
        ui->billable->setVisible(view->Billable);
        ui->tags->setVisible(!view->Tags.isEmpty());
        if (!view->Tags.isEmpty()) {
            tagsHolder = view->Tags;
        } else {
            tagsHolder = "";
        }

    }
}

void TimerWidget::mousePressEvent(QMouseEvent *event) {
    TogglApi::instance->editRunningTimeEntry("");
    QWidget::mousePressEvent(event);
}

void TimerWidget::on_duration_returnPressed() {
    start();
}

void TimerWidget::resizeEvent(QResizeEvent* event)
{
    setEllipsisTextToLabel(ui->project, project);
    QWidget::resizeEvent(event);
}

void TimerWidget::setEllipsisTextToLabel(QLabel *label, QString text)
{
    QFontMetrics metrix(label->font());
    int width = label->width() - 4;
    QString clippedText = metrix.elidedText(text, Qt::ElideRight, width);
    label->setText(clippedText);
}
