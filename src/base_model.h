// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_BASE_MODEL_H_
#define SRC_BASE_MODEL_H_

#include <string>
#include <vector>
#include <cstring>
#include <ctime>

#include <json/json.h>  // NOLINT

#include "const.h"
#include "types.h"
#include "util/logger.h"
#include "util/memory.h"
#include "https_client.h"

#include <Poco/Types.h>
#include <Poco/Data/TypeHandler.h>
#include <Poco/Data/RecordSet.h>

namespace toggl {

class ProtectedBase;
class BatchUpdateResult;

class TOGGL_INTERNAL_EXPORT BaseModel {
 public:
    BaseModel(ProtectedBase *container)
        : container_(container) {}

    virtual ~BaseModel() {}

    ProtectedBase *GetContainer();
    const ProtectedBase *GetContainer() const;
    void SetContainer(ProtectedBase *container);
    RelatedData *GetRelatedData();
    const RelatedData *GetRelatedData() const;

    const Poco::Int64 &LocalID() const {
        return local_id_;
    }
    void SetLocalID(const Poco::Int64 value) {
        local_id_ = value;
    }

    const Poco::UInt64 &ID() const {
        return id_;
    }
    void SetID(const Poco::UInt64 value);

    const Poco::Int64 &UIModifiedAt() const {
        return ui_modified_at_;
    }
    void SetUIModifiedAt(const Poco::Int64 value);
    void SetUIModified() {
        SetUIModifiedAt(time(nullptr));
    }

    const std::string &GUID() const {
        return guid_;
    }
    void SetGUID(const std::string &value);

    const Poco::UInt64 &UID() const {
        return uid_;
    }
    void SetUID(const Poco::UInt64 value);

    void SetDirty();
    const bool &Dirty() const {
        return dirty_;
    }
    void ClearDirty() {
        dirty_ = false;
    }

    const bool &Unsynced() const {
        return unsynced_;
    }
    void SetUnsynced();
    void ClearUnsynced() {
        unsynced_ = false;
    }

    // Deleting a time entry hides it from
    // UI and flags it for removal from server:
    const Poco::Int64 &DeletedAt() const {
        return deleted_at_;
    }
    void SetDeletedAt(const Poco::Int64 value);

    const Poco::Int64 &UpdatedAt() const {
        return updated_at_;
    }
    void SetUpdatedAt(const Poco::Int64 value);

    std::string UpdatedAtString() const;
    void SetUpdatedAtString(const std::string &value);

    // When a model is deleted
    // on server, it will be removed from local
    // DB using this flag:
    bool IsMarkedAsDeletedOnServer() const {
        return is_marked_as_deleted_on_server_;
    }
    void MarkAsDeletedOnServer() {
        is_marked_as_deleted_on_server_ = true;
        SetDirty();
    }

    bool NeedsPush() const;
    bool NeedsPOST() const;
    bool NeedsPUT() const;
    bool NeedsDELETE() const;

    bool NeedsToBeSaved() const;

    void EnsureGUID();

    void ClearValidationError();
    void SetValidationError(const std::string &value);
    const std::string &ValidationError() const {
        return validation_error_;
    }

    virtual std::string String() const = 0;
    virtual std::string ModelName() const = 0;
    virtual std::string ModelURL() const = 0;

    virtual void LoadFromJSON(Json::Value value) {}
    error LoadFromJSONString(const std::string &json, bool with_id);
    virtual Json::Value SaveToJSON() const {
        return 0;
    }

    static std::string DatabaseTable() { return ""; }
    static std::list<std::string> DatabaseColumns() { return {"local_id", "id", "uid", "guid"}; };
    static size_t DatabaseColumnCount() { return DatabaseColumns().size(); }
    BaseModel(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : container_(container)
    {
        if (!rs[0].isEmpty())
            local_id_ = rs[0].convert<decltype(local_id_)>();
        if (!rs[1].isEmpty())
            id_ = rs[1].convert<decltype(id_)>();
        if (!rs[2].isEmpty())
            uid_ = rs[2].convert<decltype(uid_)>();
        if (!rs[3].isEmpty())
            guid_ = rs[3].convert<decltype(guid_)>();
        ClearDirty();
    }

    virtual HTTPSRequest PrepareRequest();

    virtual bool DuplicateResource(const toggl::error &err) const {
        return false;
    }
    virtual bool ResourceCannotBeCreated(const toggl::error &err) const {
        return false;
    }
    virtual error ResolveError(const toggl::error &err) {
        return err;
    }

    error LoadFromDataString(const std::string &);

    void Delete();

    error ApplyBatchUpdateResult(BatchUpdateResult * const);

    // Convert model JSON into batch update format.
    error BatchUpdateJSON(Json::Value *result) const;

 protected:
    Logger logger() const;

    bool userCannotAccessWorkspace(const toggl::error &err) const;

 protected:
    std::string batchUpdateRelativeURL() const;
    std::string batchUpdateMethod() const;

    ProtectedBase *container_ { nullptr };

    Poco::Int64 local_id_ { 0 };
    Poco::UInt64 id_ { 0 };
    Poco::UInt64 uid_ { 0 };
    guid guid_ { "" };
    Poco::Int64 ui_modified_at_ { 0 };
    Poco::Int64 deleted_at_ { 0 };
    Poco::Int64 updated_at_ { 0 };

    // If model push to backend results in an error,
    // the error is attached to the model for later inspection.
    std::string validation_error_ { "" };

    bool dirty_ { false };
    bool is_marked_as_deleted_on_server_ { false };
    // Flag is set only when sync fails.
    // Its for viewing purposes only. It should not
    // be used to check if a model needs to be
    // pushed to backend. It only means that some
    // attempt to push failed somewhere.
    bool unsynced_ { false };
};

}  // namespace toggl

#endif  // SRC_BASE_MODEL_H_
