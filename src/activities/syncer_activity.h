#ifndef SRC_SYNCER_ACTIVITY_H_
#define SRC_SYNCER_ACTIVITY_H_

#include "activity.h"
#include "types.h"
#include "error.h"

#include <Poco/Activity.h>

#include <vector>

namespace toggl {

class Context;
class TogglClient;
class Client;
class Project;
class Workspace;

class SyncerActivity : public toggl::Activity {
public:
    void work() override;

    void syncerActivity();

    void push();
    void sync();

private:
    error pullAllUserData(TogglClient *toggl_client);
    error pushChanges(TogglClient *toggl_client, bool *had_something_to_push);
    error pushClients(std::vector<Client *> clients, std::string api_token, TogglClient toggl_client);
    error pushProjects(std::vector<Project *> projects, std::vector<Client *> clients, std::string api_token, TogglClient toggl_client);
    error pushObmAction();

    error pullWorkspaces(TogglClient* toggl_client);
    error pullWorkspacePreferences(TogglClient* toggl_client);
    error pullWorkspacePreferences(TogglClient* toggl_client, Workspace* workspace, std::string* json);
    error pullUserPreferences(TogglClient* toggl_client);

    Context *context_;

    Poco::Mutex syncer_m_;
    Poco::Activity<SyncerActivity> activity_;

    bool trigger_push_ { false };
    bool trigger_sync_ { false };
};

};

#endif // SRC_SYNCER_ACTIVITY_H_
