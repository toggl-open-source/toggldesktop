#ifndef ABOUTWINDOWCONTROLLER_H
#define ABOUTWINDOWCONTROLLER_H

#include <QDialog>

namespace Ui {
class AboutWindowController;
}

class AboutWindowController : public QDialog
{
    Q_OBJECT

public:
    explicit AboutWindowController(QWidget *parent = 0);
    ~AboutWindowController();

private:
    Ui::AboutWindowController *ui;
};

#endif // ABOUTWINDOWCONTROLLER_H
