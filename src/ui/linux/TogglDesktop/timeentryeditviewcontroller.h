#ifndef TIMEENTRYEDITVIEWCONTROLLER_H
#define TIMEENTRYEDITVIEWCONTROLLER_H

#include <QWidget>

namespace Ui {
class TimeEntryEditViewController;
}

class TimeEntryEditViewController : public QWidget
{
    Q_OBJECT

public:
    explicit TimeEntryEditViewController(QWidget *parent = 0);
    ~TimeEntryEditViewController();

private:
    Ui::TimeEntryEditViewController *ui;
};

#endif // TIMEENTRYEDITVIEWCONTROLLER_H
