#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QWidget>
#include <stdint.h>
#include "./timeentryview.h"

namespace Ui {
class OverlayWidget;
}

class OverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OverlayWidget(QWidget *parent = 0);
    ~OverlayWidget();

private:
    Ui::OverlayWidget *ui;
    int current_type;

private slots:  // NOLINT

   void displayOverlay(const int64_t type);

   void displayLogin(
       const bool open,
       const uint64_t user_id);

   void displayTimeEntryList(
       const bool open,
       QVector<TimeEntryView *> list,
       const bool show_load_more_button);

   void on_actionButton_clicked();
   void on_bottomText_linkActivated(const QString &link);
   void on_topText_linkActivated(const QString &link);
};


#endif // OVERLAYWIDGET_H
