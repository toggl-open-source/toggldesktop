#ifndef SRC_UI_UPDATER_ACTIVITY_H
#define SRC_UI_UPDATER_ACTIVITY_H

#include "activity.h"

#include <Poco/Activity.h>

namespace toggl {

class Context;

class UiUpdaterActivity : public toggl::Activity {
public:
    void work() override;
    UiUpdaterActivity(Context *context);

    void uiUpdaterActivity();

private:
    Context *context_ { nullptr };

    Poco::Mutex ui_updater_m_;
    Poco::Activity<UiUpdaterActivity> ui_updater_;
};

}

#endif // SRC_UI_UPDATER_ACTIVITY_H
