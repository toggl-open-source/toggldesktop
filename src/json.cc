// Copyright 2014 Toggl Desktop developers.

#include "./json.h"

#include <sstream>

#include "./formatter.h"

#include "Poco/Logger.h"

namespace kopsik {

Poco::UInt64 GetIDFromJSONNode(JSONNODE * const data) {
  poco_assert(data);

  JSONNODE_ITERATOR current_node = json_begin(data);
  JSONNODE_ITERATOR last_node = json_end(data);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "id") == 0) {
      return json_as_int(*current_node);
    }
    ++current_node;
  }

  poco_assert(false);
  return 0;
}

guid GetGUIDFromJSONNode(JSONNODE * const data) {
  poco_assert(data);

  JSONNODE_ITERATOR current_node = json_begin(data);
  JSONNODE_ITERATOR last_node = json_end(data);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "guid") == 0) {
      return std::string(json_as_string(*current_node));
    }
    ++current_node;
  }
  return "";
}

bool IsDeletedAtServer(JSONNODE * const data) {
  poco_assert(data);

  JSONNODE_ITERATOR current_node = json_begin(data);
  JSONNODE_ITERATOR last_node = json_end(data);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "server_deleted_at") == 0) {
      return true;
    }
    ++current_node;
  }
  return false;
}

void ParseBatchUpdateResultJSON(
    BatchUpdateResult *model,
    JSONNODE * const n) {
  poco_assert(n);
  poco_assert(model);

  model->StatusCode = 0;
  model->Body = "";
  model->GUID = "";
  model->ContentType = "";
  JSONNODE_ITERATOR i = json_begin(n);
  JSONNODE_ITERATOR e = json_end(n);
  while (i != e) {
    json_char *node_name = json_name(*i);
    if (strcmp(node_name, "status") == 0) {
      model->StatusCode = json_as_int(*i);
    } else if (strcmp(node_name, "body") == 0) {
      model->Body = std::string(json_as_string(*i));
    } else if (strcmp(node_name, "guid") == 0) {
      model->GUID = std::string(json_as_string(*i));
    } else if (strcmp(node_name, "content_type") == 0) {
      model->ContentType = std::string(json_as_string(*i));
    } else if (strcmp(node_name, "method") == 0) {
      model->Method = std::string(json_as_string(*i));
    }
    ++i;
  }
}

bool IsValidJSON(const std::string json) {
    return json_is_valid(json.c_str());
}

void LoadUserFromJSONString(
    User *model,
    const std::string &json,
    const bool full_sync,
    const bool with_related_data) {
  poco_assert(model);
  poco_assert(!json.empty());

  JSONNODE *root = json_parse(json.c_str());
  JSONNODE_ITERATOR current_node = json_begin(root);
  JSONNODE_ITERATOR last_node = json_end(root);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "since") == 0) {
      model->SetSince(json_as_int(*current_node));

      Poco::Logger &logger = Poco::Logger::get("json");
      std::stringstream s;
      s << "User data as of: " << model->Since();
      logger.debug(s.str());

    } else if (strcmp(node_name, "data") == 0) {
      LoadUserFromJSONNode(model, *current_node, full_sync, with_related_data);
    }
    ++current_node;
  }
  json_delete(root);
}

void LoadUserFromJSONNode(
    User *model,
    JSONNODE * const data,
    const bool full_sync,
    const bool with_related_data) {
  poco_assert(model);
  poco_assert(data);

  JSONNODE_ITERATOR current_node = json_begin(data);
  JSONNODE_ITERATOR last_node = json_end(data);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "id") == 0) {
      model->SetID(json_as_int(*current_node));
    } else if (strcmp(node_name, "default_wid") == 0) {
      model->SetDefaultWID(json_as_int(*current_node));
    } else if (strcmp(node_name, "api_token") == 0) {
      model->SetAPIToken(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "email") == 0) {
      model->SetEmail(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "fullname") == 0) {
      model->SetFullname(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "record_timeline") == 0) {
      model->SetRecordTimeline(json_as_bool(*current_node));
    } else if (strcmp(node_name, "store_start_and_stop_time") == 0) {
      model->SetStoreStartAndStopTime(json_as_bool(*current_node));
    } else if (with_related_data) {
      if (strcmp(node_name, "projects") == 0) {
        LoadUserProjectsFromJSONNode(model, *current_node, full_sync);
      } else if (strcmp(node_name, "tags") == 0) {
        LoadUserTagsFromJSONNode(model, *current_node, full_sync);
      } else if (strcmp(node_name, "tasks") == 0) {
        LoadUserTasksFromJSONNode(model, *current_node, full_sync);
      } else if (strcmp(node_name, "time_entries") == 0) {
        LoadUserTimeEntriesFromJSONNode(model, *current_node, full_sync);
      } else if (strcmp(node_name, "workspaces") == 0) {
        LoadUserWorkspacesFromJSONNode(model, *current_node, full_sync);
      } else if (strcmp(node_name, "clients") == 0) {
        LoadUserClientsFromJSONNode(model, *current_node, full_sync);
      }
    }
    ++current_node;
  }
}

void LoadUserTagsFromJSONNode(
    User *model,
    JSONNODE * const list,
    const bool full_sync) {
  poco_assert(model);
  poco_assert(list);

  std::set<Poco::UInt64> alive;

  JSONNODE_ITERATOR current_node = json_begin(list);
  JSONNODE_ITERATOR last_node = json_end(list);
  while (current_node != last_node) {
    LoadUserTagFromJSONNode(model, *current_node, &alive);
    ++current_node;
  }

  if (!full_sync) {
    return;
  }

  for (std::vector<Tag *>::const_iterator it =
      model->related.Tags.begin();
      it != model->related.Tags.end();
      it++) {
    Tag *model = *it;
    if (alive.end() == alive.find(model->ID())) {
      model->MarkAsDeletedOnServer();
    }
  }
}

void LoadUserTagFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {
  poco_assert(user);
  poco_assert(data);
  poco_assert(alive);

  Poco::UInt64 id = GetIDFromJSONNode(data);
  Tag *model = user->GetTagByID(id);

  if (!model) {
    model = user->GetTagByGUID(GetGUIDFromJSONNode(data));
  }

  if (IsDeletedAtServer(data)) {
    if (model) {
      model->MarkAsDeletedOnServer();
    }
    return;
  }

  if (!model) {
    model = new Tag();
    user->related.Tags.push_back(model);
  }
  if (alive) {
    alive->insert(id);
  }
  model->SetUID(user->ID());
  LoadTagFromJSONNode(model, data);
}

void LoadTagFromJSONNode(
    Tag *model,
    JSONNODE * const data) {
  poco_assert(model);
  poco_assert(data);

  JSONNODE_ITERATOR current_node = json_begin(data);
  JSONNODE_ITERATOR last_node = json_end(data);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "id") == 0) {
      model->SetID(json_as_int(*current_node));
    } else if (strcmp(node_name, "name") == 0) {
      model->SetName(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "guid") == 0) {
      model->SetGUID(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "wid") == 0) {
      model->SetWID(json_as_int(*current_node));
    }
    ++current_node;
  }
}

void LoadUserTasksFromJSONNode(
    User *user,
    JSONNODE * const list,
    const bool full_sync) {
  poco_assert(user);
  poco_assert(list);

  std::set<Poco::UInt64> alive;

  JSONNODE_ITERATOR current_node = json_begin(list);
  JSONNODE_ITERATOR last_node = json_end(list);
  while (current_node != last_node) {
    LoadUserTaskFromJSONNode(user, *current_node, &alive);
    ++current_node;
  }

  if (!full_sync) {
    return;
  }

  for (std::vector<Task *>::const_iterator it =
      user->related.Tasks.begin();
      it != user->related.Tasks.end();
      it++) {
    Task *model = *it;
    if (alive.end() == alive.find(model->ID())) {
      model->MarkAsDeletedOnServer();
    }
  }
}

void LoadUserTaskFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {
  poco_assert(user);
  poco_assert(data);
  poco_assert(alive);

  Poco::UInt64 id = GetIDFromJSONNode(data);
  Task *model = user->GetTaskByID(id);

  // Tasks have no GUID

  if (IsDeletedAtServer(data)) {
    if (model) {
      model->MarkAsDeletedOnServer();
    }
    return;
  }

  if (!model) {
    model = new Task();
    user->related.Tasks.push_back(model);
  }

  if (alive) {
    alive->insert(id);
  }
  model->SetUID(user->ID());
  LoadTaskFromJSONNode(model, data);
}

void LoadUserUpdateFromJSONString(
    User *user,
    const std::string json) {
  poco_assert(user);
  poco_assert(!json.empty());

  JSONNODE *root = json_parse(json.c_str());
  LoadUserUpdateFromJSONNode(user, root);
  json_delete(root);
}

void LoadTaskFromJSONNode(
    Task *task,
    JSONNODE * const data) {
  poco_assert(task);
  poco_assert(data);

  JSONNODE_ITERATOR current_node = json_begin(data);
  JSONNODE_ITERATOR last_node = json_end(data);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "id") == 0) {
      task->SetID(json_as_int(*current_node));
    } else if (strcmp(node_name, "name") == 0) {
      task->SetName(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "pid") == 0) {
      task->SetPID(json_as_int(*current_node));
    } else if (strcmp(node_name, "wid") == 0) {
      task->SetWID(json_as_int(*current_node));
    }
    ++current_node;
  }
}

void LoadUserUpdateFromJSONNode(
    User *user,
    JSONNODE * const node) {
  poco_assert(user);
  poco_assert(node);

  JSONNODE *data = 0;
  std::string model("");
  std::string action("");

  JSONNODE_ITERATOR i = json_begin(node);
  JSONNODE_ITERATOR e = json_end(node);
  while (i != e) {
    json_char *node_name = json_name(*i);
    if (strcmp(node_name, "data") == 0) {
      data = *i;
    } else if (strcmp(node_name, "model") == 0) {
      model = std::string(json_as_string(*i));
    } else if (strcmp(node_name, "action") == 0) {
      action = std::string(json_as_string(*i));
      Poco::toLowerInPlace(action);
    }
    ++i;
  }
  poco_assert(data);

  std::stringstream ss;
  ss << "Update parsed into action=" << action
    << ", model=" + model;
  Poco::Logger &logger = Poco::Logger::get("json");
  logger.debug(ss.str());

  if ("workspace" == model) {
    LoadUserWorkspaceFromJSONNode(user, data, 0);
  } else if ("client" == model) {
    LoadUserClientFromJSONNode(user, data, 0);
  } else if ("project" == model) {
    LoadUserProjectFromJSONNode(user, data, 0);
  } else if ("task" == model) {
    LoadUserTaskFromJSONNode(user, data, 0);
  } else if ("time_entry" == model) {
    LoadUserTimeEntryFromJSONNode(user, data, 0);
  } else if ("tag" == model) {
    LoadUserTagFromJSONNode(user, data, 0);
  }
}

void LoadUserWorkspaceFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {
  poco_assert(user);
  poco_assert(data);
  poco_assert(alive);

  Poco::UInt64 id = GetIDFromJSONNode(data);
  Workspace *model = user->GetWorkspaceByID(id);

  // Workspaces have no GUID

  if (IsDeletedAtServer(data)) {
    if (model) {
      model->MarkAsDeletedOnServer();
    }
    return;
  }

  if (!model) {
    model = new Workspace();
    user->related.Workspaces.push_back(model);
  }
  if (alive) {
    alive->insert(id);
  }
  model->SetUID(user->ID());
  LoadWorkspaceFromJSONNode(model, data);
}

void LoadWorkspaceFromJSONNode(
    Workspace *model,
    JSONNODE * const n) {
  poco_assert(model);
  poco_assert(n);

  JSONNODE_ITERATOR i = json_begin(n);
  JSONNODE_ITERATOR e = json_end(n);
  while (i != e) {
    json_char *node_name = json_name(*i);
    if (strcmp(node_name, "id") == 0) {
      model->SetID(json_as_int(*i));
    } else if (strcmp(node_name, "name") == 0) {
      model->SetName(std::string(json_as_string(*i)));
    } else if (strcmp(node_name, "premium") == 0) {
      model->SetPremium(json_as_bool(*i));
    }
    ++i;
  }
}

void LoadTimeEntryFromJSONNode(
    TimeEntry *model,
    JSONNODE * const data) {
  poco_assert(model);
  poco_assert(data);

  Poco::UInt64 ui_modified_at =
      GetUIModifiedAtFromJSONNode(data);
  if (model->UIModifiedAt() > ui_modified_at) {
      Poco::Logger &logger = Poco::Logger::get("json");
      std::stringstream ss;
      ss  << "Will not overwrite time entry "
          << model->String()
          << " with server data because we have a ui_modified_at";
      logger.debug(ss.str());
      return;
  }

  JSONNODE_ITERATOR current_node = json_begin(data);
  JSONNODE_ITERATOR last_node = json_end(data);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "id") == 0) {
      model->SetID(json_as_int(*current_node));
    } else if (strcmp(node_name, "description") == 0) {
      model->SetDescription(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "guid") == 0) {
      model->SetGUID(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "wid") == 0) {
      model->SetWID(json_as_int(*current_node));
    } else if (strcmp(node_name, "pid") == 0) {
      model->SetPID(json_as_int(*current_node));
    } else if (strcmp(node_name, "tid") == 0) {
      model->SetTID(json_as_int(*current_node));
    } else if (strcmp(node_name, "start") == 0) {
      model->SetStartString(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "stop") == 0) {
      model->SetStopString(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "duration") == 0) {
      model->SetDurationInSeconds(json_as_int(*current_node));
    } else if (strcmp(node_name, "billable") == 0) {
      model->SetBillable(json_as_bool(*current_node));
    } else if (strcmp(node_name, "duronly") == 0) {
      model->SetDurOnly(json_as_bool(*current_node));
    } else if (strcmp(node_name, "tags") == 0) {
      LoadTimeEntryTagsFromJSONNode(model, *current_node);
    } else if (strcmp(node_name, "created_with") == 0) {
      model->SetCreatedWith(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "at") == 0) {
      model->SetUpdatedAtString(std::string(json_as_string(*current_node)));
    }
    ++current_node;
  }

  model->SetUIModifiedAt(0);
}

error LoadTagsFromJSONNode(
    TimeEntry *te,
    JSONNODE * const list) {
  poco_assert(te);
  poco_assert(list);

  te->TagNames.clear();

  JSONNODE_ITERATOR current_node = json_begin(list);
  JSONNODE_ITERATOR last_node = json_end(list);
  while (current_node != last_node) {
    std::string tag = std::string(json_as_string(*current_node));
    if (!tag.empty()) {
      te->TagNames.push_back(tag);
    }
    ++current_node;
  }
  return noError;
}

void LoadUserClientFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {
  poco_assert(user);
  poco_assert(data);
  poco_assert(alive);

  Poco::UInt64 id = GetIDFromJSONNode(data);
  Client *model = user->GetClientByID(id);

  if (!model) {
    model = user->GetClientByGUID(GetGUIDFromJSONNode(data));
  }

  if (IsDeletedAtServer(data)) {
    if (model) {
      model->MarkAsDeletedOnServer();
    }
    return;
  }

  if (!model) {
    model = new Client();
    user->related.Clients.push_back(model);
  }
  if (alive) {
    alive->insert(id);
  }
  model->SetUID(user->ID());
  LoadClientFromJSONNode(model, data);
}

void LoadClientFromJSONNode(
    Client *model,
    JSONNODE * const data) {
  poco_assert(model);
  poco_assert(data);

  JSONNODE_ITERATOR current_node = json_begin(data);
  JSONNODE_ITERATOR last_node = json_end(data);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "id") == 0) {
      model->SetID(json_as_int(*current_node));
    } else if (strcmp(node_name, "name") == 0) {
      model->SetName(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "guid") == 0) {
      model->SetGUID(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "wid") == 0) {
      model->SetWID(json_as_int(*current_node));
    }
    ++current_node;
  }
}

Poco::UInt64 GetUIModifiedAtFromJSONNode(
    JSONNODE * const data) {
  poco_assert(data);

  JSONNODE_ITERATOR current_node = json_begin(data);
  JSONNODE_ITERATOR last_node = json_end(data);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "ui_modified_at") == 0) {
      return json_as_int(*current_node);
    }
    ++current_node;
  }
  return 0;
}

void LoadUserClientsFromJSONNode(
    User *user,
    JSONNODE * const list,
    const bool full_sync) {
  poco_assert(user);
  poco_assert(list);

  std::set<Poco::UInt64> alive;

  JSONNODE_ITERATOR current_node = json_begin(list);
  JSONNODE_ITERATOR last_node = json_end(list);
  while (current_node != last_node) {
    LoadUserClientFromJSONNode(user, *current_node, &alive);
    ++current_node;
  }

  if (!full_sync) {
    return;
  }

  for (std::vector<Client *>::const_iterator it =
      user->related.Clients.begin();
      it != user->related.Clients.end();
      it++) {
    Client *model = *it;
    if (alive.end() == alive.find(model->ID())) {
      model->MarkAsDeletedOnServer();
    }
  }
}

void LoadUserProjectFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {
  poco_assert(user);
  poco_assert(data);
  poco_assert(alive);

  Poco::UInt64 id = GetIDFromJSONNode(data);
  Project *model = user->GetProjectByID(id);

  if (!model) {
    model = user->GetProjectByGUID(GetGUIDFromJSONNode(data));
  }

  if (IsDeletedAtServer(data)) {
    if (model) {
      model->MarkAsDeletedOnServer();
    }
    return;
  }

  if (!model) {
    model = new Project();
    user->related.Projects.push_back(model);
  }
  if (alive) {
    alive->insert(id);
  }
  model->SetUID(user->ID());
  LoadProjectFromJSONNode(model, data);
}

void LoadProjectFromJSONNode(
    Project *project,
    JSONNODE * const data) {
  poco_assert(project);
  poco_assert(data);

  JSONNODE_ITERATOR current_node = json_begin(data);
  JSONNODE_ITERATOR last_node = json_end(data);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "id") == 0) {
      project->SetID(json_as_int(*current_node));
    } else if (strcmp(node_name, "name") == 0) {
      project->SetName(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "guid") == 0) {
      project->SetGUID(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "wid") == 0) {
      project->SetWID(json_as_int(*current_node));
    } else if (strcmp(node_name, "cid") == 0) {
      project->SetCID(json_as_int(*current_node));
    } else if (strcmp(node_name, "color") == 0) {
      project->SetColor(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "active") == 0) {
      project->SetActive(json_as_bool(*current_node));
    } else if (strcmp(node_name, "billable") == 0) {
      project->SetBillable(json_as_bool(*current_node));
    }
    ++current_node;
  }
}

void LoadUserProjectsFromJSONNode(
    User *user,
    JSONNODE * const list,
    const bool full_sync) {
  poco_assert(user);
  poco_assert(list);

  std::set<Poco::UInt64> alive;

  JSONNODE_ITERATOR current_node = json_begin(list);
  JSONNODE_ITERATOR last_node = json_end(list);
  while (current_node != last_node) {
    LoadUserProjectFromJSONNode(user, *current_node, &alive);
    ++current_node;
  }

  if (!full_sync) {
    return;
  }

  for (std::vector<Project *>::const_iterator it =
      user->related.Projects.begin();
      it != user->related.Projects.end();
      it++) {
    Project *model = *it;
    if (alive.end() == alive.find(model->ID())) {
        model->MarkAsDeletedOnServer();
    }
  }
}

error LoadTimeEntryTagsFromJSONNode(
    TimeEntry *te,
    JSONNODE * const list) {
  poco_assert(te);
  poco_assert(list);

  te->TagNames.clear();

  JSONNODE_ITERATOR current_node = json_begin(list);
  JSONNODE_ITERATOR last_node = json_end(list);
  while (current_node != last_node) {
    std::string tag = std::string(json_as_string(*current_node));
    if (!tag.empty()) {
      te->TagNames.push_back(tag);
    }
    ++current_node;
  }
  return noError;
}

void LoadUserTimeEntryFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {
  poco_assert(user);
  poco_assert(data);
  poco_assert(alive);

  Poco::UInt64 id = GetIDFromJSONNode(data);
  TimeEntry *model = user->GetTimeEntryByID(id);

  if (!model) {
    model = user->GetTimeEntryByGUID(GetGUIDFromJSONNode(data));
  }

  if (IsDeletedAtServer(data)) {
    if (model) {
      model->MarkAsDeletedOnServer();
    }
    return;
  }

  if (!model) {
    model = new TimeEntry();
    user->related.TimeEntries.push_back(model);
  }
  if (alive) {
    alive->insert(id);
  }
  model->SetUID(user->ID());
  LoadTimeEntryFromJSONNode(model, data);
}

void LoadUserWorkspacesFromJSONNode(
    User *user,
    JSONNODE * const list,
    const bool full_sync) {
  poco_assert(user);
  poco_assert(list);

  std::set<Poco::UInt64> alive;

  JSONNODE_ITERATOR current_node = json_begin(list);
  JSONNODE_ITERATOR last_node = json_end(list);
  while (current_node != last_node) {
    LoadUserWorkspaceFromJSONNode(user, *current_node, &alive);
    ++current_node;
  }

  if (!full_sync) {
    return;
  }

  for (std::vector<Workspace *>::const_iterator it =
      user->related.Workspaces.begin();
      it != user->related.Workspaces.end();
      it++) {
    Workspace *model = *it;
    if (alive.end() == alive.find(model->ID())) {
      model->MarkAsDeletedOnServer();
    }
  }
}

void LoadUserTimeEntriesFromJSONNode(
    User *user,
    JSONNODE * const list,
    const bool full_sync) {
  poco_assert(user);
  poco_assert(list);

  std::set<Poco::UInt64> alive;

  JSONNODE_ITERATOR current_node = json_begin(list);
  JSONNODE_ITERATOR last_node = json_end(list);
  while (current_node != last_node) {
    LoadUserTimeEntryFromJSONNode(user, *current_node, &alive);
    ++current_node;
  }

  if (!full_sync) {
    return;
  }

  for (std::vector<TimeEntry *>::const_iterator it =
     user->related.TimeEntries.begin();
     it != user->related.TimeEntries.end();
     it++) {
    TimeEntry *model = *it;
    if (alive.end() == alive.find(model->ID())) {
      model->MarkAsDeletedOnServer();
    }
  }
}

JSONNODE *modelUpdateJSON(
    BaseModel * const model,
    JSONNODE * const n) {
  poco_assert(model);
  poco_assert(n);

  json_set_name(n, model->ModelName().c_str());

  JSONNODE *body = json_new(JSON_NODE);
  json_set_name(body, "body");
  json_push_back(body, n);

  Poco::Logger &logger = Poco::Logger::get("json");

  JSONNODE *update = json_new(JSON_NODE);
  if (model->NeedsDELETE()) {
    std::stringstream url;
    url << model->ModelURL() << "/" << model->ID();
    json_push_back(update, json_new_a("method", "DELETE"));
    json_push_back(update, json_new_a("relative_url", url.str().c_str()));
    std::stringstream ss;
    ss << model->ModelName() << " " << model->String() << " needs a DELETE";
    logger.debug(ss.str());

  } else if (model->NeedsPOST()) {
    json_push_back(update, json_new_a("method", "POST"));
    json_push_back(update, json_new_a("relative_url",
      model->ModelURL().c_str()));
    std::stringstream ss;
    ss << model->ModelName() << " " << model->String() << " needs a POST";
    logger.debug(ss.str());

  } else if (model->NeedsPUT()) {
    std::stringstream url;
    url << model->ModelURL() << "/" << model->ID();
    json_push_back(update, json_new_a("method", "PUT"));
    json_push_back(update, json_new_a("relative_url", url.str().c_str()));
    std::stringstream ss;
    ss << model->ModelName() << " " << model->String() << " needs a PUT";
    logger.debug(ss.str());
  }
  json_push_back(update, json_new_a("GUID", model->GUID().c_str()));
  json_push_back(update, body);

  return update;
}

std::string UpdateJSON(
    std::vector<Project *> * const projects,
    std::vector<TimeEntry *> * const time_entries) {
  poco_assert(projects);
  poco_assert(time_entries);

  JSONNODE *c = json_new(JSON_ARRAY);

  // First, projects, because time entries depend on projects
  for (std::vector<Project *>::const_iterator it =
      projects->begin();
      it != projects->end(); it++) {
    Project *model = *it;
    JSONNODE *update = modelUpdateJSON(model, ProjectToJSON(model));
    json_push_back(c, update);
  }

  // Time entries go last
  for (std::vector<TimeEntry *>::const_iterator it =
      time_entries->begin();
      it != time_entries->end(); it++) {
    TimeEntry *te = *it;
    JSONNODE *update = modelUpdateJSON(te, TimeEntryToJSON(te));
    json_push_back(c, update);
  }

  json_char *jc = json_write_formatted(c);
  std::string json(jc);
  json_free(jc);
  json_delete(c);
  return json;
}

// Iterate through response array, parse response bodies.
// Collect errors into a vector.
void ProcessResponseArray(
    std::vector<BatchUpdateResult> * const results,
    std::vector<Project *> *projects,
    std::vector<TimeEntry *> *time_entries,
    std::vector<error> *errors) {
  poco_assert(results);
  poco_assert(projects);
  poco_assert(time_entries);
  poco_assert(errors);

  Poco::Logger &logger = Poco::Logger::get("json");
  for (std::vector<BatchUpdateResult>::const_iterator it = results->begin();
          it != results->end();
          it++) {
      BatchUpdateResult result = *it;

      std::stringstream ss;
      ss  << "batch update result GUID: " << result.GUID
          << ", StatusCode: " << result.StatusCode
          << ", ContentType: " << result.ContentType
          << ", Body: " << result.Body;
      logger.debug(ss.str());

      if (result.StatusCode != 404)  {
          if ((result.StatusCode < 200) || (result.StatusCode >= 300)) {
              if ("null" == result.Body) {
                  std::stringstream ss;
                  ss  << "Request failed with status code "
                      << result.StatusCode;
                  errors->push_back(ss.str());
              } else {
                  errors->push_back(result.Body);
              }
              continue;
          }

          poco_assert(!result.GUID.empty());
          poco_assert(json_is_valid(result.Body.c_str()));
      }

      TimeEntry *te = 0;
      for (std::vector<TimeEntry *>::const_iterator it =
              time_entries->begin(); it != time_entries->end(); it++) {
          if ((*it)->GUID() == result.GUID) {
              te = *it;
              break;
          }
      }
      poco_assert(te);

      // If TE was deleted, the body won't contain useful data.
      if (("DELETE" == result.Method) || (404 == result.StatusCode)) {
          te->MarkAsDeletedOnServer();
          continue;
      }

      JSONNODE *n = json_parse(result.Body.c_str());
      JSONNODE_ITERATOR i = json_begin(n);
      JSONNODE_ITERATOR e = json_end(n);
      while (i != e) {
          json_char *node_name = json_name(*i);
          if (strcmp(node_name, "data") == 0) {
              LoadTimeEntryFromJSONNode(te, *i);
          }
          ++i;
      }
      json_delete(n);
  }
}

void ParseResponseArray(
    const std::string response_body,
    std::vector<BatchUpdateResult> *responses) {
  poco_assert(responses);
  poco_assert(!response_body.empty());
  poco_assert(responses);

  JSONNODE *response_array = json_parse(response_body.c_str());
  JSONNODE_ITERATOR i = json_begin(response_array);
  JSONNODE_ITERATOR e = json_end(response_array);
  while (i != e) {
    BatchUpdateResult result;
    ParseBatchUpdateResultJSON(&result, *i);
    responses->push_back(result);
    ++i;
  }
  json_delete(response_array);
}

JSONNODE *TimeEntryToJSON(TimeEntry * const te) {
  poco_assert(te);

  JSONNODE *n = json_new(JSON_NODE);
  json_set_name(n, te->ModelName().c_str());
  if (te->ID()) {
    json_push_back(n, json_new_i("id", (json_int_t)te->ID()));
  }
  json_push_back(n, json_new_a("description",
    Formatter::EscapeJSONString(te->Description()).c_str()));
  json_push_back(n, json_new_i("wid", (json_int_t)te->WID()));
  json_push_back(n, json_new_a("guid", te->GUID().c_str()));
  json_push_back(n, json_new_i("pid", (json_int_t)te->PID()));
  json_push_back(n, json_new_i("tid", (json_int_t)te->TID()));
  json_push_back(n, json_new_a("start", te->StartString().c_str()));
  if (te->Stop()) {
    json_push_back(n, json_new_a("stop", te->StopString().c_str()));
  }
  json_push_back(n, json_new_i("duration",
    (json_int_t)te->DurationInSeconds()));
  json_push_back(n, json_new_b("billable", te->Billable()));
  json_push_back(n, json_new_b("duronly", te->DurOnly()));
  json_push_back(n, json_new_i("ui_modified_at",
      (json_int_t)te->UIModifiedAt()));
  json_push_back(n, json_new_a("created_with",
      Formatter::EscapeJSONString(te->CreatedWith()).c_str()));

  JSONNODE *tag_nodes = json_new(JSON_ARRAY);
  json_set_name(tag_nodes, "tags");
  for (std::vector<std::string>::const_iterator it = te->TagNames.begin();
          it != te->TagNames.end();
          it++) {
      std::string tag_name = *it;
      json_push_back(tag_nodes, json_new_a(NULL,
          Formatter::EscapeJSONString(tag_name).c_str()));
  }
  json_push_back(n, tag_nodes);

  return n;
}

void LoadTimeEntryFromJSONString(
    TimeEntry *model,
    const std::string json) {
  poco_assert(model);
  poco_assert(!json.empty());

  JSONNODE *root = json_parse(json.c_str());
  LoadTimeEntryFromJSONNode(model, root);
  json_delete(root);
}

JSONNODE *ProjectToJSON(Project * const model) {
  poco_assert(model);

  JSONNODE *n = json_new(JSON_NODE);
  json_set_name(n, model->ModelName().c_str());
  if (model->ID()) {
    json_push_back(n, json_new_i("id", (json_int_t)model->ID()));
  }
  json_push_back(n, json_new_a("name",
    Formatter::EscapeJSONString(model->Name()).c_str()));
  json_push_back(n, json_new_i("wid", (json_int_t)model->WID()));
  json_push_back(n, json_new_a("guid", model->GUID().c_str()));
  json_push_back(n, json_new_i("cid", (json_int_t)model->CID()));
  json_push_back(n, json_new_b("billable", model->Billable()));
  json_push_back(n, json_new_i("ui_modified_at",
      (json_int_t)model->UIModifiedAt()));

  return n;
}

}   // namespace kopsik
