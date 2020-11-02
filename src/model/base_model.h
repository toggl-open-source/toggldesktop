// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_BASE_MODEL_H_
#define SRC_BASE_MODEL_H_

#include <string>
#include <vector>
#include <cstring>
#include <ctime>
#include <map>

#include <json/json.h>  // NOLINT

#include "const.h"
#include "types.h"
#include "util/logger.h"
#include "util/property.h"
#include "util/json.h"

#include <Poco/Types.h>

namespace toggl {

enum ModelErrors {
    ERROR_NAME_HAS_BEEN_TAKEN = ErrorBase::FIRST_AVAILABLE_ENUM,
    ERROR_NAME_ALREADY_EXISTS,
    ERROR_CANNOT_ACCESS_WORKSPACE,
    ERROR_CANNOT_ACCESS_PROJECT,
    ERROR_CANNOT_ACCESS_TASK,
    ERROR_IS_IN_ANOTHER_WORKSPACE, // This should probably be handled by the library and not shown to the user
    ERROR_ONLY_ADMINS_CAN_CHANGE_VISIBILITY,
    ERROR_DURATION_TOO_LARGE,
    ERROR_START_TIME_WRONG_YEAR,
    ERROR_STOP_TIME_BEFORE_START_TIME,
    ERROR_BILLABLE_IS_PREMIUM,
    ERROR_MISSING_CREATEDWITH,
    ERROR_TIME_ENTRY_LOCKED,
    ERROR_TIME_ENTRY_NOT_FOUND,
};
inline static const std::map<int, std::string> ModelErrorMessages {
    // why the hell are these two different?
    { ERROR_NAME_HAS_BEEN_TAKEN, "Name already exists" },
    { ERROR_NAME_ALREADY_EXISTS, "Name has already been taken" },
    { ERROR_CANNOT_ACCESS_WORKSPACE, "Cannot access workspace" }
};
inline static const std::multimap<int, std::string> ModelErrorRegexes {
    // again, I don't understand this but the message being checked was incomplete in commit 213d9876 so I'm sticking to it to be sure
    { ERROR_NAME_ALREADY_EXISTS, "(Client|Project|Task) name already.*" },
    { ERROR_CANNOT_ACCESS_WORKSPACE, "cannot access workspace" }
};

class ModelError : public EnumBasedError<ModelErrors, ModelErrorMessages, ModelErrorRegexes> {
public:
    using Parent = EnumBasedError<ModelErrors, ModelErrorMessages, ModelErrorRegexes>;
    using Parent::Parent;
    ModelError &operator=(const ModelError &o) = default;
    std::string Class() const override { return "ModelError"; }
};

enum ValidationErrors {
    ERROR_FOREIGN_ENTITY_LOST = ErrorBase::FIRST_AVAILABLE_ENUM,
};
inline static const std::map<int, std::string> ValidationErrorMessages {
    { ERROR_FOREIGN_ENTITY_LOST, "Assigned foreign entity could not be found" }
};

class ValidationError : public EnumBasedError<ValidationErrors, ValidationErrorMessages> {
public:
    using Parent = EnumBasedError<ValidationErrors, ValidationErrorMessages>;
    using Parent::Parent;
    ValidationError &operator=(const ValidationError &o) = default;

    std::string Class() const override { return "ValidationError"; }
};

class TOGGL_INTERNAL_EXPORT BaseModel {
 public:
    BaseModel() {}
    // The copy constructor expect the copy to be stored in the database - no ID is copied and the copy is marked as dirty
    BaseModel(const BaseModel &o);
    virtual ~BaseModel() {}

    Property<Poco::Int64> LocalID { 0 };
    Property<Poco::UInt64> ID { 0 };
    Property<guid> GUID { "" };
    Property<Poco::Int64> UIModifiedAt { 0 };
    Property<Poco::UInt64> UID { 0 };
    Property<bool> Dirty { false };
    Property<Poco::Int64> DeletedAt { 0 };
    Property<bool> IsMarkedAsDeletedOnServer { false };
    Property<Poco::Int64> UpdatedAt { 0 };

    // If model push to backend results in an error,
    // the error is attached to the model for later inspection.
    Property<class ValidationError> ValidationError { };

    // Flag is set only when sync fails.
    // Its for viewing purposes only. It should not
    // be used to check if a model needs to be
    // pushed to backend. It only means that some
    // attempt to push failed somewhere.
    Property<bool> Unsynced { false };

    void SetLocalID(Poco::Int64 value) {
        LocalID.Set(value);
    }
    void SetID(Poco::UInt64 value);
    void SetUIModifiedAt(Poco::Int64 value);
    void SetUIModified() {
        SetUIModifiedAt(time(nullptr));
    }

    void SetGUID(const std::string &value);
    void SetUID(Poco::UInt64 value);

    void SetDirty();
    void ClearDirty();

    void SetUnsynced();
    void ClearUnsynced();

    // Deleting a time entry hides it from
    // UI and flags it for removal from server:
    void SetDeletedAt(Poco::Int64 value);

    void SetUpdatedAt(Poco::Int64 value);

    std::string UpdatedAtString() const;
    void SetUpdatedAtString(const std::string &value);

    // When a model is deleted
    // on server, it will be removed from local
    // DB using this flag:
    void MarkAsDeletedOnServer();

    bool NeedsPush() const;
    bool NeedsPOST() const;
    bool NeedsPUT() const;
    bool NeedsDELETE() const;

    bool NeedsToBeSaved() const;

    void EnsureGUID();

    void ClearValidationError();
    void SetValidationError(const class ValidationError &value);

    virtual std::string String() const = 0;
    virtual std::string ModelName() const = 0;
    virtual std::string ModelURL() const = 0;

    virtual Json::Value SaveToJSON(int apiVersion = 8) const {
        return 0;
    }
    virtual std::string SyncType() const;
    virtual Json::Value SyncMetadata() const { return {}; }
    virtual Json::Value SyncPayload() const { return {}; }

    void Delete();

 protected:
    Logger logger() const;

    bool userCannotAccessWorkspace(const toggl::error &err) const;

};

}  // namespace toggl

#endif  // SRC_BASE_MODEL_H_
