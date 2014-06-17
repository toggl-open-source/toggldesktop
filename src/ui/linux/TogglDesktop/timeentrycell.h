#ifndef TIMEENTRYCELL_H
#define TIMEENTRYCELL_H

#include <QWidget>

namespace Ui {
class TimeEntryCell;
}

class TimeEntryCell : public QWidget
{
    Q_OBJECT

public:
    explicit TimeEntryCell(QWidget *parent = 0);
    ~TimeEntryCell();

private:
    Ui::TimeEntryCell *ui;
};

#endif // TIMEENTRYCELL_H
