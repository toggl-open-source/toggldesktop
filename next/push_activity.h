#ifndef SRC_PUSH_ACTIVITY_H_
#define SRC_PUSH_ACTIVITY_H_

#include "error.h"
#include "https_client.h"
#include "model/base_model.h"

#include <vector>
#include <Poco/Logger.h>

namespace toggl {

class BaseModel;
class Client;
class Project;
class TimeEntry;
class UserData;
class Database;

class PushActivity {


    error save(bool push_changes);


    error pushChanges(
        TogglClient *toggl_client,
        bool *had_something_to_push);

    error pushClients(
        std::vector<Client *> clients,
        std::string api_token,
        TogglClient toggl_client);

    error pushProjects(
        std::vector<Project *> projects,
        std::vector<Client *> clients,
        std::string api_token,
        TogglClient toggl_client);


    error pushEntries(
        std::map<std::string, BaseModel *>,
        std::vector<TimeEntry *> time_entries,
        std::string api_token,
        TogglClient toggl_client);


    error pushObmAction();



    error updateEntryProjects(
        std::vector<Project *> projects,
        std::vector<TimeEntry *> time_entries);



    template<typename T>
    void collectPushableModels(
        ProtectedModel<T> &model,
        std::vector<T *> *result,
        std::map<std::string, BaseModel *> *models);

private:
    UserData *user();
    Database *DB();

    Poco::Logger &logger() const;

    Context *context_;


};

} // namespace toggl

#endif // SRC_PUSH_ACTIVITY_H_
