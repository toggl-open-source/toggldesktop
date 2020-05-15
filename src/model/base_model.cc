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
    return !ID && !(DeletedAt > 0);
}

bool BaseModel::NeedsPUT() const {
    // Model has been updated and is not deleted, needs a PUT
    return ID && UIModifiedAt > 0 && !(DeletedAt > 0);
}

bool BaseModel::NeedsDELETE() const {
    // Model is deleted, needs a DELETE on server side
    return ID && (DeletedAt > 0);
}

bool BaseModel::NeedsToBeSaved() const {
    return !LocalID || IsDirty();
}

void BaseModel::EnsureGUID() {
    if (!GUID->empty()) {
        return;
    }
    GUID.Set(Database::GenerateGUID());
}

void BaseModel::SetUpdatedAtString(const std::string &value) {
    UpdatedAt.Set(Formatter::Parse8601(value));
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
    DeletedAt.Set(time(nullptr));
    SetUIModified();
}

error BaseModel::ApplyBatchUpdateResult(
    BatchUpdateResult * const update) {
    poco_check_ptr(update);

    if (update->ResourceIsGone()) {
        IsMarkedAsDeletedOnServer.Set(true);
        return noError;
    }

    toggl::error err = update->Error();
    if (err != toggl::noError) {
        if (DuplicateResource(err) || ResourceCannotBeCreated(err)) {
            IsMarkedAsDeletedOnServer.Set(true);
            return noError;
        }

        if (ResolveError(err)) {
            return noError;
        }

        ValidationError.Set(err);
        return err;
    }

    ValidationError.Set(noError);

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

}   // namespace toggl
