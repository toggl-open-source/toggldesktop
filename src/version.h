// Copyright 2013 Tanel Lebedev

#ifndef SRC_VERSION_H_
#define SRC_VERSION_H_

#include <string>

namespace kopsik {

  std::string UserAgent(
      const std::string app_name,
      const std::string app_version);

}  // namespace kopsik

#endif  // SRC_VERSION_H_
