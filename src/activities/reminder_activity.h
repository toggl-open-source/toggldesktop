#ifndef SRC_REMINDER_ACTIVITY_H
#define SRC_REMINDER_ACTIVITY_H

#include "activity.h"

#include <Poco/Activity.h>

namespace toggl {

class Context;

class ReminderActivity : public toggl::Activity {
    void work() override;

    ReminderActivity(Context *context);

    void checkReminders();
    void reminderActivity();
    void displayReminder();
    void displayPomodoro();
    void displayPomodoroBreak();

private:
    Context *context_ { nullptr };

    Poco::Mutex reminder_m_;
    Poco::Activity<ReminderActivity> activity_;
};

};

#endif // SRC_REMINDER_ACTIVITY_H
