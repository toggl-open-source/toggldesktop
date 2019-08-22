// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_IDLE_H_
#define SRC_IDLE_H_

#include "model/settings.h"

#include <sstream>

#include "Poco/Types.h"

namespace toggl {

class GUI;
class UserData;

class Idle {
 public:
    explicit Idle(GUI *ui);
    virtual ~Idle() {}

    void SetIdleSeconds(const Poco::Int64 idle_seconds, UserData *current_user);

    void SetSettings(const Settings &settings);

    void SetSleep();

    void SetWake(UserData *current_user);

 private:
    void computeIdleState(const Poco::Int64 idle_seconds, UserData *current_user);

    Poco::Logger &logger() const;

    // Idle detection related values
    Poco::Int64 last_idle_seconds_reading_;
    Poco::Int64 last_idle_started_;
    time_t last_sleep_started_;

    Settings settings_;
    GUI *ui_;
};

}  // namespace toggl

#endif  // SRC_IDLE_H_
