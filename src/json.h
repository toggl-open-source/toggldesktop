
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

namespace json {

std::string UpdateJSON(
    std::vector<Project *> * const,
    std::vector<TimeEntry *> * const);

Poco::UInt64 UserID(const std::string json_data_string);

Poco::UInt64 ID(JSONNODE * const);

guid GUID(JSONNODE * const);

Poco::UInt64 UIModifiedAt(JSONNODE * const);

bool IsDeletedAtServer(JSONNODE * const);

std::string LoginToken(const std::string json_data_string);

bool IsValid(const std::string json);

}  // namespace json

}  // namespace kopsik

#endif  // SRC_JSON_H_
