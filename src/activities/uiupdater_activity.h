#ifndef SRC_UI_UPDATER_ACTIVITY_H
#define SRC_UI_UPDATER_ACTIVITY_H

#include "activity.h"

#include <Poco/Activity.h>

namespace toggl {

class Context;

class UiUpdaterActivity : public toggl::Activity {
public:
    UiUpdaterActivity(ActivityManager *parent);

    void work() override;

    void uiUpdaterActivity();

private:
    Poco::Mutex ui_updater_m_;
};

}

#endif // SRC_UI_UPDATER_ACTIVITY_H
