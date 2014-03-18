// Copyright 2014 Toggl Desktop developers.

#include "./periodic_update_checker.h"

#include "Poco/Foundation.h"
#include "Poco/Util/Application.h"
#include "Poco/Thread.h"

namespace kopsik {

void PeriodicUpdateChecker::onActivity() {
  while (!activity_.isStopped()) {
    logger().debug("PeriodicUpdateChecker onActivity");

    // FIXME: check for updates

    Poco::Thread::sleep(kCheckUpdateIntervalSeconds * 1000);
  }
}

kopsik::error PeriodicUpdateChecker::stop() {
  try {
    if (activity_.isRunning()) {
      activity_.stop();
      activity_.wait();
    }
  } catch(const Poco::Exception& exc) {
      return exc.displayText();
  } catch(const std::exception& ex) {
      return ex.what();
  } catch(const std::string& ex) {
      return ex;
  }
  return noError;
}

}  // namespace kopsik
