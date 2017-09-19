#ifndef MISSINGWSWIDGET_H
#define MISSINGWSWIDGET_H

#include <QWidget>
#include <stdint.h>
#include "./timeentryview.h"

namespace Ui {
class MissingWSWidget;
}

class MissingWSWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MissingWSWidget(QWidget *parent = 0);
    ~MissingWSWidget();

private:
    Ui::MissingWSWidget *ui;

private slots:  // NOLINT

   void displayWSError();

   void displayLogin(
       const bool open,
       const uint64_t user_id);

   void displayTimeEntryList(
       const bool open,
       QVector<TimeEntryView *> list,
       const bool show_load_more_button);

   void on_loginButton_clicked();
   void on_bottomText_linkActivated(const QString &link);
};


#endif // MISSINGWSWIDGET_H
