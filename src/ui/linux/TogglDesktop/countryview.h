#ifndef COUNTRYVIEW_H
#define COUNTRYVIEW_H

#include <QObject>
#include <QVector>

#include "./gui.h"
#include "./toggl_api.h"

class CountryView : public QObject, public toggl::view::Country
{
    Q_OBJECT

public:
    explicit CountryView(QObject *parent = nullptr, const toggl::view::Country *view = nullptr);

    static QVector<CountryView *> importAll(const TogglCountryView *first) {
        QVector<CountryView *> result;
        const TogglCountryView *it = first;
        while (it) {
            const toggl::view::Country *v = reinterpret_cast<const toggl::view::Country*>(it);
            auto sv = new CountryView(nullptr, v);
            result.push_back(sv);
            it = TogglCountryView_Next(it);
        }
        return result;
    }
};

#endif // COUNTRYVIEW_H
