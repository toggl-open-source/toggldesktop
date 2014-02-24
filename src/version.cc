// Copyright 2014 Toggl Desktop developers.

#include "../src/version.h"

#include <sstream>

namespace kopsik {

  std::string UserAgent(
      const std::string app_name,
      const std::string app_version) {
    std::stringstream ss;
    ss  << app_name
        << "/"
        << app_version;
    return ss.str();
  }

}  // namespace kopsik
