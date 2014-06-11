
// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_JSON_H_
#define SRC_JSON_H_

#include <string>
#include <set>
#include <vector>
#include <map>

#include "libjson.h" // NOLINT

#include "./workspace.h"
#include "./client.h"
#include "./project.h"
#include "./task.h"
#include "./time_entry.h"
#include "./tag.h"
#include "./batch_update_result.h"

namespace kopsik {

std::string UpdateJSON(
    std::vector<Project *> * const,
    std::vector<TimeEntry *> * const);

Poco::UInt64 UserIDFromJSONDataString(const std::string json_data_string);
Poco::UInt64 IDFromJSONNode(JSONNODE * const);
guid GUIDFromJSONNode(JSONNODE * const);
Poco::UInt64 UIModifiedAtFromJSONNode(JSONNODE * const);
bool IsDeletedAtServer(JSONNODE * const);

std::string LoginTokenFromJSONDataString(const std::string json_data_string);

bool IsValidJSON(const std::string json);

}  // namespace kopsik

#endif  // SRC_JSON_H_
