// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_IDLE_H_
#define SRC_IDLE_H_

#include <sstream>

#include "./settings.h"
#include "./gui.h"
#include "./user.h"

#include "Poco/Types.h"
#include "Poco/Logger.h"

namespace toggl {

class Idle {
 public:
    explicit Idle(GUI *ui);
    virtual ~Idle() {}

    void SetIdleSeconds(
        const Poco::UInt64 idle_seconds,
        User *current_user);

    void SetSettings(const Settings settings) {
        settings_ = settings;
    }

    void SetSleep() {
        last_sleep_started_ = time(0);
    }

    void SetWake(User *current_user) {
        if (last_sleep_started_) {
            Poco::Int64 slept_seconds = time(0) - last_sleep_started_;
            if (slept_seconds > 0) {
                SetIdleSeconds(slept_seconds, current_user);
            }
        }

        last_sleep_started_ = 0;
    }

 private:
    void computeIdleState(
        const Poco::UInt64 idle_seconds,
        User *current_user);

    Poco::Logger &logger() const {
        return Poco::Logger::get("idle");
    }

    // Idle detection related values
    Poco::UInt64 last_idle_seconds_reading_;
    Poco::UInt64 last_idle_started_;
    time_t last_sleep_started_;

    Settings settings_;
    GUI *ui_;
};

}  // namespace toggl

#endif  // SRC_IDLE_H_
