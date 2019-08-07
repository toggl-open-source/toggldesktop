
#include "activity_manager.h"

namespace toggl {

toggl::ActivityManager::ActivityManager(toggl::Context *context)
{

}

void toggl::ActivityManager::stopAll() {
    /*
    {
        Poco::Mutex::ScopedLock lock(syncer_m_);
        if (syncer_.isRunning()) {
            syncer_.stop();
            syncer_.wait(2000);
        }
    }
    */
}

SyncerActivity *toggl::ActivityManager::syncer() {
    return syncer_;
}

};
