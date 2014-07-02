#ifndef GENERICVIEW_H
#define GENERICVIEW_H

#include <QObject>
#include <QVector>

#include "kopsik_api.h"

class GenericView : public QObject
{
    Q_OBJECT
 public:
    explicit GenericView(QObject *parent = 0);

    static QVector<GenericView *> importAll(KopsikViewItem *first) {
        QVector<GenericView *> result;
        KopsikViewItem *it = first;
        while (it) {
            GenericView *view = new GenericView();
            view->ID = it->ID;
            view->WID = it->WID;
            view->GUID = QString(it->GUID);
            view->Name = QString(it->Name);
            result.push_back(view);
            it = static_cast<KopsikViewItem *>(it->Next);
        }
        return result;
    }

    uint64_t ID;
    uint64_t WID;
    QString GUID;
    QString Name;

 signals:

 public slots:

};

#endif // GENERICVIEW_H
