// Copyright 2014 Toggl Desktop developers.

#include "./json.h"

#include <sstream>
#include <cstring>

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
    loadUserTagFromJSONNode(model, *current_node, &alive);
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

void loadUserTagFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {
  poco_assert(user);
  poco_assert(data);
  // alive can be 0

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
  model->LoadFromJSONNode(data);
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
    loadUserTaskFromJSONNode(user, *current_node, &alive);
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

void loadUserTaskFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {
  poco_assert(user);
  poco_assert(data);
  // alive can be 0

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
  model->LoadFromJSONNode(data);
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
    loadUserWorkspaceFromJSONNode(user, data);
  } else if ("client" == model) {
    loadUserClientFromJSONNode(user, data);
  } else if ("project" == model) {
    loadUserProjectFromJSONNode(user, data);
  } else if ("task" == model) {
    loadUserTaskFromJSONNode(user, data);
  } else if ("time_entry" == model) {
    loadUserTimeEntryFromJSONNode(user, data);
  } else if ("tag" == model) {
    loadUserTagFromJSONNode(user, data);
  }
}

void loadUserWorkspaceFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {
  poco_assert(user);
  poco_assert(data);
  // alive can be 0

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
  model->LoadFromJSONNode(data);
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

void loadUserClientFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {
  poco_assert(user);
  poco_assert(data);
  // alive can be 0

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
  model->LoadFromJSONNode(data);
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
    loadUserClientFromJSONNode(user, *current_node, &alive);
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

void loadUserProjectFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {
  poco_assert(user);
  poco_assert(data);
  // alive can be 0

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
  model->LoadFromJSONNode(data);
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
    loadUserProjectFromJSONNode(user, *current_node, &alive);
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

void loadUserTimeEntryFromJSONNode(
    User *user,
    JSONNODE * const data,
    std::set<Poco::UInt64> *alive) {
  poco_assert(user);
  poco_assert(data);
  // alive can be 0

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
  model->LoadFromJSONNode(data);
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
    loadUserWorkspaceFromJSONNode(user, *current_node, &alive);
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
    loadUserTimeEntryFromJSONNode(user, *current_node, &alive);
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
  json_push_back(update, json_new_a("guid", model->GUID().c_str()));
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
    JSONNODE *update = modelUpdateJSON(model, model->SaveToJSONNode());
    json_push_back(c, update);
  }

  // Time entries go last
  for (std::vector<TimeEntry *>::const_iterator it =
      time_entries->begin();
      it != time_entries->end(); it++) {
    TimeEntry *te = *it;
    JSONNODE *update = modelUpdateJSON(te, te->SaveToJSONNode());
    json_push_back(c, update);
  }

  json_char *jc = json_write_formatted(c);
  std::string json(jc);
  json_free(jc);
  json_delete(c);
  return json;
}

}   // namespace kopsik
