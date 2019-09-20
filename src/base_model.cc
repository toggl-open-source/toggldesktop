// Copyright 2014 Toggl Desktop developers.

#include "../src/base_model.h"

#include <sstream>

#include "./batch_update_result.h"
#include "./database.h"
#include "./formatter.h"
#include "./model_change.h"

#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include "Poco/LocalDateTime.h"
#include "Poco/Logger.h"

namespace toggl {

bool BaseModel::NeedsPush() const {
    // Note that if a model has a validation error previously
    // received and attached from the backend, the model won't be
    // pushed again unless the error is somehow fixed by user.
    // We will assume that if user modifies the model, the error
    // will go away. But until then, don't push the errored data.
    return ValidationError().empty() &&
           (NeedsPOST() || NeedsPUT() || NeedsDELETE());
}

bool BaseModel::NeedsPOST() const {
    // No server side ID yet, meaning it's not POSTed yet
    return !id_ && !(deleted_at_ > 0);
}

bool BaseModel::NeedsPUT() const {
    // Model has been updated and is not deleted, needs a PUT
    return id_ && ui_modified_at_ > 0 && !(deleted_at_ > 0);
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

void BaseModel::ClearValidationError() {
    SetValidationError(noError);
}

void BaseModel::SetValidationError(const std::string &value) {
    if (validation_error_ != value) {
        validation_error_ = value;
        SetDirty();
    }
}

void BaseModel::SetDeletedAt(const Poco::Int64 value) {
    if (deleted_at_ != value) {
        deleted_at_ = value;
        SetDirty();
    }
}

void BaseModel::SetUpdatedAt(const Poco::Int64 value) {
    if (updated_at_ != value) {
        updated_at_ = value;
        SetDirty();
    }
}

void BaseModel::SetGUID(const std::string &value) {
    if (guid_ != value) {
        guid_ = value;
        SetDirty();
    }
}

void BaseModel::SetUIModifiedAt(const Poco::Int64 value) {
    if (ui_modified_at_ != value) {
        ui_modified_at_ = value;
        SetDirty();
    }
}

void BaseModel::SetUID(const Poco::UInt64 value) {
    if (uid_ != value) {
        uid_ = value;
        SetDirty();
    }
}

void BaseModel::SetID(const Poco::UInt64 value) {
    if (id_ != value) {
        id_ = value;
        SetDirty();
    }
}

void BaseModel::SetUpdatedAtString(const std::string &value) {
    SetUpdatedAt(Formatter::Parse8601(value));
}

error BaseModel::LoadFromDataString(const std::string &data_string) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(data_string, root)) {
        return error("Failed to parse data string");
    }
    LoadFromJSON(root["data"]);
    return noError;
}

void BaseModel::Delete() {
    SetDeletedAt(time(nullptr));
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

        SetValidationError(err);
        return err;
    }

    SetValidationError(noError);

    return LoadFromDataString(update->Body);
}

bool BaseModel::userCannotAccessWorkspace(const error &err) const {
    return (std::string::npos != std::string(err).find(
        kCannotAccessWorkspaceError));
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
error BaseModel::BatchUpdateJSON(Json::Value *result) const {
    if (GUID().empty()) {
        return error("Cannot export model to batch update without a GUID");
    }

    Json::Value body;
    body[ModelName()] = SaveToJSON();

    (*result)["method"] = batchUpdateMethod();
    (*result)["relative_url"] = batchUpdateRelativeURL();
    (*result)["guid"] = GUID();
    (*result)["body"] = body;

    return noError;
}

Poco::Logger &BaseModel::logger() const {
    return Poco::Logger::get(ModelName());
}

void BaseModel::SetDirty() {
    dirty_ = true;
}

void BaseModel::SetUnsynced() {
    unsynced_ = true;
}

}   // namespace toggl
