// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_BASE_MODEL_H_
#define SRC_BASE_MODEL_H_

#include <string>
#include <vector>
#include <cstring>
#include <ctime>
#include <array>

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

    typedef std::vector<std::string> Join;
    typedef std::string Table;
    typedef std::vector<std::string> Columns;
    typedef std::vector<std::string> OrderBy;
    /**
     * @brief The Query struct
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
    struct Query {
        Table table_ {};
        Columns columns_ {};
        Join join_ {};
        OrderBy order_ {};
        const Query *parent_ { nullptr };

        /**
         * @brief ToSelect
         * @param where - which column will be supplied outside as the WHERE condition
         * @return string with the completed SELECT query
         */
        std::string ToSelect(const std::string &where = "uid") const {
            std::ostringstream ss;
            /*
             * columns
             */
            ss << "SELECT ";
            bool firstColumn = true;
            if (parent_)
                printColumns(ss, firstColumn, parent_->columns_);
            printColumns(ss, firstColumn, columns_);
            /*
             * table name
             */
            ss << " FROM " << table_;
            /*
             * join (optional)
             */
            for (auto i : join_)
                ss << " " <<  i;
            /*
             * condition (actual value supplied outside as a reference)
             */
            ss << " WHERE " << table_ << "." << where << " = :" << where;
            /*
             * order (optional)
             */
            if (!order_.empty()) {
                bool firstOrder = true; // disney ruined star wars :(
                ss << " ORDER BY ";
                printColumns(ss, firstOrder, order_);
            }
            ss << ";";
            return ss.str();
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
                if (!join_.empty() && i.find(".") == std::string::npos)
                    ss << table_ << ".";
                ss << i;
            }
        };
    };
protected:
    inline static const Query query {
        Table(),
        Columns({"local_id", "id", "uid", "guid"}),
        Join(),
        OrderBy(),
        nullptr
    };
public:
/*
    template<typename T, typename U >
    struct Boo {
        Boo(U T::*x) {
            boo = [x]() {
                return *T->x;
            }
        }
        std::function<U(T)> boo;
    };

    Boo ahoj { &BaseModel::local_id_ };
    */
    struct Foo {
        Foo(Poco::Int64 BaseModel::*ptr) {
            std::function<Poco::Int64 (BaseModel*)> b = [ptr](BaseModel *that) {
                return (that->*ptr);
            };
        }
    };
    Foo foo { &BaseModel::local_id_ };

    typedef std::function<bool(void *that, Poco::Data::RecordSet &rs, int index)> db_load_t;
    class Binding {
    public:
        enum Type {
            REQUIRED = 0,
            OPTIONAL
        };
        std::string column_;
        db_load_t load_;
    };

    template <typename Value, typename Class>
    static Binding Bind(const std::string &column, Value Class::*ptr, Binding::Type required) {
        return {
            column,
            [ptr, required](void *that, Poco::Data::RecordSet &rs, int index) {
                auto actuallyThat = reinterpret_cast<Class*>(that);
                if (!rs[index].isEmpty() || required) {
                    (actuallyThat->*ptr) = rs[index].convert<Value>();
                    return true;
                }
                return false;
            }
        };
    }


    /*
    inline static const std::vector<BindBase*> bindings {
        new Binding<Poco::Int64, BaseModel>("local_id", &BaseModel::local_id_);
    };
    */

    //Bind<Poco::Int64, BaseModel> ugh { "local_id", &BaseModel::local_id_ };

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

    static std::string DatabaseTable() { return {}; }
    static std::list<std::string> DatabaseColumns() { return {"local_id", "id", "uid", "guid"}; };
    static std::string DatabaseJoin() { return {}; }
    BaseModel(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : container_(container)
    {
        loadFromDatabase(rs, 0, local_id_);
        loadFromDatabase(rs, 1, id_, false);
        loadFromDatabase(rs, 2, uid_);
        loadFromDatabase(rs, 3, guid_, false);
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

    /**
     * @brief loadFromDatabase - Loads a value from database, automatically deducing its type
     * @param rs - Poco RecordSet (database query result)
     * @param index - Column index to load from
     * @param member - Reference to a variable to load the value into
     * @param required - If true (default), any error (null value, wrong conversion) will make the method throw Poco exceptions
     */
    template<typename T>
    bool loadFromDatabase(Poco::Data::RecordSet &rs, size_t index, T &member, bool required = true) {
        if (!rs[index].isEmpty() || required) {
            member = rs[index].convert<T>();
            return true;
        }
        return false;
    }

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

    inline static std::vector<Binding> whatever {
        Bind("local_id", &BaseModel::local_id_, Binding::REQUIRED),
        Bind("id", &BaseModel::id_, Binding::OPTIONAL),
        Bind("uid", &BaseModel::uid_, Binding::REQUIRED),
        Bind("guid", &BaseModel::guid_, Binding::OPTIONAL),
    };
};

}  // namespace toggl

#endif  // SRC_BASE_MODEL_H_
