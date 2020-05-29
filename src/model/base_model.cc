// Copyright 2014 Toggl Desktop developers.

#include "model/base_model.h"

#include <sstream>

#include "batch_update_result.h"
#include "database/database.h"
#include "util/formatter.h"
#include "model_change.h"

#include <Poco/Timestamp.h>
#include <Poco/DateTime.h>
#include <Poco/LocalDateTime.h>

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
    return !ID() && !(DeletedAt() > 0);
}

bool BaseModel::NeedsPUT() const {
    // Model has been updated and is not deleted, needs a PUT
    return ID() && UIModifiedAt() > 0 && !(DeletedAt() > 0);
}

bool BaseModel::NeedsDELETE() const {
    // Model is deleted, needs a DELETE on server side
    return ID() && (DeletedAt() > 0);
}

bool BaseModel::NeedsToBeSaved() const {
    return !LocalID() || Dirty();
}

void BaseModel::EnsureGUID() {
    if (!GUID().empty()) {
        return;
    }
    SetGUID(Database::GenerateGUID());
}

void BaseModel::ClearValidationError() {
    SetValidationError(noError);
}

void BaseModel::SetValidationError(const std::string &value) {
    ValidationError.Set(value);
    SetDirty();
}

void BaseModel::SetUpdatedAtString(const std::string &value) {
    SetUpdatedAt(Formatter::Parse8601(value));
}

void BaseModel::MarkAsDeletedOnServer() {
    IsMarkedAsDeletedOnServer.Set(true);
    SetDirty();
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

Logger BaseModel::logger() const {
    return { ModelName() };
}

void BaseModel::SetID(const Poco::UInt64 value) {
    ID.Set(value);
    SetDirty();
}

void BaseModel::SetUIModifiedAt(const Poco::Int64 value) {
    UIModifiedAt.Set(value);
    SetDirty();
}

void BaseModel::SetGUID(const std::string &value) {
    GUID.Set(value);
    SetDirty();
}

void BaseModel::SetUID(const Poco::UInt64 value) {
    UID.Set(value);
    SetDirty();
}

void BaseModel::SetDirty() {
    Dirty.Set(true);
}

void BaseModel::ClearDirty() {
    Dirty.Set(false);
}

void BaseModel::SetUnsynced() {
    Unsynced.Set(true);
}

void BaseModel::ClearUnsynced() {
    Unsynced.Set(false);
}

void BaseModel::SetDeletedAt(const Poco::Int64 value) {
    DeletedAt.Set(value);
    SetDirty();
}

void BaseModel::SetUpdatedAt(const Poco::Int64 value) {
    UpdatedAt.Set(value);
    SetDirty();
}

}   // namespace toggl
