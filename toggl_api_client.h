// Copyright 2013 Tanel Lebedev

#ifndef TOGGL_API_CLIENT_H_
#define TOGGL_API_CLIENT_H_

#include <string>
#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>

#include "libjson.h" // NOLINT

#include "Poco/Types.h"

#include "./types.h"

namespace kopsik {

    class Workspace {
    public:
        Workspace() : LocalID(0), ID(0), Name(""), UID(0) {}

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        std::string Name;
        Poco::UInt64 UID;

        error Load(JSONNODE *node);
        std::string String();
    };

    class Client {
    public:
        Client() : LocalID(0), ID(0), GUID(""), WID(0), Name(""), UID(0) {}

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        guid GUID;
        Poco::UInt64 WID;
        std::string Name;
        Poco::UInt64 UID;

        error Load(JSONNODE *node);
        std::string String();
    };

    class Project {
    public:
        Project() : LocalID(0), ID(0), GUID(""), WID(0), CID(0), Name(""),
            UID(0) {}

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        guid GUID;
        Poco::UInt64 WID;
        Poco::UInt64 CID;
        std::string Name;
        Poco::UInt64 UID;

        error Load(JSONNODE *node);
        std::string String();
    };

    class Task {
    public:
        Task() : LocalID(0), ID(0), Name(""), WID(0), PID(0), UID(0) {}

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        std::string Name;
        Poco::UInt64 WID;
        Poco::UInt64 PID;
        Poco::UInt64 UID;

        error Load(JSONNODE *node);
        std::string String();
    };

    class Tag {
    public:
        Tag() : LocalID(0), ID(0), WID(0), Name(""), GUID(""), UID(0) {}

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        Poco::UInt64 WID;
        std::string Name;
        guid GUID;
        Poco::UInt64 UID;

        error Load(JSONNODE *node);
        std::string String();
    };

    class TimeEntry {
    public:
        TimeEntry() : LocalID(0),
            ID(0), GUID(""), WID(0), PID(0), TID(0), Billable(false),
            Start(""), Stop(""), DurationInSeconds(0), Description(""),
            DurOnly(false), UIModifiedAt(0), UID(0) {}

        Poco::Int64 LocalID;
        Poco::UInt64 ID;
        guid GUID;
        Poco::UInt64 WID;
        Poco::UInt64 PID;
        Poco::UInt64 TID;
        bool Billable;
        // FIXME: should be Uint64 maybe
        std::string Start;
        std::string Stop;
        Poco::Int64 DurationInSeconds;
        std::string Description;
        bool DurOnly;
        // TE is the only model that can actually
        // be updated by user.
        Poco::UInt64 UIModifiedAt;
        Poco::UInt64 UID;

        std::vector<std::string> TagNames;
        std::string Tags() {
            std::stringstream ss;
            std::copy(TagNames.begin(), TagNames.end(),
                std::ostream_iterator<std::string>(ss, "|"));
            return ss.str();
        }
        void SetTags(std::string tags) {
            TagNames.clear();
            std::stringstream ss(tags);
            while (ss.good()) {
                std::string tag;
                getline(ss, tag, '|');
                TagNames.push_back(tag);
            }
        }

        error Load(JSONNODE *node);
        std::string String();

    private:
        error loadTags(JSONNODE *list);
    };

    class User {
    public:
        User() : LocalID(0), ID(0), APIToken(""), DefaultWID(0), Since(0) {}
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

        std::vector<Workspace *> Workspaces;
        std::vector<Client *> Clients;
        std::vector<Project *> Projects;
        std::vector<Task *> Tasks;
        std::vector<Tag *> Tags;
        std::vector<TimeEntry *> TimeEntries;

        error Fetch();
        error Load(const std::string &json);
        error Load(JSONNODE *node);
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

    private:
        error loadProjects(JSONNODE *list);
        error loadTags(JSONNODE *list);
        error loadClients(JSONNODE *list);
        error loadTasks(JSONNODE *list);
        error loadTimeEntries(JSONNODE *list);
        error loadWorkspaces(JSONNODE *list);
    };
}  // namespace kopsik

#endif  // TOGGL_API_CLIENT_H_
