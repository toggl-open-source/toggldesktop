#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QVector>

#include <stdint.h>

#include "timeentryview.h"

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = 0);
    ~LoginWidget();

private slots:
    void on_login_clicked();

    void displayLogin(
        const bool open,
        const uint64_t user_id);

    void displayTimeEntryList(
        const bool open,
        QVector<TimeEntryView *> list);

private:
    Ui::LoginWidget *ui;
};

#endif // LOGINWIDGET_H
