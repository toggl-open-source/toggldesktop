// Copyright 2014 Toggl Desktop developers.

#include "./batch_update_result.h"

#include <sstream>

namespace kopsik {

error BatchUpdateResult::Error() const {
  if (StatusCode >= 200 && StatusCode < 300) {
    return noError;
  }
  if ("null" != Body) {
    return Body;
  }
  std::stringstream ss;
  ss  << "Request failed with status code "
      << StatusCode;
  return ss.str();
}

std::string BatchUpdateResult::String() const {
  std::stringstream ss;
  ss  << "batch update result GUID: " << GUID
      << ", StatusCode: " << StatusCode
      << ", ContentType: " << ContentType
      << ", Body: " << Body;
  return ss.str();
}

bool BatchUpdateResult::ResourceIsGone() const {
  return ("DELETE" == Method || 404 == StatusCode);
}

}   // namespace kopsik
