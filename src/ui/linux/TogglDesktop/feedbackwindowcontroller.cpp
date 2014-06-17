#include "feedbackwindowcontroller.h"
#include "ui_feedbackwindowcontroller.h"

FeedbackWindowController::FeedbackWindowController(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FeedbackWindowController)
{
    ui->setupUi(this);
}

FeedbackWindowController::~FeedbackWindowController()
{
    delete ui;
}
