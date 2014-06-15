#ifndef MAINWINDOWCONTROLLER_H
#define MAINWINDOWCONTROLLER_H

#include <QMainWindow>

namespace Ui {
class MainWindowController;
}

class MainWindowController : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowController(QWidget *parent = 0);
    ~MainWindowController();

private:
    Ui::MainWindowController *ui;
    void *ctx_;
};

#endif // MAINWINDOWCONTROLLER_H
