// Copyright 2014 Toggl Desktop developers.

#include "./batch_update_result.h"

#include <sstream>
#include <cstring>

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

void BatchUpdateResult::LoadFromJSONNode(JSONNODE * const n) {
  poco_assert(n);

  StatusCode = 0;
  Body = "";
  GUID = "";
  ContentType = "";
  JSONNODE_ITERATOR i = json_begin(n);
  JSONNODE_ITERATOR e = json_end(n);
  while (i != e) {
    json_char *node_name = json_name(*i);
    if (strcmp(node_name, "status") == 0) {
      StatusCode = json_as_int(*i);
    } else if (strcmp(node_name, "body") == 0) {
      Body = std::string(json_as_string(*i));
    } else if (strcmp(node_name, "guid") == 0) {
      GUID = std::string(json_as_string(*i));
    } else if (strcmp(node_name, "content_type") == 0) {
      ContentType = std::string(json_as_string(*i));
    } else if (strcmp(node_name, "method") == 0) {
      Method = std::string(json_as_string(*i));
    }
    ++i;
  }
}

}   // namespace kopsik
