// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_RELATED_DATA_H_
#define SRC_RELATED_DATA_H_

#include <vector>
#include <set>
#include <string>
#include <map>
#include <functional>

#include "./timeline_event.h"
#include "./types.h"

#include "Poco/Mutex.h"
#include <functional>
#include <thread>
#include <mutex>

namespace toggl {

class AutotrackerRule;
class Client;
class ObmAction;
class ObmExperiment;
class Project;
class Tag;
class Task;
class TimeEntry;
class Workspace;

namespace view {
class TimeEntry;
};

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
 * @class locked_impl
 * \brief Contains convenience methods and a set of data to be protected with a single mutex.
 *
 * This class is intended to be used especially with BaseModel-based objects (TimeEntry, Project, etc.)
 * It also provides facilities to lock other objects using the internal mutex.
 */
template <typename T>
class locked_impl {
public:
    /**
     * @brief operator () - Convenience method to access the internal locked container directly
     * @return A @ref locked object containing a map of <typename T> types
     */
    locked<std::set<T*>> operator()() {
        return { mutex_, &container_ };
    }
    /**
     * @brief operator () - Convenience method to access the internal locked container directly
     * @return A @ref locked object containing a map of <typename T> types
     */
    locked<const std::set<T*>> operator()() const {
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
     */
    locked<T> create() {
        T *val = new T();
        (*this)()->insert(val);
        return { mutex_, val };
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
    std::set<T*> container_;
    mutable std::recursive_mutex mutex_;
};


class RelatedData {
public:
    locked_impl<Workspace> Workspaces;
    locked_impl<Client> Clients;
    locked_impl<Project> Projects;
    locked_impl<Task> Tasks;
    locked_impl<Tag> Tags;
    locked_impl<TimeEntry> TimeEntries;
    locked_impl<AutotrackerRule> AutotrackerRules;
    locked_impl<TimelineEvent> TimelineEvents;
    locked_impl<ObmAction> ObmActions;
    locked_impl<ObmExperiment> ObmExperiments;

    void Clear();

    void TagList(std::vector<std::string> *result, Poco::UInt64 wid) const;
    locked<std::vector<Workspace*>> WorkspaceList();
    locked<std::vector<Client*>> ClientList();

    Poco::Int64 NumberOfUnsyncedTimeEntries() const;

    // Find the time entry that was stopped most recently
    locked<const TimeEntry> LatestTimeEntry() const;

    // Collect visible timeline events
    locked<std::vector<TimelineEvent *> > VisibleTimelineEvents();

    // Collect visible time entries
    locked<std::vector<TimeEntry *>> VisibleTimeEntries();

    Poco::Int64 TotalDurationForDate(const TimeEntry &match) const;

    // avoid duplicates
    bool HasMatchingAutotrackerRule(const std::string &lowercase_term) const;

    error DeleteAutotrackerRule(Poco::Int64 local_id);

    void TimeEntryAutocompleteItems(std::vector<view::Autocomplete> *) const;
    void MinitimerAutocompleteItems(std::vector<view::Autocomplete> *) const;
    void ProjectAutocompleteItems(std::vector<view::Autocomplete> *) const;

    void ProjectLabelAndColorCode(
        const TimeEntry &te,
        view::TimeEntry *view) const;

    AutotrackerRule *FindAutotrackerRule(const TimelineEvent &event) const;

    locked<Client> clientByProject(const Project &p);
    locked<const Client> clientByProject(const Project &p) const;

 private:
    void timeEntryAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<view::Autocomplete> *list,
        std::map<std::string, std::vector<view::Autocomplete> > *items) const;

    void taskAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<view::Autocomplete> *list,
        std::map<Poco::Int64, std::vector<view::Autocomplete> > *items) const;

    void projectAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<view::Autocomplete> *list,
        std::map<std::string, std::vector<view::Autocomplete> > *items,
        std::map<Poco::Int64, std::vector<view::Autocomplete> > *task_items) const;

    void workspaceAutocompleteItems(
        std::set<std::string> *unique_names,
        std::map<Poco::UInt64, std::string> *ws_names,
        std::vector<view::Autocomplete> *list) const;

    void mergeGroupedAutocompleteItems(
        std::vector<view::Autocomplete> *result,
        std::map<std::string, std::vector<view::Autocomplete> > *items) const;
};

template<typename T>
void clearList(std::set<T *> &list);

}  // namespace toggl

#endif  // SRC_RELATED_DATA_H_
