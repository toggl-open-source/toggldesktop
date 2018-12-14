#include "overlaywidget.h"
#include "ui_overlaywidget.h"
#include "./toggl.h"

OverlayWidget::OverlayWidget(QStackedWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverlayWidget)
{
    ui->setupUi(this);
    current_type = -1;

    ui->bottomText->setCursor(Qt::PointingHandCursor);

    connect(TogglApi::instance, SIGNAL(displayLogin(bool,uint64_t)),  // NOLINT
            this, SLOT(displayLogin(bool,uint64_t)));  // NOLINT

    connect(TogglApi::instance, SIGNAL(displayOverlay(int64_t)),  // NOLINT
            this, SLOT(displayOverlay(int64_t)));  // NOLINT
}

OverlayWidget::~OverlayWidget()
{
    delete ui;
}

void OverlayWidget::display() {
    qobject_cast<QStackedWidget*>(parent())->setCurrentWidget(this);
}

void OverlayWidget::displayOverlay(const int64_t type) {
    current_type = type;
    QString top("");
    QString button("");
    QString bottom("");
    // setup overlay content
    if (type == 0) {
        //  missing ws
        top.append("You no longer have access to your last Workspace. Create a new workspace on Toggl.com to continue tracking");

        bottom.append("<html><head/><body><p align='center'>");
        bottom.append("Created your new workspace?<br/><a href='#force-sync'>");
        bottom.append("<span style='text-decoration: underline; color:#0000ff;'>");
        bottom.append("Force sync</span></a></p></body></html>");

        button.append("Log in to Toggl.com");
    } else if (type == 1) {
        // tos accept
        top.append("<h2>Our Terms of Service and Privacy Policy have changed</h2>");
        top.append("Please read and accept our updated ");
        top.append("<a href='terms'>");
        top.append("<span style='text-decoration: underline; color:#0000ff;'>");
        top.append("Terms of Service</span></a> and ");
        top.append("<a href='privacy'>");
        top.append("<span style='text-decoration: underline; color:#0000ff;'>");
        top.append("Privacy policy</span></a> to continuue using Toggl");
        bottom.append("If you have any questions, contact us at <span style='color:#0000ff;'>support@toggl.com</span>");
        button.append("I accept");
    }

    ui->topText->setText(top);
    ui->actionButton->setText(button);
    ui->bottomText->setText(bottom);
    display();
}

void OverlayWidget::displayLogin(
    const bool open,
    const uint64_t user_id) {

    if (open || user_id) {
    }
}

void OverlayWidget::on_actionButton_clicked()
{
    if (current_type == 0) {
        TogglApi::instance->openInBrowser();
    } else if (current_type == 1) {
        TogglApi::instance->tosAccept();
    }
}

void OverlayWidget::on_bottomText_linkActivated(const QString &link)
{
    Q_UNUSED(link);
    if (current_type == 0) {
        TogglApi::instance->fullSync();
    }
}

void OverlayWidget::on_topText_linkActivated(const QString &link)
{
    if (current_type == 1) {
        TogglApi::instance->openLegal(link);
    }
}
