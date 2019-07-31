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

template <typename T>
class locked : public std::unique_lock<std::recursive_mutex> {
public:
    locked()
        : std::unique_lock<std::recursive_mutex>()
        , data_(nullptr)
    {}
    locked(mutex_type &mutex, T *data)
        : std::unique_lock<std::recursive_mutex>(mutex)
        , data_(data)
    {}
    T* data() { return data_; }
    T *operator->() { return data_; }
    T &operator*() { return *data_; }
    operator bool() const {
        return owns_lock() && data_;
    }
private:
    T *data_;
};

template <typename T>
class locked_impl {
public:
    locked<std::set<T*>> operator()() {
        return { mutex_, &container_ };
    }
    locked<const std::set<T*>> operator()() const {
        return { mutex_, &container_ };
    }
    void clear(bool deleteItems = true) {
        auto lockedContainer = (*this)();
        if (deleteItems) {
            for (auto i : *lockedContainer)
                delete i;
        }
        lockedContainer->clear();
    }
    locked<T> create() {
        T *val = new T();
        (*this)()->insert(val);
        return { mutex_, val };
    }
    template<typename U> locked<U> make_locked(U *val) {
        return { mutex_, val };
    }
    template<typename U> locked<const U> make_locked(const U *val) const {
        return { mutex_, val };
    }
    locked<T> findByID(Poco::UInt64 id) {
        auto lockedContainer = (*this)();
        for (auto i : *lockedContainer) {
            if (i->ID() == id)
                return { mutex_, i };
        }
        return {};
    }
    locked<const T> findByID(Poco::UInt64 id) const {
        auto lockedContainer = (*this)();
        for (auto i : *lockedContainer) {
            if (i->ID() == id)
                return { mutex_, i };
        }
        return {};
    }
    locked<T> findByGUID(const std::string &guid) {
        auto lockedContainer = (*this)();
        for (auto i : *lockedContainer) {
            if (i->GUID() == guid)
                return { mutex_, i };
        }
        return {};
    }
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
