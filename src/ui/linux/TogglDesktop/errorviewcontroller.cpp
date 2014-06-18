#include "errorviewcontroller.h"
#include "ui_errorviewcontroller.h"
#include "toggl_api.h"

ErrorViewController::ErrorViewController(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ErrorViewController)
{
    ui->setupUi(this);
    setVisible(false);

    connect(TogglApi::instance, SIGNAL(displayError(QString,bool)), this, SLOT(displayError(QString,bool)));
}

ErrorViewController::~ErrorViewController()
{
    delete ui;
}

void ErrorViewController::on_pushButton_clicked()
{
    setVisible(false);
}

void ErrorViewController::displayError(
    const QString errmsg,
    const bool user_error)
{
    ui->errorMessage->setText(errmsg);
    setVisible(true);
    if (!user_error) {
        // FIXME: notify bugsnag
    }

}

