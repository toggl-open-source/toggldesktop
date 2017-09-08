#include "missingwswidget.h"
#include "ui_missingwswidget.h"
#include "./toggl.h"

MissingWSWidget::MissingWSWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MissingWSWidget)
{
    ui->setupUi(this);
    setVisible(false);

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTimeEntryList(bool,QVector<TimeEntryView*>,bool)),  // NOLINT
            this, SLOT(displayTimeEntryList(bool,QVector<TimeEntryView*>,bool)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayWSError()),  // NOLINT
            this, SLOT(displayWSError()));  // NOLINT
}

MissingWSWidget::~MissingWSWidget()
{
    delete ui;
}

void MissingWSWidget::displayWSError() {
    setVisible(true);
}

void MissingWSWidget::displayLogin(
    const bool open,
    const uint64_t user_id) {

    if (open || user_id) {
        setVisible(false);
    }
}

void MissingWSWidget::displayTimeEntryList(
    const bool open,
    QVector<TimeEntryView *> list,
    const bool) {
    if (open) {
        setVisible(false);
    }
}

void MissingWSWidget::on_loginButton_clicked()
{
    TogglApi::instance->openInBrowser();

}

void MissingWSWidget::on_bottomText_linkActivated(const QString &link)
{
    TogglApi::instance->fullSync();

}
