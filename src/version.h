// Copyright 2013 Tanel Lebedev

#ifndef SRC_VERSION_H_
#define SRC_VERSION_H_

#include <string>
#include <sstream>

namespace kopsik {
  static std::string UserAgent(
      const std::string app_name,
      const std::string app_version) {
    std::stringstream ss;
    ss  << app_name
        << "/"
        << app_version;
    return ss.str();
  }
}  // namespace kopsik

#endif  // SRC_VERSION_H_
