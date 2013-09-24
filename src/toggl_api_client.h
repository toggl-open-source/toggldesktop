// Copyright 2013 Tanel Lebedev

#ifndef SRC_TOGGL_API_CLIENT_H_
#define SRC_TOGGL_API_CLIENT_H_

#include <string>
#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <ctime>

#include "libjson.h" // NOLINT

#include "Poco/Types.h"

#include "./types.h"

namespace kopsik {

    class BatchUpdate {
    public:
        BatchUpdate() : Method(""), RelativeUrl(""), Body("") {
        }
        std::string Method;
        std::string RelativeUrl;
        std::string Body;
    };

    class BatchUpdateResult {
    public:
        BatchUpdateResult() : StatusCode(0), Body("") {
        }
        Poco::Int64 StatusCode;
        std::string Body;

        void parseResponseJSON(JSONNODE *n);
        void parseResponseJSONBody(std::string body);
    };

    class Workspace {
    public:
        Workspace() : LocalID(0), ID(0), Name(""), UID(0), Dirty(false) {}

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        std::string Name;
        Poco::UInt64 UID;
        bool Dirty;

        error LoadFromJSONNode(JSONNODE *node);
        std::string String();
    };

    class Client {
    public:
        Client() : LocalID(0), ID(0), GUID(""), WID(0), Name(""), UID(0),
            Dirty(false) {}

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        guid GUID;
        Poco::UInt64 WID;
        std::string Name;
        Poco::UInt64 UID;
        bool Dirty;

        error LoadFromJSONNode(JSONNODE *node);
        std::string String();
    };

    class Project {
    public:
        Project() : LocalID(0), ID(0), GUID(""), WID(0), CID(0), Name(""),
            UID(0), Dirty(false) {}

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        guid GUID;
        Poco::UInt64 WID;
        Poco::UInt64 CID;
        std::string Name;
        Poco::UInt64 UID;
        bool Dirty;

        error LoadFromJSONNode(JSONNODE *node);
        std::string String();
    };

    class Task {
    public:
        Task() : LocalID(0), ID(0), Name(""), WID(0), PID(0), UID(0),
            Dirty(false) {}

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        std::string Name;
        Poco::UInt64 WID;
        Poco::UInt64 PID;
        Poco::UInt64 UID;
        bool Dirty;

        error LoadFromJSONNode(JSONNODE *node);
        std::string String();
    };

    class Tag {
    public:
        Tag() : LocalID(0), ID(0), WID(0), Name(""), GUID(""), UID(0),
            Dirty(false) {}

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        Poco::UInt64 WID;
        std::string Name;
        guid GUID;
        Poco::UInt64 UID;
        bool Dirty;

        error LoadFromJSONNode(JSONNODE *node);
        std::string String();
    };

    class TimeEntry {
    public:
        TimeEntry() : LocalID(0),
            ID(0), GUID(""), WID(0), PID(0), TID(0), Billable(false),
            Start(0), Stop(0), DurationInSeconds(0), Description(""),
            DurOnly(false), UIModifiedAt(0), UID(0), Dirty(false) {}

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        guid GUID;
        Poco::UInt64 WID;
        Poco::UInt64 PID;
        Poco::UInt64 TID;
        bool Billable;
        Poco::UInt64 Start;
        Poco::UInt64 Stop;
        Poco::Int64 DurationInSeconds;
        std::string Description;
        bool DurOnly;
        // TE is the only model that can actually
        // be updated by user.
        Poco::UInt64 UIModifiedAt;
        Poco::UInt64 UID;
        bool Dirty;

        std::vector<std::string> TagNames;
        std::string Tags();
        void SetTags(std::string tags);

        std::string StartString();
        void SetStartString(std::string value);
        std::string StopString();
        void SetStopString(std::string value);

        error LoadFromJSONNode(JSONNODE *node);
        error LoadFromJSONString(std::string json);

        std::string String();
        JSONNODE *JSON();

        bool NeedsPush();

    private:
        error loadTagsFromJSONNode(JSONNODE *list);

        std::time_t Parse8601(std::string iso_8601_formatted_date);
        std::string Format8601(std::time_t date);
    };

    class User {
    public:
        User() : LocalID(0), ID(0), APIToken(""), DefaultWID(0), Since(0),
            Dirty(false), Fullname(""), LoginEmail(""), LoginPassword("") {}
        ~User() {
            ClearWorkspaces();
            ClearClients();
            ClearProjects();
            ClearTasks();
            ClearTags();
            ClearTimeEntries();
        }

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        std::string APIToken;
        Poco::UInt64 DefaultWID;
        // Unix timestamp of the user data; returned from API
        Poco::UInt64 Since;
        bool Dirty;
        std::string Fullname;

        std::vector<Workspace *> Workspaces;
        std::vector<Client *> Clients;
        std::vector<Project *> Projects;
        std::vector<Task *> Tasks;
        std::vector<Tag *> Tags;
        std::vector<TimeEntry *> TimeEntries;

        error Sync();
        error Login(const std::string &email, const std::string &password);
        error LoadFromJSONString(const std::string &json,
            bool with_related_data);
        error LoadFromJSONNode(JSONNODE *node, bool with_related_data);
        std::string String();

        void ClearWorkspaces();
        void ClearClients();
        void ClearProjects();
        void ClearTasks();
        void ClearTags();
        void ClearTimeEntries();

        Workspace *GetWorkspaceByID(const Poco::UInt64 id);
        Client *GetClientByID(const Poco::UInt64 id);
        Project *GetProjectByID(const Poco::UInt64 id);
        Task *GetTaskByID(const Poco::UInt64 id);
        Tag *GetTagByID(const Poco::UInt64 id);
        TimeEntry *GetTimeEntryByID(const Poco::UInt64 id);

        void CollectDirtyObjects(std::vector<TimeEntry *> *result);

        TimeEntry *RunningTimeEntry();
        TimeEntry *Start();
        std::vector<TimeEntry *> Stop();

        std::string LoginEmail;
        std::string LoginPassword;

    private:
        error pull(bool authenticate_with_api_token);
        error push();

        error loadProjectsFromJSONNode(JSONNODE *list);
        error loadTagsFromJSONNode(JSONNODE *list);
        error loadClientsFromJSONNode(JSONNODE *list);
        error loadTasksFromJSONNode(JSONNODE *list);
        error loadTimeEntriesFromJSONNode(JSONNODE *list);
        error loadWorkspacesFromJSONNode(JSONNODE *list);

        error requestJSON(std::string method, std::string relative_url,
                std::string json,
                bool authenticate_with_api_token,
                std::string *response_body);
        bool isStatusOK(int status);
        void parseResponseArray(std::string response_body,
            std::vector<BatchUpdateResult> *responses);
    };
}  // namespace kopsik

#endif  // SRC_TOGGL_API_CLIENT_H_
