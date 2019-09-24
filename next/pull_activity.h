#ifndef SRC_PULL_ACTIVITY_H_
#define SRC_PULL_ACTIVITY_H_

#include "error.h"
#include "https_client.h"

#include <Poco/Logger.h>

namespace toggl {

class Workspace;
class UserData;
class GUI;

class PullActivity {
public:
    PullActivity(Context *context);

    error login(const std::string &email, const std::string &password);

    error me(TogglClient *toggl_client, const std::string email, const std::string password, std::string *user_data_json, const Poco::Int64 since);

    error pullWorkspacePreferences(TogglClient* toggl_client);
    error pullWorkspacePreferences(TogglClient* toggl_client, Workspace* workspace, std::string* json);
    error pullUserPreferences(TogglClient* toggl_client);
    error pullWorkspaces(TogglClient* toggl_client);
    error pullObmExperiments();
    error pullAllUserData(TogglClient *toggl_client);

private:
    Poco::Logger &logger() const;
    UserData *user();
    GUI *UI();
    TogglClient *httpsClient();

    Context *context_;

};

} // namespace toggl

#endif // SRC_PULL_ACTIVITY_H_
