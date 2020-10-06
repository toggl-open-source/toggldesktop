#include "desktopservices.h"

#include <QDesktopServices>
#include <QDialog>
#include <QHBoxLayout>
#include <QLineEdit>

#include "clipboarddialog.h"

void DesktopServices::init(QWidget *parent) {
    (void) instance();
    if (m_dialog)
        m_dialog->deleteLater();
    QDesktopServices::setUrlHandler("http", instance(), "openUrl");
    QDesktopServices::setUrlHandler("https", instance(), "openUrl");
    m_dialog = new ClipboardDialog(parent);
}

ClipboardDialog *DesktopServices::dialog() {
    if (!m_dialog)
        init();
    return m_dialog;
}

DesktopServices *DesktopServices::instance() {
    static DesktopServices self = DesktopServices();
    return &self;
}

void DesktopServices::openUrl(const QUrl &url) {
    if (!QDesktopServices::openUrl(url)) {
        dialog()->showUrl(url);
    }
}

DesktopServices::DesktopServices(QObject *parent) : QObject(parent) { }
