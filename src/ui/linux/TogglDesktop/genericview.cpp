// Copyright 2014 Toggl Desktop developers.

#include "./genericview.h"

GenericView::GenericView(QObject *parent, const toggl::view::Generic *view)
    : QObject(parent)
    , toggl::view::Generic(*view) {
}
