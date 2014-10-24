// Copyright 2014 Toggl Desktop developers.

#include "./base_model.h"

#include <sstream>

#include "./formatter.h"
#include "./database.h"
#include "./model_change.h"

#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include "Poco/LocalDateTime.h"

namespace toggl {

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

error BaseModel::LoadFromDataString(const std::string data_string) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(data_string, root)) {
        return error("Failed to parse data string");
    }
    LoadFromJSON(root["data"]);
    return noError;
}

std::string BaseModel::SaveToJSONString() const {
    Json::StyledWriter writer;
    return writer.write(SaveToJSON());
}

error BaseModel::LoadFromJSONString(const std::string json_string) {
    if (json_string.empty()) {
        return noError;
    }
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json_string, root)) {
        return error("Failed to parse JSON string");
    }
    LoadFromJSON(root);
    return noError;
}

void BaseModel::Delete() {
    SetDeletedAt(time(0));
    SetUIModified();
}

error BaseModel::ApplyBatchUpdateResult(
    BatchUpdateResult * const update) {
    poco_check_ptr(update);

    if (update->ResourceIsGone()) {
        MarkAsDeletedOnServer();
        return noError;
    }

    toggl::error err = update->Error();
    if (err != toggl::noError) {
        if (DuplicateResource(err) || ResourceCannotBeCreated(err)) {
            MarkAsDeletedOnServer();
            return noError;
        }

        if (ResolveError(err)) {
            return noError;
        }

        SetError(err);
        return err;
    }

    return LoadFromDataString(update->Body);
}

bool BaseModel::userCannotAccessWorkspace(const toggl::error err) const {
    return (std::string::npos != std::string(err).find(
        "User cannot access workspace"));
}

std::string BaseModel::batchUpdateRelativeURL() const {
    if (NeedsPOST()) {
        return ModelURL();
    }

    std::stringstream url;
    url << ModelURL() << "/" << ID();
    return url.str();
}

std::string BaseModel::batchUpdateMethod() const {
    if (NeedsDELETE()) {
        return "DELETE";
    }

    if (NeedsPOST()) {
        return "POST";
    }

    return "PUT";
}

// Convert model JSON into batch update format.
Json::Value BaseModel::BatchUpdateJSON() {
    EnsureGUID();

    poco_assert(!GUID().empty());

    Json::Value body;
    body[ModelName()] = SaveToJSON();

    Json::Value update;
    update["method"] = batchUpdateMethod();
    update["relative_url"] = batchUpdateRelativeURL();
    update["guid"] = GUID();
    update["body"] = body;
    return update;
}

}   // namespace toggl
