#ifndef DESKTOPSERVICES_H
#define DESKTOPSERVICES_H

#include <QUrl>

#include "clipboarddialog.h"

class DesktopServices : public QObject
{
    Q_OBJECT
public:
    static void init(QWidget *parent = nullptr);
    static ClipboardDialog *dialog();
    static DesktopServices *instance();

public slots:
    // IMPORTANT:
    // It's necessary to use this function as a slot or with QMetaObject::invokeMethod
    // because the dialog can live in a different thread than the caller (which is bad times)
    void openUrl(const QUrl &url);

private:
    DesktopServices(QObject *parent = nullptr);
    inline static ClipboardDialog *m_dialog { nullptr };

};

#endif // DESKTOPSERVICES_H
