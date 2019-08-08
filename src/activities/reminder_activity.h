#ifndef SRC_REMINDER_ACTIVITY_H
#define SRC_REMINDER_ACTIVITY_H

#include "activity.h"

#include <Poco/Activity.h>

namespace toggl {

class Context;
class TimeEntry;

class ReminderActivity : public toggl::Activity {
public:
    void work() override;

    void resetLastReminderTime();

    ReminderActivity(Context *context);

private:
    void checkReminders();
    void reminderActivity();
    void displayReminder();
    void displayPomodoro();
    void displayPomodoroBreak();

    Context *context_ { nullptr };

    Poco::Mutex reminder_m_;
    Poco::Activity<ReminderActivity> activity_;

    Poco::UInt64 last_tracking_reminder_time_;
    Poco::UInt64 last_pomodoro_reminder_time_;
    Poco::UInt64 last_pomodoro_break_reminder_time_;

    // OVERHAUL_TODO - this SHOULD NOT be here, all time entries are protected with a mutex
    // currently solved with a hack (accessing the pointer directly)
    TimeEntry *pomodoro_break_entry_;
};

};

#endif // SRC_REMINDER_ACTIVITY_H
