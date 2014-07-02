#ifndef UPDATEVIEW_H
#define UPDATEVIEW_H

#include <QObject>

#include "kopsik_api.h"

class UpdateView : public QObject
{
    Q_OBJECT
 public:
    explicit UpdateView(QObject *parent = 0);

    static UpdateView *importOne(KopsikUpdateViewItem *view) {
        UpdateView *result = new UpdateView();
        result->UpdateChannel = QString(view->UpdateChannel);
        result->IsChecking = view->IsChecking;
        result->IsUpdateAvailable = view->IsUpdateAvailable;
        result->URL = QString(view->URL);
        result->Version = QString(view->Version);
        return result;
    }

    QString UpdateChannel;
    bool IsChecking;
    bool IsUpdateAvailable;
    QString URL;
    QString Version;

 signals:

 public slots:

};

#endif // UPDATEVIEW_H
