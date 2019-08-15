#ifndef SRC_USER_DATA_H_
#define SRC_USER_DATA_H_

#include "model/user.h"

namespace toggl {

class TimeEntry;
class Workspace;
class Project;
class Task;
class Tag;
class Client;
class TimelineEvent;

class UserData : public User {

    bool CanSeeBillable(
        const Workspace *ws) const;

    //RelatedData related;

    TimeEntry *RunningTimeEntry() const;
    bool IsTracking() const {
        return RunningTimeEntry() != nullptr;
    }

    TimeEntry *Start(
        const std::string description,
        const std::string duration,
        const Poco::UInt64 task_id,
        const Poco::UInt64 project_id,
        const std::string project_guid,
        const std::string tags);

    TimeEntry *Continue(
        const std::string GUID,
        const bool manual_mode);

    void Stop(std::vector<TimeEntry *> *stopped = nullptr);

    // Discard time. Return a new time entry if
    // the discarded time was split into a new time entry
    TimeEntry *DiscardTimeAt(
        const std::string guid,
        const Poco::Int64 at,
        const bool split_into_new_entry);

    Project *CreateProject(
        const Poco::UInt64 workspace_id,
        const Poco::UInt64 client_id,
        const std::string client_guid,
        const std::string client_name,
        const std::string project_name,
        const bool is_private,
        const std::string project_color,
        const bool billable);

    void AddProjectToList(Project *p);

    Client *CreateClient(
        const Poco::UInt64 workspace_id,
        const std::string client_name);

    void AddClientToList(Client *c);

    std::string DateDuration(TimeEntry *te) const;

    void MarkTimelineBatchAsUploaded(
        const std::vector<TimelineEvent> &events);
    void CompressTimeline();
    std::vector<TimelineEvent> CompressedTimeline() const;

    error UpdateJSON(
        std::vector<TimeEntry *> * const,
        std::string *result) const;


    std::string dirtyObjectsJSON(std::vector<TimeEntry *> * const) const;

    void processResponseArray(
        std::vector<BatchUpdateResult> * const results,
        std::vector<TimeEntry *> *dirty,
        std::vector<error> *errors);


    template<class T>
    void deleteZombies(
        const std::vector<T> &list,
        const std::set<Poco::UInt64> &alive);

    template <typename T>
    void deleteRelatedModelsWithWorkspace(const Poco::UInt64 wid,
                                          std::vector<T *> *list);

    template <typename T>
    void removeProjectFromRelatedModels(const Poco::UInt64 pid,
                                        std::vector<T *> *list);


    template<typename T>
    void EnsureWID(T *model) const {
        /*
        // Do nothing if TE already has WID assigned
        if (model->WID()) {
            return;
        }

        // Try to set default user WID
        if (DefaultWID()) {
            model->SetWID(DefaultWID());
            return;
        }

        // Try to set first WID available
        std::vector<Workspace *>::const_iterator it =
            related.Workspaces.begin();
        if (it != related.Workspaces.end()) {
            Workspace *ws = *it;
            model->SetWID(ws->ID());
        }
        */
    }
};

} // namespace toggl

#endif SRC_USER_DATA_H_
