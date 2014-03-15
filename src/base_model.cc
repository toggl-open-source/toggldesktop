// Copyright 2014 Toggl Desktop developers.

#include "./base_model.h"

#include <sstream>

#include "./formatter.h"
#include "./database.h"
#include "./model_change.h"

#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include "Poco/LocalDateTime.h"

namespace kopsik {

bool BaseModel::NeedsPush() const {
    return NeedsPOST() || NeedsPUT() || NeedsDELETE();
}

bool BaseModel::NeedsPOST() const {
    // No server side ID yet, meaning it's not POSTed yet
    return !id_ && !(deleted_at_ > 0);
}

bool BaseModel::NeedsPUT() const {
    // Model has been updated and is not deleted, needs a PUT
    return ui_modified_at_ > 0 && !(deleted_at_ > 0);
}

bool BaseModel::NeedsDELETE() const {
    // Model is deleted, needs a DELETE on server side
    return id_ && (deleted_at_ > 0);
}

bool BaseModel::NeedsToBeSaved() const {
  return !local_id_ || dirty_;
}

void BaseModel::EnsureGUID() {
    if (!guid_.empty()) {
        return;
    }
    SetGUID(Database::GenerateGUID());
}

void BaseModel::SetDeletedAt(const Poco::UInt64 value) {
    if (deleted_at_ != value) {
        deleted_at_ = value;
        dirty_ = true;
    }
}

void BaseModel::SetUpdatedAt(const Poco::UInt64 value) {
    if (updated_at_ != value) {
        updated_at_ = value;
        dirty_ = true;
    }
}

void BaseModel::SetGUID(const std::string value) {
    if (guid_ != value) {
        guid_ = value;
        dirty_ = true;
    }
}

void BaseModel::SetUIModifiedAt(const Poco::UInt64 value) {
    if (ui_modified_at_ != value) {
        ui_modified_at_ = value;
        dirty_ = true;
    }
}

void BaseModel::SetUID(const Poco::UInt64 value) {
    if (uid_ != value) {
        uid_ = value;
        dirty_ = true;
    }
}

void BaseModel::SetID(const Poco::UInt64 value) {
    if (id_ != value) {
        id_ = value;
        dirty_ = true;
    }
}

void BaseModel::SetUpdatedAtString(const std::string value) {
    SetUpdatedAt(Formatter::Parse8601(value));
}

void BaseModel::LoadFromDataString(const std::string data_string) {
  JSONNODE *n = json_parse(data_string.c_str());
  JSONNODE_ITERATOR i = json_begin(n);
  JSONNODE_ITERATOR e = json_end(n);
  while (i != e) {
    json_char *node_name = json_name(*i);
    if (strcmp(node_name, "data") == 0) {
      LoadFromJSONNode(*i);
    }
    ++i;
  }
  json_delete(n);
}

void BaseModel::LoadFromJSONString(const std::string json_string) {
  poco_assert(!json_string.empty());

  JSONNODE *root = json_parse(json_string.c_str());
  this->LoadFromJSONNode(root);
  json_delete(root);
}

void BaseModel::Delete() {
  SetDeletedAt(time(0));
  SetUIModified();
}

error BaseModel::ApplyBatchUpdateResult(
    BatchUpdateResult * const update) {
  poco_assert(update);

  if (update->ResourceIsGone()) {
    MarkAsDeletedOnServer();
    return noError;
  }

  kopsik::error err = update->Error();
  if (err != kopsik::noError) {
    if (DuplicateResource(err)) {
      MarkAsDeletedOnServer();
      return noError;
    }

    if (ResolveError(err)) {
      return noError;
    }

    SetError(err);
    return err;
  }

  poco_assert(json_is_valid(update->Body.c_str()));
  LoadFromDataString(update->Body);

  return noError;
}

}   // namespace kopsik
