#ifndef SRC_PUSH_ACTIVITY_H_
#define SRC_PUSH_ACTIVITY_H_

#include "https_client.h"
#include "activity.h"
#include "types.h"
#include "error.h"

#include <Poco/Activity.h>

#include <vector>
#include <map>
#include <set>

namespace toggl {

class Context;
class TogglClient;
class Client;
class Project;
class Workspace;
class BaseModel;
class TimeEntry;

class PushActivity : public toggl::Activity {
public:
    PushActivity(ActivityManager *parent);

    void work() override;

    void syncerActivity();

    void push();
    void sync();

private:
    template<typename T>
    void collectPushableModels(const std::set<T *> &list, std::vector<T *> *result, std::map<std::string, BaseModel *> *models = nullptr);

    error pullAllUserData(TogglClient *toggl_client);
    error pushChanges(TogglClient *toggl_client, bool *had_something_to_push);

    error pushEntries(std::map<std::string, BaseModel *> models, std::vector<TimeEntry *> time_entries, std::string api_token, TogglClient toggl_client);
    error pushClients(std::vector<Client *> clients, std::string api_token, TogglClient toggl_client);
    error pushProjects(std::vector<Project *> projects, std::vector<Client *> clients, std::string api_token, TogglClient toggl_client);
    error pushObmAction();

    error pullWorkspaces(TogglClient* toggl_client);
    error pullWorkspacePreferences(TogglClient* toggl_client);
    error pullWorkspacePreferences(TogglClient* toggl_client, Workspace* workspace, std::string* json);
    error pullUserPreferences(TogglClient* toggl_client);

    Poco::Mutex syncer_m_;

    bool trigger_push_ { false };
    bool trigger_sync_ { false };
};

};

#endif // SRC_SYNCER_ACTIVITY_H_
