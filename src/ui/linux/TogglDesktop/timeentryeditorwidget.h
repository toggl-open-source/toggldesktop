// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYEDITORWIDGET_H_
#define SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYEDITORWIDGET_H_

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QListWidgetItem>
#include <QStackedWidget>

#include <stdint.h>
#include "./colorpicker.h"

namespace Ui {
class TimeEntryEditorWidget;
}

class AutocompleteView;
class AutocompleteListModel;
class GenericView;
class TimeEntryView;

class TimeEntryEditorWidget : public QWidget {
    Q_OBJECT

 public:
    explicit TimeEntryEditorWidget(QStackedWidget *parent = nullptr);
    ~TimeEntryEditorWidget();
    void setSelectedColor(QString color);

    void display();

 private:
    Ui::TimeEntryEditorWidget *ui;

    QString guid;

    QVector<AutocompleteView *> timeEntryAutocompleteUpdate;
    bool timeEntryAutocompleteNeedsUpdate;

    QVector<AutocompleteView *> projectAutocompleteUpdate;
    bool projectAutocompleteNeedsUpdate;

    QVector<GenericView *> workspaceSelectUpdate;
    bool workspaceSelectNeedsUpdate;

    QVector<GenericView *> clientSelectUpdate;
    bool clientSelectNeedsUpdate;

    ColorPicker *colorPicker;

    QTimer *timer;

    int64_t duration;

    QString previousTagList;

    bool confirmlessDelete;

    AutocompleteListModel *descriptionModel;
    AutocompleteListModel *projectModel;

    bool applyNewProject();
    bool eventFilter(QObject *object, QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void toggleNewClientMode(const bool visible);

 private slots:  // NOLINT
    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void aboutToDisplayTimeEntryList();

    void displayTimeEntryEditor(
        const bool open,
        TimeEntryView *view,
        const QString focused_field_name);

    void displayTags(
        QVector<GenericView*> list);

    void displayWorkspaceSelect(
        QVector<GenericView *> list);

    void displayClientSelect(
        QVector<GenericView *> list);

    void displayTimeEntryAutocomplete(
        QVector<AutocompleteView *> list);

    void displayProjectAutocomplete(
        QVector<AutocompleteView *> list);

    void setProjectColors(QVector<char *> list);

    void timeout();

    void on_doneButton_clicked();
    void on_deleteButton_clicked();
    void on_addNewProject_clicked();
    void on_newProjectWorkspace_currentIndexChanged(int index);
    void on_description_currentIndexChanged(int index);
    void on_description_activated(const QString &arg1);
    void on_project_activated(int index);
    void on_duration_editingFinished();
    void on_start_editingFinished();
    void on_stop_editingFinished();
    void on_dateEdit_editingFinished();
    void on_billable_clicked(bool checked);
    void on_tags_itemClicked(QListWidgetItem *item);
    void on_addNewClient_clicked();
    void on_addClientButton_clicked();
    void on_cancelNewClient_clicked();
    void on_colorButton_clicked();
};

#endif  // SRC_UI_LINUX_TOGGLDESKTOP_TIMEENTRYEDITORWIDGET_H_
