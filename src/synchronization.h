#ifndef SRC_SYNCHRONIZATION_H_
#define SRC_SYNCHRONIZATION_H_

#include "util/types.h"

#include <string>
#include <vector>

#include <Poco/Activity.h>

namespace toggl {

class Context;
class TogglClient;
class Client;
class Project;
class Workspace;

class Synchronization
{
public:
    Synchronization(Context *parent);

    void syncerActivity();

    error pullAllUserData(
        TogglClient *toggl_client);

    error pushChanges(
        TogglClient *toggl_client,
        bool *had_something_to_push);

    error pushClients(
        const std::vector<Client *> &clients,
        const std::string &api_token,
        const TogglClient &toggl_client);

    error pushProjects(
        const std::vector<Project *> &projects,
        const std::vector<Client *> &clients,
        const std::string &api_token,
        const TogglClient &toggl_client);


    error pushObmAction();

    error pullWorkspaces(TogglClient* toggl_client);

    error pullWorkspacePreferences(TogglClient* toggl_client);

    error pullWorkspacePreferences(
        TogglClient* toggl_client,
        Workspace* workspace,
        std::string* json);

    error pullUserPreferences(
        TogglClient* toggl_client);

    error save(const bool push_changes);

private:
    Logger logger { "Synchronization" };
    error displayError(const error &err);

    Context *context_;

    Poco::Mutex syncer_m_;
    Poco::Activity<Synchronization> syncer_;

    bool trigger_sync_;
    bool trigger_push_;
};

} // namespace toggl

#endif // SRC_SYNCHRONIZATION_H_
