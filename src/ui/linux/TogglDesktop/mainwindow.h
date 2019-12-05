#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionAbout_triggered();
    void on_actionHelp_triggered();
    void on_actionUpdateCheck_triggered();
    void on_actionPreferences_triggered();
    void on_actionQuit_triggered();

    void on_actionNew_triggered();
    void on_actionContinue_triggered();
    void on_actionStop_triggered();
    void on_actionShow_triggered();
    void on_actionEdit_triggered();

    void on_actionSync_triggered();
    void on_actionClearCache_triggered();
    void on_actionLogout_triggered();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
