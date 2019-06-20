#include "countryview.h"

CountryView::CountryView(QObject *parent, const Country *view)
    : QObject(parent)
    , toggl::view::Country(*view) {
}
