// Copyright 2014 Tanel Lebedev

#ifndef SRC_EXPLICIT_SCOPED_LOCK_H_
#define SRC_EXPLICIT_SCOPED_LOCK_H_

#include <string>
#include <sstream>

#include "Poco/Logger.h"

namespace kopsik {

#define kLockTimeoutMillis 100

class ExplicitScopedLock : public Poco::Mutex::ScopedLock {
   public:
    ExplicitScopedLock(
      const std::string context,
      Poco::Mutex& mutex) :
        Poco::Mutex::ScopedLock(mutex, kLockTimeoutMillis),
        context_(context) {
      std::stringstream text;
      text << context_ << " locking";
      logger().debug(text.str());
    }

    ~ExplicitScopedLock() {
      std::stringstream text;
      text << context_ << " unlocking";
      logger().debug(text.str());
    }

   private:
    Poco::Logger &logger() { return Poco::Logger::get("lock"); }

    std::string context_;
};

}  // namespace kopsik

#endif  // SRC_EXPLICIT_SCOPED_LOCK_H_
