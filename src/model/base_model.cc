// Copyright 2014 Toggl Desktop developers.

#include "model/base_model.h"

#include <sstream>

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
    Unsynced.Set(!value.empty());
    if (ValidationError.Set(value))
        SetDirty();
}

std::string BaseModel::SyncType() const {
    if (NeedsPOST())
        return "create";
    else if (NeedsPUT())
        return "update";
    else if (NeedsDELETE())
        return "delete";
    return {};
}

void BaseModel::SetUpdatedAtString(const std::string &value) {
    SetUpdatedAt(Formatter::Parse8601(value));
}

void BaseModel::MarkAsDeletedOnServer() {
    if (IsMarkedAsDeletedOnServer.Set(true))
        SetDirty();
}

void BaseModel::Delete() {
    SetDeletedAt(time(nullptr));
    SetUIModified();
}

bool BaseModel::userCannotAccessWorkspace(const error &err) const {
    return (std::string::npos != std::string(err).find(
        kCannotAccessWorkspaceError));
}

Logger BaseModel::logger() const {
    return { ModelName() };
}

BaseModel::BaseModel(const BaseModel &o)
    // ID, GUID, LocalID are intentionally omitted
    : UIModifiedAt { o.UIModifiedAt }
    , UID { o.UID }
    , Dirty { true }
    , DeletedAt { o.DeletedAt }
    , IsMarkedAsDeletedOnServer { o.IsMarkedAsDeletedOnServer }
    , UpdatedAt { o.UpdatedAt }
    , ValidationError { o.ValidationError }
    , Unsynced { o.Unsynced }
{
}

void BaseModel::SetID(Poco::UInt64 value) {
    if (ID.Set(value))
        SetDirty();
}

void BaseModel::SetUIModifiedAt(Poco::Int64 value) {
    if (UIModifiedAt.Set(value))
        SetDirty();
}

void BaseModel::SetGUID(const std::string &value) {
    if (GUID.Set(value))
        SetDirty();
}

void BaseModel::SetUID(Poco::UInt64 value) {
    if (UID.Set(value))
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
    ClearValidationError();
}

void BaseModel::SetDeletedAt(Poco::Int64 value) {
    if (DeletedAt.Set(value))
        SetDirty();
}

void BaseModel::SetUpdatedAt(Poco::Int64 value) {
    if (UpdatedAt.Set(value))
        SetDirty();
}

}   // namespace toggl
