#ifndef SETTINGSWINDOWCONTROLLER_H
#define SETTINGSWINDOWCONTROLLER_H

#include <QDialog>

namespace Ui {
class SettingsWindowController;
}

class SettingsWindowController : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindowController(QWidget *parent = 0);
    ~SettingsWindowController();

private:
    Ui::SettingsWindowController *ui;
};

#endif // SETTINGSWINDOWCONTROLLER_H
