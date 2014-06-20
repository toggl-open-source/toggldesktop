#ifndef TIMEENTRYEDITORWIDGET_H
#define TIMEENTRYEDITORWIDGET_H

#include <QWidget>
#include <QVector>

#include <stdint.h>

#include "timeentryview.h"
#include "genericview.h"
#include "autocompleteview.h"

namespace Ui {
class TimeEntryEditorWidget;
}

class TimeEntryEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimeEntryEditorWidget(QWidget *parent = 0);
    ~TimeEntryEditorWidget();

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

private slots:
    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayTimeEntryList(
        const bool open,
        QVector<TimeEntryView *> list);

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

    void on_doneButton_clicked();

    void on_deleteButton_clicked();
    void on_addNewProject_linkActivated(const QString &link);
    void on_timeOverview_linkActivated(const QString &link);
};

#endif // TIMEENTRYEDITORWIDGET_H
