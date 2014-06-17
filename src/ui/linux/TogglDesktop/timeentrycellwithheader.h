#ifndef TIMEENTRYCELLWITHHEADER_H
#define TIMEENTRYCELLWITHHEADER_H

#include <QWidget>

namespace Ui {
class TimeEntryCellWithHeader;
}

class TimeEntryCellWithHeader : public QWidget
{
    Q_OBJECT

public:
    explicit TimeEntryCellWithHeader(QWidget *parent = 0);
    ~TimeEntryCellWithHeader();

private:
    Ui::TimeEntryCellWithHeader *ui;
};

#endif // TIMEENTRYCELLWITHHEADER_H
