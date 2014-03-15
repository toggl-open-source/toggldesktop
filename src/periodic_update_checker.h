// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_PERIODIC_UPDATE_CHECKER_H_
#define SRC_PERIODIC_UPDATE_CHECKER_H_

#include <string>

#include "./const.h"
#include "./types.h"

#include "Poco/Activity.h"
#include "Poco/Logger.h"

namespace kopsik {

class PeriodicUpdateChecker {
  public:
    PeriodicUpdateChecker(
        const std::string app_name,
        const std::string app_version)
      : app_name_(app_name)
      , app_version_(app_version)
      , activity_(this, &PeriodicUpdateChecker::onActivity) {
        activity_.start();
    }

  ~PeriodicUpdateChecker() {
      stop();
  }

 protected:
    void onActivity();

 private:
    std::string app_name_;
    std::string app_version_;

    Poco::Activity<PeriodicUpdateChecker> activity_;

    Poco::Logger &logger() const {
      return Poco::Logger::get("PeriodicUpdateChecker"); }

    error stop();
};

}  // namespace kopsik

#endif  // SRC_PERIODIC_UPDATE_CHECKER_H_
