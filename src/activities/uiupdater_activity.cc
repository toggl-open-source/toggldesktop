#include "uiupdater_activity.h"

#include "../gui.h"
#include "../related_data.h"
#include "../context.h"

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
        bool gotDuration = true;
        context_->UserVisit([&](User *user_){
            if (!user_) {
                gotDuration = false;
            }
            te = user_->RunningTimeEntry();
            if (!te) {
                gotDuration = false;
            }
            duration = user_->related.TotalDurationForDate(*te);
        });
        if (!gotDuration)
            continue;

        std::string date_duration =
                Formatter::FormatDurationForDateHeader(duration);

        if (running_time != date_duration) {
            UIElements render;
            render.display_time_entries = true;
            context_->updateUI(render);
        }

        running_time = date_duration;
    }
}

};
