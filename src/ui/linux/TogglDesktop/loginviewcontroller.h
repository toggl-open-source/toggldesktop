#ifndef LOGINVIEWCONTROLLER_H
#define LOGINVIEWCONTROLLER_H

#include <QWidget>

namespace Ui {
class LoginViewController;
}

class LoginViewController : public QWidget
{
    Q_OBJECT

public:
    explicit LoginViewController(QWidget *parent = 0);
    ~LoginViewController();

private:
    Ui::LoginViewController *ui;
};

#endif // LOGINVIEWCONTROLLER_H
