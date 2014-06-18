#ifndef ERRORVIEWCONTROLLER_H
#define ERRORVIEWCONTROLLER_H

#include <QWidget>

namespace Ui {
class ErrorViewController;
}

class ErrorViewController : public QWidget
{
    Q_OBJECT

public:
    explicit ErrorViewController(QWidget *parent = 0);
    ~ErrorViewController();

private:
    Ui::ErrorViewController *ui;
};

#endif // ERRORVIEWCONTROLLER_H
