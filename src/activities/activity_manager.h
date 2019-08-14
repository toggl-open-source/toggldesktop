#ifndef SRC_ACTIVITY_MANAGER_H
#define SRC_ACTIVITY_MANAGER_H

#include "activity.h"
#include "pull_activity.h"
#include "push_activity.h"
#include "reminder_activity.h"
#include "uiupdater_activity.h"

#include <map>
#include <thread>
#include <mutex>

namespace toggl {

class Context;

class ActivityManager {
public:
    ActivityManager(Context *context);

    enum ActivityType {
        PULL,
        PUSH,
        REMINDER,
        UI_UPDATER
    };

    static int64_t now();
    void schedule(ActivityType activity, int64_t at = now());

    void start(ActivityType activity);
    void stop(ActivityType activity);
    void restart(ActivityType activity);
    void stopAll();

    Context *context();

private:
    std::recursive_mutex mutex_;
    typedef std::unique_lock<std::recursive_mutex> lock_type;

    Context *context_ { nullptr };

    std::map<int64_t, Activity*> events_;
    std::map<ActivityType, Activity*> activities_ {
        { PULL, new PullActivity { this } },
        { PUSH, new PushActivity { this } },
        { REMINDER, new ReminderActivity { this } },
        { UI_UPDATER, new UiUpdaterActivity { this } }
    };
};

}

#endif // SRC_ACTIVITY_MANAGER_H
