#include "activity.h"
#include "activity_manager.h"

namespace toggl {

Activity::Activity(ActivityManager *parent)
    : activity_(this, &Activity::work)
    , manager_(parent)
{

}

Context *Activity::context() {
    return manager_->context();
}

void Activity::stop() {
    activity_.stop();
}

void Activity::start() {
    activity_.start();
}

void Activity::restart() {
    if (activity_.isRunning())
        activity_.stop();
    activity_.start();
}

};
