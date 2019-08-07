#include "reminder_activity.h"

namespace toggl {

void ReminderActivity::checkReminders() {
    displayReminder();
    displayPomodoro();
    displayPomodoroBreak();
}

void ReminderActivity::reminderActivity() {
    while (true) {
        // Sleep in increments for faster shutdown.
        for (int i = 0; i < 4; i++) {
            if (reminder_.isStopped()) {
                return;
            }
            Poco::Thread::sleep(250);
        }

        checkReminders();
    }
}

void ReminderActivity::displayReminder() {
    if (!settings_.reminder) {
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return;
        }

        if (user_ && user_->RunningTimeEntry()) {
            return;
        }

        if (time(0) - last_tracking_reminder_time_
                < settings_.reminder_minutes * 60) {
            return;
        }
    }

    // Check if allowed to display reminder on this weekday
    Poco::LocalDateTime now;
    int wday = now.dayOfWeek();
    if (
            (Poco::DateTime::MONDAY == wday && !settings_.remind_mon) ||
            (Poco::DateTime::TUESDAY == wday && !settings_.remind_tue) ||
            (Poco::DateTime::WEDNESDAY == wday && !settings_.remind_wed) ||
            (Poco::DateTime::THURSDAY == wday && !settings_.remind_thu) ||
            (Poco::DateTime::FRIDAY == wday && !settings_.remind_fri) ||
            (Poco::DateTime::SATURDAY == wday && !settings_.remind_sat) ||
            (Poco::DateTime::SUNDAY == wday && !settings_.remind_sun)) {
        logger().debug("reminder is not enabled on this weekday");
        return;
    }

    // Check if allowed to display reminder at this time
    if (!settings_.remind_starts.empty()) {
        int h(0), m(0);
        if (toggl::Formatter::ParseTimeInput(settings_.remind_starts, &h, &m)) {
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
    if (!settings_.remind_ends.empty()) {
        int h(0), m(0);
        if (toggl::Formatter::ParseTimeInput(settings_.remind_ends, &h, &m)) {
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

    resetLastTrackingReminderTime();

    UI()->DisplayReminder();
}

void ReminderActivity::displayPomodoro() {
    if (!settings_.pomodoro) {
        return;
    }

    Poco::UInt64 wid(0);

    {
        Poco::Mutex::ScopedLock lock(user_m_);
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
                    < settings_.pomodoro_minutes * 60) {
                return;
            }
        } else {
            if (time(0) - current_te->Start()
                    < settings_.pomodoro_minutes * 60) {
                return;
            }
        }
        const Poco::Int64 pomodoroDuration = settings_.pomodoro_minutes * 60;
        wid = current_te->WID();
        Stop(true);
        current_te->SetDurationInSeconds(pomodoroDuration);
        current_te->SetStop(current_te->Start() + pomodoroDuration);
    }
    UI()->DisplayPomodoro(settings_.pomodoro_minutes);

    if (settings_.pomodoro_break) {
        //  Start a new task with the tag "pomodoro-break"
        pomodoro_break_entry_ = user_->Start("Pomodoro Break",  // description
                                             "",  // duration
                                             0,  // task_id
                                             0,  // project_id
                                             "",  // project_guid
                                             "pomodoro-break").data();  // tags

        // Set workspace id to same as the previous entry
        pomodoro_break_entry_->SetWID(wid);
    }
}

void ReminderActivity::displayPomodoroBreak() {
    if (!settings_.pomodoro_break) {
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
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
                < settings_.pomodoro_break_minutes * 60) {
            return;
        }
        const Poco::Int64 pomodoroDuration = settings_.pomodoro_break_minutes * 60;
        Stop(true);
        current_te->SetDurationInSeconds(pomodoroDuration);
        current_te->SetStop(current_te->Start() + pomodoroDuration);
    }
    pomodoro_break_entry_ = nullptr;

    UI()->DisplayPomodoroBreak(settings_.pomodoro_break_minutes);
}

};
