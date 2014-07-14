// Copyright 2014 Tanel Lebedev.

#include "./bugsnag.h"

QString Bugsnag::releaseStage = QString("production");
QStringList Bugsnag::notifyReleaseStages =
    QStringList() << "production" << "development";
bool Bugsnag::autoNotify = true;
bool Bugsnag::useSSL = true;
