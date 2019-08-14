#include "reminder_activity.h"

#include "../context.h"
#include "../time_entry.h"

#include <Poco/Logger.h>

namespace toggl {

ReminderActivity::ReminderActivity(Context *context)
    : toggl::Activity(context)
{

}

void ReminderActivity::work() {
    reminderActivity();
}

void ReminderActivity::resetLastReminderTime() {
    last_tracking_reminder_time_ = time(0);
}

void ReminderActivity::checkReminders() {
    displayReminder();
    displayPomodoro();
    displayPomodoroBreak();
}

void ReminderActivity::reminderActivity() {
    while (true) {
        // Sleep in increments for faster shutdown.
        for (int i = 0; i < 4; i++) {
            if (activity_.isStopped()) {
                return;
            }
            Poco::Thread::sleep(250);
        }

        checkReminders();
    }
}

void ReminderActivity::displayReminder() {
    if (!context_->settings()->reminder) {
        return;
    }

    context_->UserVisit([&](User *user_){
        if (!user_) {
            return;
        }

        if (user_ && user_->RunningTimeEntry()) {
            return;
        }

        if (time(0) - last_tracking_reminder_time_
                < context_->settings()->reminder_minutes * 60) {
            return;
        }
    });

    // Check if allowed to display reminder on this weekday
    Poco::LocalDateTime now;
    int wday = now.dayOfWeek();
    if (
            (Poco::DateTime::MONDAY == wday && !context_->settings()->remind_mon) ||
            (Poco::DateTime::TUESDAY == wday && !context_->settings()->remind_tue) ||
            (Poco::DateTime::WEDNESDAY == wday && !context_->settings()->remind_wed) ||
            (Poco::DateTime::THURSDAY == wday && !context_->settings()->remind_thu) ||
            (Poco::DateTime::FRIDAY == wday && !context_->settings()->remind_fri) ||
            (Poco::DateTime::SATURDAY == wday && !context_->settings()->remind_sat) ||
            (Poco::DateTime::SUNDAY == wday && !context_->settings()->remind_sun)) {
        logger().debug("reminder is not enabled on this weekday");
        return;
    }

    // Check if allowed to display reminder at this time
    if (!context_->settings()->remind_starts.empty()) {
        int h(0), m(0);
        if (toggl::Formatter::ParseTimeInput(context_->settings()->remind_starts, &h, &m)) {
            Poco::LocalDateTime start(
                        now.year(), now.month(), now.day(), h, m, now.second());
            if (now < start) {
                std::stringstream ss;
                ss << "Reminder - its too early for reminders"
                   << " [" << now.hour() << ":" << now.minute() << "]"
                   << " (allowed from " << h << ":" << m << ")";
                logger().debug(ss.str());
                return;
            }
        }
    }
    if (!context_->settings()->remind_ends.empty()) {
        int h(0), m(0);
        if (toggl::Formatter::ParseTimeInput(context_->settings()->remind_ends, &h, &m)) {
            Poco::LocalDateTime end(
                        now.year(), now.month(), now.day(), h, m, now.second());
            if (now > end) {
                std::stringstream ss;
                ss << "Reminder - its too late for reminders"
                   << " [" << now.hour() << ":" << now.minute() << "]"
                   << " (allowed until " << h << ":" << m << ")";
                logger().debug(ss.str());
                return;
            }
        }
    }

    resetLastReminderTime();

    context_->UI()->DisplayReminder();
}

void ReminderActivity::displayPomodoro() {
    if (!context_->settings()->pomodoro) {
        return;
    }

    Poco::UInt64 wid(0);

    context_->UserVisit([&](User *user_) {
        if (!user_) {
            return;
        }

        auto current_te = user_->RunningTimeEntry();
        if (!current_te) {
            return;
        }
        if (pomodoro_break_entry_ != nullptr
                && !current_te->GUID().empty()
                && current_te->GUID().compare(
                    pomodoro_break_entry_->GUID()) == 0) {
            return;
        }

        if (current_te->DurOnly() && current_te->LastStartAt() != 0) {
            if (time(0) - current_te->LastStartAt()
                    < context_->settings()->pomodoro_minutes * 60) {
                return;
            }
        } else {
            if (time(0) - current_te->Start()
                    < context_->settings()->pomodoro_minutes * 60) {
                return;
            }
        }
        const Poco::Int64 pomodoroDuration = context_->settings()->pomodoro_minutes * 60;
        wid = current_te->WID();
        context_->Stop(true);
        current_te->SetDurationInSeconds(pomodoroDuration);
        current_te->SetStop(current_te->Start() + pomodoroDuration);
    });

    context_->UI()->DisplayPomodoro(context_->settings()->pomodoro_minutes);

    if (context_->settings()->pomodoro_break) {
        //  Start a new task with the tag "pomodoro-break"
        context_->UserVisit([&](User *user_){
            pomodoro_break_entry_ = user_->Start("Pomodoro Break",  // description
                                                 "",  // duration
                                                 0,  // task_id
                                                 0,  // project_id
                                                 "",  // project_guid
                                                 "pomodoro-break").data();  // tags
            // Set workspace id to same as the previous entry
            pomodoro_break_entry_->SetWID(wid);
        });
    }
}

void ReminderActivity::displayPomodoroBreak() {
    if (!context_->settings()->pomodoro_break) {
        return;
    }

    context_->UserVisit([&](User *user_){
        if (!user_) {
            return;
        }

        auto current_te = user_->RunningTimeEntry();

        if (!current_te) {
            return;
        }

        if (pomodoro_break_entry_ == nullptr
                || current_te->GUID().compare(
                    pomodoro_break_entry_->GUID()) != 0) {
            return;
        }

        if (time(0) - current_te->Start()
                < context_->settings()->pomodoro_break_minutes * 60) {
            return;
        }
        const Poco::Int64 pomodoroDuration = context_->settings()->pomodoro_break_minutes * 60;
        context_->Stop(true);
        current_te->SetDurationInSeconds(pomodoroDuration);
        current_te->SetStop(current_te->Start() + pomodoroDuration);
    });
    pomodoro_break_entry_ = nullptr;

    context_->UI()->DisplayPomodoroBreak(context_->settings()->pomodoro_break_minutes);
}

};
