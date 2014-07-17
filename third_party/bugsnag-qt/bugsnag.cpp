// Copyright 2014 Tanel Lebedev.

#include "./bugsnag.h"

QString Bugsnag::apiKey = QString("");
QString Bugsnag::releaseStage = QString("production");
QStringList Bugsnag::notifyReleaseStages =
    QStringList() << "production" << "development";
bool Bugsnag::autoNotify = true;
bool Bugsnag::useSSL = true;

Device Bugsnag::device;
App Bugsnag::app;
User Bugsnag::user;

Bugsnag Bugsnag::instance;
