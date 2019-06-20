// Copyright 2014 Toggl Desktop developers.

#include "./timeentryeditorwidget.h"
#include "./ui_timeentryeditorwidget.h"

#include <QCompleter>  // NOLINT
#include <QDebug>  // NOLINT
#include <QMessageBox>  // NOLINT
#include <QKeyEvent> // NOLINT
#include <QTimer> // NOLINT
#include <QLineEdit> // NOLINT

#include "./autocompletelistview.h"
#include "./autocompletelistmodel.h"
#include "./autocompleteview.h"
#include "./genericview.h"
#include "./timeentryview.h"
#include "./toggl.h"

TimeEntryEditorWidget::TimeEntryEditorWidget(QStackedWidget *parent) : QWidget(parent),
ui(new Ui::TimeEntryEditorWidget),
guid(""),
timeEntryAutocompleteNeedsUpdate(false),
projectAutocompleteNeedsUpdate(false),
workspaceSelectNeedsUpdate(false),
clientSelectNeedsUpdate(false),
colorPicker(new ColorPicker(this)),
timer(new QTimer(this)),
duration(0),
previousTagList(""),
descriptionModel(new AutocompleteListModel(this, QVector<AutocompleteView*>())),
projectModel(new AutocompleteListModel(this, QVector<AutocompleteView*>(), AC_PROJECT))
{
    ui->setupUi(this);

    ui->description->setModel(descriptionModel);
    ui->project->setModel(projectModel);

    ui->description->installEventFilter(this);
    ui->project->installEventFilter(this);

    connect(ui->description, &AutocompleteComboBox::returnPressed, [this]() {
        focusNextChild();
    });
    connect(ui->project, &AutocompleteComboBox::returnPressed, [this]() {
        focusNextChild();
    });

    connect(ui->newProjectWorkspace, QOverload<int>::of(&QComboBox::activated), [this](int) {
        focusNextChild();
    });
    connect(ui->newProjectClient, QOverload<int>::of(&QComboBox::activated), [this](int) {
        focusNextChild();
    });

    toggleNewClientMode(false);

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(aboutToDisplayTimeEntryList()),  // NOLINT
            this, SLOT(aboutToDisplayTimeEntryList()));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTimeEntryEditor(bool,TimeEntryView*,QString)),  // NOLINT
            this, SLOT(displayTimeEntryEditor(bool,TimeEntryView*,QString)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTags(QVector<GenericView*>)),  // NOLINT
            this, SLOT(displayTags(QVector<GenericView*>)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayWorkspaceSelect(QVector<GenericView*>)),  // NOLINT
            this, SLOT(displayWorkspaceSelect(QVector<GenericView*>)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayProjectAutocomplete(QVector<AutocompleteView*>)),  // NOLINT
            this, SLOT(displayProjectAutocomplete(QVector<AutocompleteView*>)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTimeEntryAutocomplete(QVector<AutocompleteView*>)),  // NOLINT
            this, SLOT(displayTimeEntryAutocomplete(QVector<AutocompleteView*>)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayClientSelect(QVector<GenericView*>)),  // NOLINT
            this, SLOT(displayClientSelect(QVector<GenericView*>)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(setProjectColors(QVector<char*>)),  // NOLINT
            this, SLOT(setProjectColors(QVector<char*>)));  // NOLINT

    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));

    TogglApi::instance->getProjectColors();
}

TimeEntryEditorWidget::~TimeEntryEditorWidget() {
    delete ui;
}

void TimeEntryEditorWidget::setSelectedColor(QString color) {
    QString style = "font-size:72px;" + color;
    ui->colorButton->setStyleSheet(style);
}

void TimeEntryEditorWidget::display() {
    qobject_cast<QStackedWidget*>(parent())->setCurrentWidget(this);
}

void TimeEntryEditorWidget::deleteTimeEntry() {
    if (timeEntry->confirmlessDelete() || QMessageBox::Ok == QMessageBox(
        QMessageBox::Question,
        "Delete this time entry?",
        "Deleted time entries cannot be restored.",
        QMessageBox::Ok|QMessageBox::Cancel).exec()) {
        TogglApi::instance->deleteTimeEntry(guid);
    }
}

void TimeEntryEditorWidget::clickDone()
{
    on_doneButton_clicked();
}

void TimeEntryEditorWidget::displayClientSelect(
    QVector<GenericView *> list) {
    clientSelectUpdate = list;
    clientSelectNeedsUpdate = true;
    if (ui->newProjectClient->hasFocus()) {
        return;
    }

    uint64_t workspaceID = 0;
    QVariant data = ui->newProjectWorkspace->currentData();
    if (data.canConvert<GenericView *>()) {
        GenericView *view = data.value<GenericView *>();
        workspaceID = view->ID;
    }
    ui->newProjectClient->clear();
    ui->newProjectClient->addItem("");
    foreach(GenericView *view, clientSelectUpdate) {
        if (workspaceID && workspaceID != view->WID) {
            continue;
        }
        ui->newProjectClient->addItem(QString::fromStdString(view->Name), QVariant::fromValue(view));
    }
    clientSelectNeedsUpdate = false;
}

void TimeEntryEditorWidget::displayTimeEntryAutocomplete(
    QVector<AutocompleteView *> list) {
    timeEntryAutocompleteUpdate = list;
    timeEntryAutocompleteNeedsUpdate = true;
    if (ui->description->hasFocus()) {
        return;
    }
    QString currentText = ui->description->currentText();
    ui->description->clear();
    descriptionModel->setList(list);
    timeEntryAutocompleteNeedsUpdate = false;
    ui->description->setEditText(currentText);
}

void TimeEntryEditorWidget::displayProjectAutocomplete(
    QVector<AutocompleteView *> list) {
    projectAutocompleteUpdate = list;
    projectAutocompleteNeedsUpdate = true;
    if (ui->project->hasFocus()) {
        return;
    }
    ui->project->clear();
    ui->project->addItem("");

    projectModel->setList(list);
    projectAutocompleteNeedsUpdate = false;
}

void TimeEntryEditorWidget::displayWorkspaceSelect(
    QVector<GenericView *> list) {
    workspaceSelectUpdate = list;
    workspaceSelectNeedsUpdate = true;
    if (ui->newProjectWorkspace->hasFocus()) {
        return;
    }
    ui->newProjectWorkspace->clear();
    foreach(GenericView *view, workspaceSelectUpdate) {
        ui->newProjectWorkspace->addItem(QString::fromStdString(view->Name), QVariant::fromValue(view));
    }
    workspaceSelectNeedsUpdate = false;
}

void TimeEntryEditorWidget::displayLogin(
    const bool open,
    const uint64_t user_id) {
    if (open || !user_id) {
        timer->stop();
    }
}

void TimeEntryEditorWidget::aboutToDisplayTimeEntryList() {
    timer->stop();
}

void TimeEntryEditorWidget::displayTimeEntryEditor(
    const bool open,
    TimeEntryView *view,
    const QString focused_field_name) {

    timeEntry = view;

    if (!ui->description->hasFocus()) {
        ui->description->setEditText(QString::fromStdString(view->Description));
    }
    if (!ui->project->hasFocus()) {
        ui->project->setEditText(QString::fromStdString(view->ProjectAndTaskLabel));
    }
    if (!ui->duration->hasFocus()) {
        ui->duration->setText(QString::fromStdString(view->Duration));
    }

    if (open) {
        // Show the dialog first, hide items later (to preserve size)
        display();

        // Reset adding new project
        ui->newProject->setVisible(false);
        ui->project->setVisible(true);
        ui->addNewProject->setVisible(true);
        ui->newProjectName->setText("");
        ui->publicProject->setChecked(false);
        ui->newProjectWorkspace->setCurrentIndex(-1);
        ui->newProjectClient->setCurrentIndex(-1);

        // Reset adding new client
        toggleNewClientMode(false);
        toggleNewTagMode(false);

        if (focused_field_name == TogglApi::Duration) {
            ui->duration->setFocus();
        } else if (focused_field_name == TogglApi::Description) {
            ui->description->setFocus();
        } else if (focused_field_name == TogglApi::Project) {
            ui->project->setFocus();
        }
    }

    guid = QString::fromStdString(view->GUID);
    duration = view->DurationInSeconds;

    if (duration < 0) {
        timer->start(1000);
    }

    if (!ui->start->hasFocus()) {
        ui->start->setText(QString::fromStdString(view->StartTimeString));
    }
    if (!ui->stop->hasFocus()) {
        ui->stop->setText(QString::fromStdString(view->EndTimeString));
    }
    ui->stop->setVisible(duration >= 0);
    ui->timeSeparator->setVisible(duration >= 0);
    if (!ui->dateEdit->hasFocus()) {
        ui->dateEdit->setDateTime(QDateTime::fromTime_t(view->Started));
    }

    ui->billable->setChecked(view->Billable);

    ui->lastUpdate->setVisible(view->UpdatedAt);
    ui->lastUpdate->setText(view->lastUpdate());

    ui->billable->setVisible(view->CanSeeBillable);
    if (!view->CanAddProjects) {
        ui->newProject->setVisible(false);
        ui->addNewProject->setVisible(false);
    }

    QStringList tags = QString::fromStdString(view->Tags).split("\t", QString::SkipEmptyParts);
    tags.sort();
    previousTagList = tags.join("\t");

    for (int i = 0; i < ui->tags->count(); i++) {
        QListWidgetItem *item = ui->tags->item(i);
        if (tags.contains(item->text())) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }

    ui->workspaceNameLabel->setText(QString::fromStdString(view->WorkspaceName));
}

void TimeEntryEditorWidget::on_doneButton_clicked() {
    if (applyNewProject()) {
        TogglApi::instance->viewTimeEntryList();
    }
}

bool TimeEntryEditorWidget::applyNewProject() {
    if (!ui->newProject->isVisible()) {
        return true;
    }

    if (ui->newProjectName->text().isEmpty()) {
        return true;
    }

    QVariant workspace = ui->newProjectWorkspace->currentData();
    if (!workspace.canConvert<GenericView *>()) {
        return false;
    }
    uint64_t workspaceID = workspace.value<GenericView *>()->ID;

    uint64_t clientID = 0;
    QVariant client = ui->newProjectClient->currentData();
    if (client.canConvert<GenericView *>()) {
        clientID = client.value<GenericView *>()->ID;
    }

    // Get the selected project color from stylesheet
    QString colorCode = ui->colorButton->styleSheet()
                        .replace("font-size:72px;color:", "")
                        .replace(";", "");

    QString projectGUID = TogglApi::instance->addProject(guid,
                          workspaceID,
                          clientID,
                          "",
                          ui->newProjectName->text(),
                          !ui->publicProject->isChecked(),
                          colorCode);

    ui->project->setVisible(true);

    return !projectGUID.isEmpty();
}

bool TimeEntryEditorWidget::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::FocusOut) {
        if (object == ui->description && !guid.isEmpty()) {
            TogglApi::instance->setTimeEntryDescription(guid,
                    ui->description->currentText());
        }

        if (object == ui->project && ui->project->currentText().length() == 0) {
            TogglApi::instance->setTimeEntryProject(guid,
                                                    0,
                                                    0,
                                                    "");
        }
    }

    return false;
}

void TimeEntryEditorWidget::on_deleteButton_clicked() {
    deleteTimeEntry();
}

void TimeEntryEditorWidget::on_addNewProject_clicked() {
    bool hasMultipleWorkspaces = ui->newProjectWorkspace->count() > 1;
    ui->newProjectWorkspace->setVisible(hasMultipleWorkspaces);
    ui->newProjectWorkspaceLabel->setVisible(hasMultipleWorkspaces);

    ui->newProject->setVisible(true);
    ui->addNewProject->setVisible(false);
    ui->project->setVisible(false);

    if (!hasMultipleWorkspaces) {
        ui->newProjectWorkspace->setCurrentIndex(0);
    }
}

void TimeEntryEditorWidget::on_newProjectWorkspace_currentIndexChanged(
    int index) {
    Q_UNUSED(index);
    displayClientSelect(clientSelectUpdate);
}

void TimeEntryEditorWidget::on_description_activated(int index) {
    AutocompleteView *view = ui->description->currentView();
    if (view) {
        ui->description->setEditText(QString::fromStdString(view->Description));
        ui->project->setFocus();
        ui->description->setFocus();
        TogglApi::instance->setTimeEntryProject(guid,
                                                view->TaskID,
                                                view->ProjectID,
                                                "");
        if (view->Billable) {
            TogglApi::instance->setTimeEntryBillable(guid, view->Billable);
        }

        if (!view->Tags.empty() && ui->tags->count() > 0) {
            bool tagsSet = false;
            for (int i = 0; i < ui->tags->count(); i++) {
                QListWidgetItem *widgetItem = ui->tags->item(i);
                if (widgetItem->checkState() == Qt::Checked) {
                    tagsSet = true;
                    break;
                }
            }
            if (!tagsSet) {
                TogglApi::instance->setTimeEntryTags(guid, QString::fromStdString(view->Tags));
            }
        }
    }
}

void TimeEntryEditorWidget::on_project_activated(int index) {
    Q_UNUSED(index);
    AutocompleteView *view = ui->project->currentView();
    if (view) {
        TogglApi::instance->setTimeEntryProject(guid,
                                                view->TaskID,
                                                view->ProjectID,
                                                "");
    }
}

void TimeEntryEditorWidget::on_duration_editingFinished() {
    TogglApi::instance->setTimeEntryDuration(guid,
            ui->duration->text());
}

void TimeEntryEditorWidget::on_start_editingFinished() {
    TogglApi::instance->setTimeEntryStart(guid,
                                          ui->start->text());
}

void TimeEntryEditorWidget::on_stop_editingFinished() {
    TogglApi::instance->setTimeEntryStop(guid,
                                         ui->stop->text());
}

void TimeEntryEditorWidget::on_dateEdit_editingFinished() {
    TogglApi::instance->setTimeEntryDate(guid,
                                         ui->dateEdit->dateTime().toTime_t());
}

void TimeEntryEditorWidget::displayTags(
    QVector<GenericView*> tags) {
    ui->tags->clear();
    QStringList tagList;
    foreach(GenericView *view, tags) {
        tagList << QString::fromStdString(view->Name);
    }

    QSet<QString> actuallyAddedTags;
for (auto recentlyAddedTag : recentlyAddedTags) {
        if (!recentlyAddedTag.isEmpty() && !tagList.contains(recentlyAddedTag)) {
            tagList << recentlyAddedTag;
        }
        if (!recentlyAddedTag.isEmpty() && tagList.contains(recentlyAddedTag)) {
            actuallyAddedTags.insert(recentlyAddedTag);
        }
    }
    tagList.sort();
    recentlyAddedTags = recentlyAddedTags - actuallyAddedTags;

for(auto tag : tagList) {
        QListWidgetItem *item = new QListWidgetItem(tag, ui->tags);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
}

void TimeEntryEditorWidget::on_billable_clicked(bool checked) {
    TogglApi::instance->setTimeEntryBillable(guid, checked);
}

void TimeEntryEditorWidget::timeout() {
    if (duration < 0 &&
            ui->duration->isVisible() &&
            !ui->duration->hasFocus()) {
        ui->duration->setText(
            TogglApi::formatDurationInSecondsHHMMSS(duration));
    }
}

void TimeEntryEditorWidget::toggleNewTagMode(bool visible) {
    ui->addNewTagButton->setVisible(!visible);
    ui->newTagLabel->setVisible(visible);
    ui->newTagButton->setVisible(visible);
    ui->newTag->setVisible(visible);
    if (visible)
        ui->newTag->setFocus();
    else
        ui->addNewTagButton->setFocus();
}

void TimeEntryEditorWidget::on_tags_itemClicked(QListWidgetItem *item) {
    Q_UNUSED(item);
    QStringList tags;
    for (int i = 0; i < ui->tags->count(); i++) {
        QListWidgetItem *widgetItem = ui->tags->item(i);
        if (widgetItem->checkState() == Qt::Checked) {
            tags.push_back(widgetItem->text());
        }
    }
    if (item) {
        tags.push_back(item->text());
    }
    tags.sort();
    QString list = tags.join("\t");

    if (previousTagList != list) {
        TogglApi::instance->setTimeEntryTags(guid, list);
    }
}

void TimeEntryEditorWidget::toggleNewClientMode(const bool visible) {
    // First hide stuff, to avoid expanding
    ui->cancelNewClient->setVisible(false);
    ui->addNewClient->setVisible(false);
    ui->newProjectClient->setVisible(false);
    ui->newClientName->setVisible(false);
    ui->addClientButton->setVisible(false);

    // No display whats needed
    ui->cancelNewClient->setVisible(visible);
    ui->addNewClient->setVisible(!visible);
    ui->newProjectClient->setVisible(!visible);
    ui->newClientName->setVisible(visible);
    ui->addClientButton->setVisible(visible);

    ui->newProjectClient->setFocus();
    ui->newClientName->setFocus();

    ui->newClientName->setText("");
}

void TimeEntryEditorWidget::on_addNewClient_clicked() {
    toggleNewClientMode(true);
}

void TimeEntryEditorWidget::on_addClientButton_clicked() {
    QString name = ui->newClientName->text();
    if (name.isEmpty()) {
        ui->newClientName->setFocus();
        return;
    }
    QVariant workspace = ui->newProjectWorkspace->currentData();
    if (!workspace.canConvert<GenericView *>()) {
        ui->newProjectWorkspace->setFocus();
        return;
    }
    uint64_t wid = workspace.value<GenericView *>()->ID;
    if (!wid) {
        ui->newProjectWorkspace->setFocus();
        return;
    }
    QString clientGUID = TogglApi::instance->createClient(wid, name);
    if (clientGUID.isEmpty()) {
        return;
    }
    toggleNewClientMode(false);
}

void TimeEntryEditorWidget::on_cancelNewClient_clicked() {
    toggleNewClientMode(false);
}

void TimeEntryEditorWidget::on_colorButton_clicked()
{
    int newX = window()->pos().x() + window()->width() - colorPicker->width() + 5;
    int newY = window()->pos().y() + ui->newProject->pos().y()+80;

    colorPicker->move(newX, newY);
    colorPicker->show();
}

void TimeEntryEditorWidget::on_newTagButton_clicked() {
    QStringList tags;
    QStringList allTags;
    QString newTag = ui->newTag->text();

    if (!newTag.isEmpty()) {
        ui->newTag->clear();
        for (int i = 0; i < ui->tags->count(); i++) {
            QListWidgetItem *widgetItem = ui->tags->item(i);
            if (widgetItem->text() == newTag) {
                if (widgetItem->checkState() != Qt::Checked)
                    on_tags_itemClicked(widgetItem);
                return;
            }
            allTags << widgetItem->text();
            if (widgetItem->checkState() == Qt::Checked) {
                tags.push_back(widgetItem->text());
            }
        }
        tags.push_back(newTag);
        tags.sort();
        allTags.push_back(newTag);
        allTags.sort();

        QString list = tags.join("\t");
        if (previousTagList != list) {
            TogglApi::instance->setTimeEntryTags(guid, list);
            recentlyAddedTags.insert(newTag);
        }
        previousTagList = list;

        ui->tags->clear();
        for (int i = 0; i < allTags.count(); i++) {
            auto item = new QListWidgetItem(allTags[i], ui->tags);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            if (tags.contains(allTags[i]))
                item->setCheckState(Qt::Checked);
            else
                item->setCheckState(Qt::Unchecked);
            ui->tags->addItem(item);
        }
    }

    QTimer::singleShot(0, [this]() {
        toggleNewTagMode(false);
    });
}

void TimeEntryEditorWidget::on_newTag_returnPressed() {
    on_newTagButton_clicked();
}

void TimeEntryEditorWidget::on_addNewTagButton_clicked() {
    toggleNewTagMode(true);
}

void TimeEntryEditorWidget::setProjectColors(QVector<char *> list) {
    colorPicker->setColors(list);
}

