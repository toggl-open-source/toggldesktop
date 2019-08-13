#ifndef SRC_ACTIVITY_MANAGER_H
#define SRC_ACTIVITY_MANAGER_H

#include "activity.h"
#include "syncer_activity.h"
#include "reminder_activity.h"
#include "uiupdater_activity.h"

#include <map>

namespace toggl {

class Context;

class ActivityManager {
public:
    ActivityManager(Context *context);

    void stopAll();

    ReminderActivity *reminder();
    SyncerActivity *syncer();
    UiUpdaterActivity *uiUpdater();

private:
    Context *context_ { nullptr };

    std::map<int64_t, Activity*> events_;

    SyncerActivity *syncer_ { nullptr };
    ReminderActivity *reminder_ { nullptr };
    UiUpdaterActivity *uiUpdater_ { nullptr };
};

}

#endif // SRC_ACTIVITY_MANAGER_H
