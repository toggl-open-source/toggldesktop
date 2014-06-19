#ifndef AUTOCOMPLETEVIEW_H
#define AUTOCOMPLETEVIEW_H

#include <QObject>
#include <QVector>

#include "kopsik_api.h"

class AutocompleteView : public QObject
{
    Q_OBJECT
public:
    explicit AutocompleteView(QObject *parent = 0);

    static QVector<AutocompleteView *> importAll(KopsikAutocompleteItem *first) {
        QVector<AutocompleteView *> result;
        KopsikAutocompleteItem *it = first;
        while (it) {
            AutocompleteView *view = new AutocompleteView();
            view->Text = QString(it->Text);
            view->Description = QString(it->Description);
            view->ProjectAndTaskLabel = QString(it->ProjectAndTaskLabel);
            view->ProjectLabel = QString(it->ProjectLabel);
            view->ClientLabel = QString(it->ClientLabel);
            view->ProjectColor = QString(it->ProjectColor);
            view->TaskID = it->TaskID;
            view->ProjectID = it->ProjectID;
            view->Type = it->Type;
            result.push_back(view);
            it = static_cast<KopsikAutocompleteItem *>(it->Next);
        }
        return result;
    }

    QString Text;
    QString Description;
    QString ProjectAndTaskLabel;
    QString ProjectLabel;
    QString ClientLabel;
    QString ProjectColor;
    uint64_t TaskID;
    uint64_t ProjectID;
    uint64_t Type;


signals:

public slots:

};

#endif // AUTOCOMPLETEVIEW_H
