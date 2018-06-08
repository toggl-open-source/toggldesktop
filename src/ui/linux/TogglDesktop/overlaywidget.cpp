#include "overlaywidget.h"
#include "ui_overlaywidget.h"
#include "./toggl.h"

OverlayWidget::OverlayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverlayWidget)
{
    ui->setupUi(this);
    setVisible(false);

    ui->bottomText->setCursor(Qt::PointingHandCursor);

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayTimeEntryList(bool,QVector<TimeEntryView*>,bool)),  // NOLINT
            this, SLOT(displayTimeEntryList(bool,QVector<TimeEntryView*>,bool)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayOverlay(int64_t)),  // NOLINT
            this, SLOT(displayOverlay(int64_t)));  // NOLINT
}

OverlayWidget::~OverlayWidget()
{
    delete ui;
}

void OverlayWidget::displayOverlay(const int64_t type) {
    setVisible(true);
}

void OverlayWidget::displayLogin(
    const bool open,
    const uint64_t user_id) {

    if (open || user_id) {
        setVisible(false);
    }
}

void OverlayWidget::displayTimeEntryList(
    const bool open,
    QVector<TimeEntryView *> list,
    const bool) {
    if (open) {
        setVisible(false);
    }
}

void OverlayWidget::on_loginButton_clicked()
{
    TogglApi::instance->openInBrowser();
}

void OverlayWidget::on_bottomText_linkActivated(const QString &link)
{
    TogglApi::instance->fullSync();
}
