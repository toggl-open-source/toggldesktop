#ifndef FEEDBACKWINDOWCONTROLLER_H
#define FEEDBACKWINDOWCONTROLLER_H

#include <QDialog>

namespace Ui {
class FeedbackWindowController;
}

class FeedbackWindowController : public QDialog
{
    Q_OBJECT

public:
    explicit FeedbackWindowController(QWidget *parent = 0);
    ~FeedbackWindowController();

private:
    Ui::FeedbackWindowController *ui;
};

#endif // FEEDBACKWINDOWCONTROLLER_H
