#ifndef SRC_ACTIVITY_MANAGER_H
#define SRC_ACTIVITY_MANAGER_H

#include "activity.h"
#include "syncer_activity.h"

#include <map>

namespace toggl {

class Context;

class ActivityManager {
public:
    ActivityManager(Context *context);

    void stopAll();

    SyncerActivity *syncer();

private:
    Context *context_ { nullptr };

    std::map<int64_t, Activity*> events_;

    SyncerActivity *syncer_ { nullptr };
};

};

#endif // SRC_ACTIVITY_MANAGER_H
