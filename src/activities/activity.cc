#include "activity.h"

namespace toggl {

Activity::Activity(toggl::Context *context)
    : activity_(this, &Activity::work)
    , context_(context)
{

}

void Activity::start() {
    activity_.start();
}

};
