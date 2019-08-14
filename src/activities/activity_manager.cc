
#include "activity_manager.h"

namespace toggl {

toggl::ActivityManager::ActivityManager(toggl::Context *context)
    : context_(context)
{

}

void ActivityManager::schedule(ActivityType activity, int64_t at) {
    lock_type l(mutex_);

}

void ActivityManager::start(ActivityManager::ActivityType activity) {
    lock_type l(mutex_);
    activities_[activity]->start();
}

void ActivityManager::stop(ActivityManager::ActivityType activity) {
    lock_type l(mutex_);
    activities_[activity]->stop();
}

void ActivityManager::restart(ActivityManager::ActivityType activity) {
    lock_type l(mutex_);
    activities_[activity]->restart();
}

int64_t ActivityManager::now() {

}

void toggl::ActivityManager::stopAll() {
    lock_type l(mutex_);
    /*

        {
            Poco::Mutex::ScopedLock lock(reminder_m_);
            if (reminder_.isRunning()) {
                reminder_.stop();
                reminder_.wait(2000);
            }
        }

        {
            Poco::Mutex::ScopedLock lock(ui_updater_m_);
            if (ui_updater_.isRunning()) {
                ui_updater_.stop();
                ui_updater_.wait(2000);
            }
        }
    {
        Poco::Mutex::ScopedLock lock(syncer_m_);
        if (syncer_.isRunning()) {
            syncer_.stop();
            syncer_.wait(2000);
        }
    }
    */
}

Context *ActivityManager::context() {
    return context_;
}

};
