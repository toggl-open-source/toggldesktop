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
#include <Poco/Data/RecordSet.h>

namespace toggl {

class ProtectedBase;
class BatchUpdateResult;

/**
 * @brief The BaseModelQuery struct
 * Use to determine how to retrieve data from the database. Each BaseModel class should implement this and
 * reference BaseModel::query as the @ref parent_ pointer.
 *
 * Only the @ref columns_ list will be prepended to the implementation.
 * That means, if you do this:
 *   In BaseModel: Query { "", { "base1", "base2"}, ...
 *   In Client: { "clients", { "client1", "client2" }, ...
 * Then Client query will work with the "clients" table and look for columns "base1", "base2", "client1", "client2" (in this order).
 * This is to have things like ID, Local ID and GUID in the base class for all models here in one place
 * and to not have to handle their serialization in all child classes separately.
 */
struct BaseModelQuery {

    struct Column {
        std::string name;
        bool required;
        // so printColumns still works
        explicit operator std::string() const {
            return name;
        }
    };
    typedef std::vector<std::string> Join;
    typedef std::string Table;
    typedef std::vector<Column> Columns;
    typedef std::vector<std::string> OrderBy;

    Table table_ {};
    Columns columns_ {};
    Join join_ {};
    OrderBy order_ {};
    const BaseModelQuery *parent_ { nullptr };

    /**
     * @brief ToSelect
     * @param where - which column will be supplied outside as the WHERE condition
     * @return string with the completed SELECT query
     */
    std::string ToSelect(const std::string &where = "uid") const;

    size_t ColumnCount() const {
        return columns_.size();
    }

    size_t Offset() const {
        if (parent_)
            return parent_->ColumnCount();
        return 0;
    }

    bool IsRequired(size_t idx) const {
        if (parent_ && idx >= Offset()) {
            return parent_->IsRequired(idx - Offset());
        }
        return columns_[idx].required;
    }

    /**
     * @brief writes a list of columns to @ref ss
     * Handles case when there's multiple tables and prepends "main" table name to columns without a dot
     */
    template <typename T>
    void printColumns(std::ostringstream &ss, bool &first, T &list) const {
        for (auto i : list) {
            if (!first)
                ss << ", ";
            first = false;
            if (!join_.empty() && std::string(i).find(".") == std::string::npos)
                ss << table_ << ".";
            ss << std::string(i);
        }
    };
};

class TOGGL_INTERNAL_EXPORT BaseModel {
 protected:
    using Query = BaseModelQuery;
    inline static std::string modelName {};
    inline static const Query query {
        Query::Table(),
        Query::Columns { 
            { "local_id", true },
            { "id", false },
            { "uid", true },
            { "guid", false },
        },
        Query::Join(),
        Query::OrderBy(),
        nullptr
    };

    template <typename T>
    bool load(Poco::Data::RecordSet &rs, bool required, size_t index, T& member) {
        if (!rs[index].isEmpty() || required) {
            member = rs[index].convert<T>();
            return true;
        }
        return false;
    }

    BaseModel(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : container_(container)
    {
        size_t ptr { query.Offset() };
        load(rs, query.IsRequired(ptr), ptr++, local_id_);
        load(rs, query.IsRequired(ptr), ptr++, id_);
        load(rs, query.IsRequired(ptr), ptr++, uid_);
        load(rs, query.IsRequired(ptr), ptr++, guid_);
        ClearDirty();
    }

    BaseModel(ProtectedBase* container)
        : container_(container)
    {}
    virtual ~BaseModel() {}
 public:

    ProtectedBase *GetContainer();
    const ProtectedBase *GetContainer() const;
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

 private:
    std::string batchUpdateRelativeURL() const;
    std::string batchUpdateMethod() const;

    ProtectedBase *container_ { nullptr };

    Poco::Int64 local_id_ { 0 };
    Poco::UInt64 id_ { 0 };
    Poco::Int64 ui_modified_at_ { 0 };
    Poco::UInt64 uid_ { 0 };
    Poco::Int64 deleted_at_ { 0 };
    Poco::Int64 updated_at_ { 0 };
    guid guid_ { "" };

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
