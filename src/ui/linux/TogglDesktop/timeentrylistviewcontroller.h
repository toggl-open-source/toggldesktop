#ifndef TIMEENTRYLISTVIEWCONTROLLER_H
#define TIMEENTRYLISTVIEWCONTROLLER_H

#include <QWidget>

namespace Ui {
class TimeEntryListViewController;
}

class TimeEntryListViewController : public QWidget
{
    Q_OBJECT

public:
    explicit TimeEntryListViewController(QWidget *parent = 0);
    ~TimeEntryListViewController();

private:
    Ui::TimeEntryListViewController *ui;
};

#endif // TIMEENTRYLISTVIEWCONTROLLER_H
