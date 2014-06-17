#ifndef TIMEREDITVIEWCONTROLLER_H
#define TIMEREDITVIEWCONTROLLER_H

#include <QWidget>

namespace Ui {
class TimerEditViewController;
}

class TimerEditViewController : public QWidget
{
    Q_OBJECT

public:
    explicit TimerEditViewController(QWidget *parent = 0);
    ~TimerEditViewController();

private:
    Ui::TimerEditViewController *ui;
};

#endif // TIMEREDITVIEWCONTROLLER_H
