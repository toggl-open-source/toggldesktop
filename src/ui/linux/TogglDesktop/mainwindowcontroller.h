#ifndef MAINWINDOWCONTROLLER_H
#define MAINWINDOWCONTROLLER_H

#include <QMainWindow>
#include <QActionGroup>

#include <stdbool.h>
#include <stdint.h>

#include "toggl_api.h"
#include "loginwidget.h"
#include "timeentrylistwidget.h"

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
    void displayApp(
        const bool open);

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void onAction(QAction *action);

private:
    Ui::MainWindowController *ui;
    bool shutdown;
    TogglApi *togglApi;
    QActionGroup *menuActions;

    void readSettings();
    void writeSettings();

    void connectMenuActions();
};

#endif // MAINWINDOWCONTROLLER_H
