#ifndef MAINWINDOWCONTROLLER_H
#define MAINWINDOWCONTROLLER_H

#include <QMainWindow>
#include <stdbool.h>
#include <stdint.h>

#include "kopsik_api.h"

namespace Ui {
class MainWindowController;
}

class MainWindowController : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowController(QWidget *parent = 0);
    ~MainWindowController();

    static MainWindowController *Instance;

public slots:
    void onDisplayApp(const _Bool open);

    void onDisplayError(
        const QString errmsg,
        const _Bool user_error);

    void onDisplayUpdate(
        const _Bool open,
        KopsikUpdateViewItem *update);

    void onDisplayOnlineState(
        const _Bool is_online);

    void onDisplayUrl(
        const QString url);

    void onDisplayLogin(
        const _Bool open,
        const uint64_t user_id);

    void onDisplayReminder(
        const QString title,
        const QString informative_text);

    void onDisplayTimeEntryList(
        const _Bool open,
        KopsikTimeEntryViewItem *first);

    void onDisplayTimeEntryAutocomplete(
        KopsikAutocompleteItem *first);

    void onDisplayProjectAutocomplete(
        KopsikAutocompleteItem *first);

    void onDisplayWorkspaceSelect(
        KopsikViewItem *first);

    void onDisplayTimeEntryEditor(
        const _Bool open,
        KopsikTimeEntryViewItem *te,
        const QString focused_field_name);

    void onDisplaySettings(
        const _Bool open,
        KopsikSettingsViewItem *settings);

    void onDisplayTimerState(
        KopsikTimeEntryViewItem *te);

    void onDisplayIdleNotification(
        const QString since,
        const QString duration,
        const uint64_t started);

    void onDisplayClientSelect(
        KopsikViewItem *first);

    void onDisplayTags(
        KopsikViewItem *first);

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindowController *ui;
    void *ctx_;
    bool shutdown_;

    void readSettings();
    void writeSettings();
};

#endif // MAINWINDOWCONTROLLER_H
