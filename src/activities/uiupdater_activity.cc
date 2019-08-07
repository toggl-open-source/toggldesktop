#include "ui_updater_activity.h"

namespace toggl {

void UiUpdaterActivity::uiUpdaterActivity() {
    std::string running_time("");
    while (!ui_updater_.isStopped()) {
        // Sleep in increments for faster shutdown.
        for (unsigned int i = 0; i < 4*10; i++) {
            if (ui_updater_.isStopped()) {
                return;
            }
            Poco::Thread::sleep(250);
        }

        locked<TimeEntry> te;
        Poco::Int64 duration(0);
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                continue;
            }
            te = user_->RunningTimeEntry();
            if (!te) {
                continue;
            }
            duration = user_->related.TotalDurationForDate(*te);
        }

        std::string date_duration =
                Formatter::FormatDurationForDateHeader(duration);

        if (running_time != date_duration) {
            UIElements render;
            render.display_time_entries = true;
            updateUI(render);
        }

        running_time = date_duration;
    }
}

};
