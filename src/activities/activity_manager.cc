
#include "activity_manager.h"

namespace toggl {

toggl::ActivityManager::ActivityManager(toggl::Context *context)
    : reminder_(new ReminderActivity(context))
    , uiUpdater_(new UiUpdaterActivity(context))
    , syncer_(new SyncerActivity(context))
{

}

void toggl::ActivityManager::stopAll() {
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

ReminderActivity *ActivityManager::reminder() {
    return reminder_;
}

SyncerActivity *toggl::ActivityManager::syncer() {
    return syncer_;
}

UiUpdaterActivity *ActivityManager::uiUpdater() {
    return uiUpdater_;
}

};
