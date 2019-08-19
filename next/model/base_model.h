// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_BASE_MODEL_H_
#define SRC_BASE_MODEL_H_

#include "const.h"
#include "types.h"

#include <string>
#include <vector>
#include <cstring>
#include <ctime>

#include <mutex>
#include <set>

#include <json/json.h>
#include <Poco/Types.h>

namespace Poco {
class Logger;
}

namespace toggl {

class BatchUpdateResult;

/**
 * @class locked
 * \brief A thread-safe wrapper class. Contains a unique_lock that gets unlocked when it goes out of scope
 *
 */
template <typename T>
class locked : public std::unique_lock<std::recursive_mutex> {
public:
    /**
     * @brief Constructs an empty invalid locked object
     */
    locked()
        : std::unique_lock<std::recursive_mutex>()
        , data_(nullptr)
    {}
    /**
     * @brief Constructs a valid locked object
     * @param mutex - Needs to be a valid mutex, the application will block if it's already locked in a different thread
     * @param data - Pointer to data to protect
     */
    locked(mutex_type &mutex, T *data)
        : std::unique_lock<std::recursive_mutex>(mutex)
        , data_(data)
    {}
    T* data() { return data_; }
    T *operator->() { return data_; }
    T &operator*() { return *data_; }
    /**
     * @brief operator bool - Returns true if the mutex is locked and the pointer is not null
     */
    operator bool() const {
        return owns_lock() && data_;
    }
private:
    T *data_;
};

/**
 * @class ProtectedModel
 * \brief Contains convenience methods and a set of data to be protected with a single mutex.
 *
 * This class is intended to be used especially with BaseModel-based objects (TimeEntry, Project, etc.)
 * It also provides facilities to lock other objects using the internal mutex.
 */
template <typename T>
class ProtectedModel {
public:
    /**
     * @brief operator () - Convenience method to access the internal locked container directly
     * @return A @ref locked object containing a map of <typename T> types
     */
    locked<std::vector<T*>> operator()() {
        return { mutex_, &container_ };
    }
    /**
     * @brief operator () - Convenience method to access the internal locked container directly
     * @return A @ref locked object containing a map of <typename T> types
     */
    locked<const std::vector<T*>> operator()() const {
        return { mutex_, &container_ };
    }
    /**
     * @brief clear - Clear the @ref container_
     * @param deleteItems - Set to true if the pointers contained in the @ref container_ should be deleted, too
     */
    void clear(bool deleteItems = true) {
        auto lockedContainer = (*this)();
        if (deleteItems) {
            for (auto i : *lockedContainer)
                delete i;
        }
        lockedContainer->clear();
    }
    /**
     * @brief create - Allocate a new instance of <typename T>
     * @return - a @ref locked new instance of T
     * OVERHAUL TODO: this could be more efficient
     */
    locked<T> create() {
        T *val = new T();
        (*this)()->push_back(val);
        return { mutex_, val };
    }
    /**
     * @brief size - Get how many items are contained inside
     * @return - number of items inside the container
     */
    size_t size() {
        auto lockedContainer = (*this)();
        return lockedContainer->size();
    }
    /**
     * @brief make_locked - Makes pointer to any type locked by the internal mutex
     * @param val - can be of any type (but a pointer)
     *
     * @warning beware of using pointers to non-static local variables with this method
     */
    template<typename U> locked<U> make_locked(U *val) {
        return { mutex_, val };
    }
    /**
     * @brief make_locked - Makes pointer to any type locked by the internal mutex
     * @param val - can be of any type (but a pointer)
     *
     * @warning beware of using pointers to non-static local variables with this method
     */
    template<typename U> locked<const U> make_locked(const U *val) const {
        return { mutex_, val };
    }
    /**
     * @brief findByID - Finds a contained BaseModel instance by looking for its ID
     * @param id - ID to look for
     * @return - valid @ref locked object containing a pointer to the instance if found, invalid @ref locked object if not found
     */
    locked<T> findByID(Poco::UInt64 id) {
        auto lockedContainer = (*this)();
        for (auto i : *lockedContainer) {
            if (i->ID() == id)
                return { mutex_, i };
        }
        return {};
    }
    /**
     * @brief findByID - Finds a contained BaseModel instance by looking for its ID
     * @param id - ID to look for
     * @return - valid @ref locked object containing a pointer to the instance if found, invalid @ref locked object if not found
     */
    locked<const T> findByID(Poco::UInt64 id) const {
        auto lockedContainer = (*this)();
        for (auto i : *lockedContainer) {
            if (i->ID() == id)
                return { mutex_, i };
        }
        return {};
    }
    /**
     * @brief findByID - Finds a contained BaseModel instance by looking for its GUID
     * @param guid - GUID to look for
     * @return - valid @ref locked object containing a pointer to the instance if found, invalid @ref locked object if not found
     */
    locked<T> findByGUID(const std::string &guid) {
        auto lockedContainer = (*this)();
        for (auto i : *lockedContainer) {
            if (i->GUID() == guid)
                return { mutex_, i };
        }
        return {};
    }
    /**
     * @brief findByID - Finds a contained BaseModel instance by looking for its GUID
     * @param guid - GUID to look for
     * @return - valid @ref locked object containing a pointer to the instance if found, invalid @ref locked object if not found
     */
    locked<const T> findByGUID(const std::string &guid) const {
        auto lockedContainer = (*this)();
        for (auto i : *lockedContainer) {
            if (i->GUID() == guid)
                return { mutex_, i };
        }
        return {};
    }
private:
    std::vector<T*> container_;
    mutable std::recursive_mutex mutex_;
};

class BaseModel {
 public:
    BaseModel()
        : local_id_(0)
    , id_(0)
    , guid_("")
    , ui_modified_at_(0)
    , uid_(0)
    , dirty_(false)
    , deleted_at_(0)
    , is_marked_as_deleted_on_server_(false)
    , updated_at_(0)
    , validation_error_("")
    , unsynced_(false) {}

    virtual ~BaseModel() {}

    const int64_t &LocalID() const {
        return local_id_;
    }
    void SetLocalID(const int64_t value) {
        local_id_ = value;
    }

    const uint64_t &ID() const {
        return id_;
    }
    void SetID(const uint64_t value);

    const int64_t &UIModifiedAt() const {
        return ui_modified_at_;
    }
    void SetUIModifiedAt(const int64_t value);
    void SetUIModified() {
        SetUIModifiedAt(time(nullptr));
    }

    const std::string &GUID() const {
        return guid_;
    }
    void SetGUID(const std::string value);

    const uint64_t &UID() const {
        return uid_;
    }
    void SetUID(const uint64_t value);

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
    const int64_t &DeletedAt() const {
        return deleted_at_;
    }
    void SetDeletedAt(const int64_t value);

    const int64_t &UpdatedAt() const {
        return updated_at_;
    }
    void SetUpdatedAt(const int64_t value);

    std::string UpdatedAtString() const;
    void SetUpdatedAtString(const std::string value);

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
    void SetValidationError(const std::string value);
    const std::string &ValidationError() const {
        return validation_error_;
    }

    virtual std::string String() const = 0;
    virtual std::string ModelName() const = 0;
    virtual std::string ModelURL() const = 0;

    virtual void LoadFromJSON(Json::Value value) {}
    virtual Json::Value SaveToJSON() const {
        return 0;
    }

    virtual bool DuplicateResource(const toggl::error) const {
        return false;
    }
    virtual bool ResourceCannotBeCreated(const toggl::error) const {
        return false;
    }
    virtual bool ResolveError(const toggl::error) {
        return false;
    }

    error LoadFromDataString(const std::string);

    void Delete();

    error ApplyBatchUpdateResult(BatchUpdateResult * const);

    // Convert model JSON into batch update format.
    error BatchUpdateJSON(Json::Value *result) const;

 protected:
    Poco::Logger &logger() const;

    bool userCannotAccessWorkspace(const toggl::error err) const;

 private:
    std::string batchUpdateRelativeURL() const;
    std::string batchUpdateMethod() const;

    int64_t local_id_;
    uint64_t id_;
    std::string guid_;
    int64_t ui_modified_at_;
    uint64_t uid_;
    bool dirty_;
    int64_t deleted_at_;
    bool is_marked_as_deleted_on_server_;
    int64_t updated_at_;

    // If model push to backend results in an error,
    // the error is attached to the model for later inspection.
    std::string validation_error_;

    // Flag is set only when sync fails.
    // Its for viewing purposes only. It should not
    // be used to check if a model needs to be
    // pushed to backend. It only means that some
    // attempt to push failed somewhere.
    bool unsynced_;
};

}  // namespace toggl

#endif  // SRC_BASE_MODEL_H_
