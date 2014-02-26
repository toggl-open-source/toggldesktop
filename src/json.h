// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_JSON_H_
#define SRC_JSON_H_

#include <string>
#include <set>
#include <vector>

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

  void ParseResponseArray(
    const std::string response_body,
    std::vector<BatchUpdateResult> *responses);
  void ProcessResponseArray(
    std::vector<BatchUpdateResult> *results,
    std::vector<TimeEntry *> *dirty,
    std::vector<error> *errors);

  void LoadWorkspaceFromJSONNode(Workspace *model, JSONNODE *node);
  void LoadClientFromJSONNode(Client *model, JSONNODE *node);
  void LoadProjectFromJSONNode(Project *model, JSONNODE *node);
  void LoadTaskFromJSONNode(Task *model, JSONNODE *node);
  void LoadTagFromJSONNode(Tag *model, JSONNODE *node);
  error LoadTimeEntryTagsFromJSONNode(
    TimeEntry *model,
    JSONNODE *list);

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
  void LoadUserProjectFromJSONNode(
    User *model,
    JSONNODE *data,
    std::set<Poco::UInt64> *alive);
  void LoadUserTagsFromJSONNode(
    User *user,
    JSONNODE *list,
    const bool full_sync);
  void LoadUserTagFromJSONNode(
    User *user,
    JSONNODE *data,
    std::set<Poco::UInt64> *alive);
  void LoadUserClientsFromJSONNode(
    User *user,
    JSONNODE *list,
    const bool full_sync);
  void LoadUserClientFromJSONNode(
    User *user,
    JSONNODE *data,
    std::set<Poco::UInt64> *alive);
  void LoadUserTasksFromJSONNode(
    User *user,
    JSONNODE *list,
    const bool full_sync);
  void LoadUserTaskFromJSONNode(
    User *user,
    JSONNODE *data,
    std::set<Poco::UInt64> *alive);
  void LoadUserTimeEntriesFromJSONNode(
    User *user,
    JSONNODE *list,
    const bool full_sync);
  void LoadUserTimeEntryFromJSONNode(
    User *user,
    JSONNODE *data,
    std::set<Poco::UInt64> *alive);
  void LoadUserWorkspacesFromJSONNode(
    User *user,
    JSONNODE *list,
    const bool full_sync);
  void LoadUserWorkspaceFromJSONNode(
    User *user,
    JSONNODE *data,
    std::set<Poco::UInt64> *alive);
  void LoadUserUpdateFromJSONNode(
    User *user,
    JSONNODE *data);
  void LoadUserUpdateFromJSONString(
    User *user,
    const std::string json);

  void LoadTimeEntryFromJSONNode(TimeEntry *model, JSONNODE *node);
  void LoadTimeEntryFromJSONString(TimeEntry *model, std::string json);

  JSONNODE *TimeEntryToJSON(TimeEntry * const);

  std::string DirtyUserObjectsJSON(
    std::vector<TimeEntry *> * const);

  void ParseResponseJSON(
    BatchUpdateResult *model,
    JSONNODE *n);

  Poco::UInt64 GetIDFromJSONNode(JSONNODE * const);
  guid GetGUIDFromJSONNode(JSONNODE * const);
  Poco::UInt64 GetUIModifiedAtFromJSONNode(JSONNODE * const);
  bool IsDeletedAtServer(JSONNODE * const);

  bool IsValidJSON(const std::string json);

}  // namespace kopsik

#endif  // SRC_JSON_H_
