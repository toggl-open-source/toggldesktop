
// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_JSON_H_
#define SRC_JSON_H_

#include <string>
#include <set>
#include <vector>
#include <map>

#include "libjson.h" // NOLINT

#include "./user.h"
#include "./workspace.h"
#include "./client.h"
#include "./project.h"
#include "./task.h"
#include "./time_entry.h"
#include "./tag.h"
#include "./batch_update_result.h"

namespace kopsik {

  void LoadUserFromJSONNode(
    User *model,
    JSONNODE *node,
    const bool full_sync,
    const bool with_related_data);
  void LoadUserFromJSONString(
    User *model,
    const std::string &json,
    const bool full_sync,
    const bool with_related_data);
  void LoadUserProjectsFromJSONNode(
    User *model,
    JSONNODE *list,
    const bool full_sync);
  void LoadUserTagsFromJSONNode(
    User *user,
    JSONNODE *list,
    const bool full_sync);
  void LoadUserClientsFromJSONNode(
    User *user,
    JSONNODE *list,
    const bool full_sync);
  void LoadUserTasksFromJSONNode(
    User *user,
    JSONNODE *list,
    const bool full_sync);
  void LoadUserTimeEntriesFromJSONNode(
    User *user,
    JSONNODE *list,
    const bool full_sync);
  void LoadUserWorkspacesFromJSONNode(
    User *user,
    JSONNODE *list,
    const bool full_sync);
  void LoadUserUpdateFromJSONNode(
    User *user,
    JSONNODE *data);
  void LoadUserUpdateFromJSONString(
    User *user,
    const std::string json);

  void loadUserProjectFromJSONNode(
    User *model,
    JSONNODE *data,
    std::set<Poco::UInt64> *alive = 0);
  void loadUserWorkspaceFromJSONNode(
    User *user,
    JSONNODE *data,
    std::set<Poco::UInt64> *alive = 0);
  void loadUserTagFromJSONNode(
    User *user,
    JSONNODE *data,
    std::set<Poco::UInt64> *alive = 0);
  void loadUserClientFromJSONNode(
    User *user,
    JSONNODE *data,
    std::set<Poco::UInt64> *alive = 0);
  void loadUserTaskFromJSONNode(
    User *user,
    JSONNODE *data,
    std::set<Poco::UInt64> *alive = 0);
  void loadUserTimeEntryFromJSONNode(
    User *user,
    JSONNODE *data,
    std::set<Poco::UInt64> *alive = 0);

  std::string UpdateJSON(
    std::vector<Project *> * const,
    std::vector<TimeEntry *> * const);

  Poco::UInt64 GetIDFromJSONNode(JSONNODE * const);
  guid GetGUIDFromJSONNode(JSONNODE * const);
  Poco::UInt64 GetUIModifiedAtFromJSONNode(JSONNODE * const);
  bool IsDeletedAtServer(JSONNODE * const);

  bool IsValidJSON(const std::string json);

}  // namespace kopsik

#endif  // SRC_JSON_H_
