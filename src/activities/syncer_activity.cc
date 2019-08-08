
#include "syncer_activity.h"

#include "../context.h"
#include "../https_client.h"
#include "../obm_action.h"
#include "../client.h"
#include "../project.h"

#include <Poco/Logger.h>
#include <Poco/Stopwatch.h>

namespace toggl {

void SyncerActivity::syncerActivity() {
    while (true) {
        // Sleep in increments for faster shutdown.
        for (int i = 0; i < 4; i++) {
            if (activity_.isStopped()) {
                return;
            }
            Poco::Thread::sleep(250);
        }

        {
            Poco::Mutex::ScopedLock lock(syncer_m_);

            if (trigger_sync_) {
                TogglClient client(context_->UI());

                error err = pullAllUserData(&client);
                if (err != noError) {
                    context_->displayError(err);
                }

                context_->setOnline("Data pulled");

                err = pushChanges(&client, &trigger_sync_);
                trigger_push_ = false;
                if (err != noError) {
                    user_->ConfirmLoadedMore();
                    context_->displayError(err);
                    return;
                } else {
                    context_->setOnline("Data pushed");
                }
                trigger_sync_ = false;

                // Push cached OBM action
                err = pushObmAction();
                if (err != noError) {
                    std::cout << "SYNC: sync-pushObm ERROR\n";
                    logger().error("Error pushing OBM action: " + err);
                }

                context_->displayError(save(false));
            }

        }

        {
            Poco::Mutex::ScopedLock lock(syncer_m_);

            if (trigger_push_) {
                TogglClient client(UI());

                error err = pushChanges(&client, &trigger_sync_);
                if (err != noError) {
                    user_->ConfirmLoadedMore();
                    context_->displayError(err);
                } else {
                    context_->setOnline("Data pushed");
                }
                trigger_push_ = false;

                // Push cached OBM action
                err = pushObmAction();
                if (err != noError) {
                    std::cout << "SYNC: pushObm ERROR\n";
                    logger().error("Error pushing OBM action: " + err);
                }

                context_->displayError(save(false));
            }
        }
    }
}

void SyncerActivity::push() {

    trigger_push_ = true;
    activity_.start();
}

void SyncerActivity::sync() {

    trigger_sync_ = true;
    activity_.start();
}

error SyncerActivity::pullAllUserData(TogglClient *toggl_client) {

    std::string api_token("");
    Poco::UInt64 since(0);
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("cannot pull user data when logged out");
            return noError;
        }
        api_token = user_->APIToken();
        if (user_->HasValidSinceDate()) {
            since = user_->Since();
        }
    }

    if (api_token.empty()) {
        return error("cannot pull user data without API token");
    }

    try {
        Poco::Stopwatch stopwatch;
        stopwatch.start();

        std::string user_data_json("");
        error err = me(
                    toggl_client,
                    api_token,
                    "api_token",
                    &user_data_json,
                    since);
        if (err != noError) {
            return err;
        }

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                return error("cannot load user data when logged out");
            }
            auto running_entry = user_->RunningTimeEntry();

            error err = user_->LoadUserAndRelatedDataFromJSONString(user_data_json, !since);

            if (err != noError) {
                return err;
            }
            overlay_visible_ = false;
            auto new_running_entry = user_->RunningTimeEntry();

            // Reset reminder time when entry stopped by sync
            if (running_entry && !new_running_entry) {
                resetLastTrackingReminderTime();
            }
        }

        err = pullWorkspaces(toggl_client);
        if (err != noError) {
            return err;
        }

        pullWorkspacePreferences(toggl_client);

        pullUserPreferences(toggl_client);

        stopwatch.stop();
        std::stringstream ss;
        ss << "User with related data JSON fetched and parsed in "
           << stopwatch.elapsed() / 1000 << " ms";
        logger().debug(ss.str());
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error SyncerActivity::pushChanges(TogglClient *toggl_client, bool *had_something_to_push) {
    try {
        Poco::Stopwatch stopwatch;
        stopwatch.start();

        poco_check_ptr(had_something_to_push);

        *had_something_to_push = true;

        std::map<std::string, BaseModel *> models;

        std::vector<TimeEntry *> time_entries;
        locked<std::vector<TimeEntry *>> time_entries_guard;
        std::vector<Project *> projects;
        locked<std::vector<Project *>> projects_guard;
        std::vector<Client *> clients;
        locked<std::vector<Client *>> clients_guard;

        std::string api_token("");

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("cannot push changes when logged out");
                return noError;
            }

            api_token = user_->APIToken();
            if (api_token.empty()) {
                return error("cannot push changes without API token");
            }

            collectPushableModels(
                        *user_->related.TimeEntries(),
                        &time_entries,
                        &models);
            collectPushableModels(
                        *user_->related.Projects(),
                        &projects,
                        &models);
            collectPushableModels(
                        *user_->related.Clients(),
                        &clients,
                        &models);

            if (!time_entries.empty())
                time_entries_guard = user_->related.TimeEntries.make_locked(&time_entries);
            if (!projects.empty())
                projects_guard = user_->related.Projects.make_locked(&projects);
            if (!clients.empty())
                clients_guard = user_->related.Clients.make_locked(&clients);

            if (time_entries.empty()
                    && projects.empty()
                    && clients.empty()) {
                *had_something_to_push = false;
                return noError;
            }
        }

        std::stringstream ss;
        ss << "Sync success (";

        // Clients first as projects may depend on clients
        if (clients.size() > 0) {
            Poco::Stopwatch client_stopwatch;
            client_stopwatch.start();
            error err = pushClients(
                        clients,
                        api_token,
                        *toggl_client);
            if (err != noError &&
                    err.find(kClientNameAlreadyExists) == std::string::npos) {
                return err;
            }
            client_stopwatch.stop();
            ss << clients.size() << " clients in "
               << client_stopwatch.elapsed() / 1000 << " ms";
        }

        // Projects second as time entries may depend on projects
        if (projects.size() > 0) {
            Poco::Stopwatch project_stopwatch;
            project_stopwatch.start();
            error err = pushProjects(
                        projects,
                        clients,
                        api_token,
                        *toggl_client);
            if (err != noError &&
                    err.find(kProjectNameAlready) == std::string::npos) {
                return err;
            }

            // Update project id on time entries if needed
            err = updateEntryProjects(
                        projects,
                        time_entries);
            if (err != noError) {
                return err;
            }
            project_stopwatch.stop();
            ss << " | " << projects.size() << " projects in "
               << project_stopwatch.elapsed() / 1000 << " ms";
        }

        // Time entries last to be sure clients and projects are synced
        if (time_entries.size() > 0) {
            Poco::Stopwatch entry_stopwatch;
            entry_stopwatch.start();
            error err = pushEntries(
                        models,
                        time_entries,
                        api_token,
                        *toggl_client);
            if (err != noError) {
                // Hide load more button when offline
                user_->ConfirmLoadedMore();
                // Reload list to show unsynced icons in items
                UIElements render;
                render.display_time_entries = true;
                updateUI(render);
                return err;
            }

            entry_stopwatch.stop();
            ss << " | " << time_entries.size() << " time entries in "
               << entry_stopwatch.elapsed() / 1000 << " ms";
        }

        stopwatch.stop();
        ss << ") Total = " << stopwatch.elapsed() / 1000 << " ms";
        logger().debug(ss.str());
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error SyncerActivity::pushClients(std::vector<Client *> clients, std::string api_token, TogglClient toggl_client) {
    std::string client_json("");
    error err = noError;
    for (std::vector<Client *>::const_iterator it =
         clients.begin();
         it != clients.end(); it++) {
        Json::Value clientJson = (*it)->SaveToJSON();

        Json::StyledWriter writer;
        client_json = writer.write(clientJson);

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = (*it)->ModelURL();
        req.payload = client_json;
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = toggl_client.Post(req);

        if (resp.err != noError) {
            // if we're able to solve the error
            if ((*it)->ResolveError(resp.body)) {
                displayError(save(false));
            }
            continue;
        }

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(resp.body, root)) {
            err = error("error parsing client POST response");
            continue;
        }

        (*it)->LoadFromJSON(root);
    }

    return err;
}

error SyncerActivity::pushProjects(std::vector<Project *> projects, std::vector<Client *> clients, std::string api_token, TogglClient toggl_client) {
    error err = noError;
    std::string project_json("");
    for (std::vector<Project *>::const_iterator it =
         projects.begin();
         it != projects.end(); it++) {
        if (!(*it)->CID() && !(*it)->ClientGUID().empty()) {
            // Find client id
            for (std::vector<Client *>::const_iterator itc =
                 clients.begin();
                 itc != clients.end(); itc++) {
                if ((*itc)->GUID().compare((*it)->ClientGUID()) == 0) {
                    (*it)->SetCID((*itc)->ID());
                    break;
                }
            }
        }

        Json::Value projectJson = (*it)->SaveToJSON();

        Json::StyledWriter writer;
        project_json = writer.write(projectJson);

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = (*it)->ModelURL();
        req.payload = project_json;
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = toggl_client.Post(req);

        if (resp.err != noError) {
            // if we're able to solve the error
            if ((*it)->ResolveError(resp.body)) {
                displayError(save(false));
            }
            continue;
        }

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(resp.body, root)) {
            err = error("error parsing project POST response");
            continue;
        }

        (*it)->LoadFromJSON(root);
    }

    return err;
}

error SyncerActivity::pushObmAction() {
    try {
        ObmAction *for_upload = nullptr;
        HTTPSRequest req;
        req.host = urls::API();
        req.basic_auth_password = "api_token";

        // Get next OBM action for upload
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("cannot push changes when logged out");
                return noError;
            }

            auto obmActions = user_->related.ObmActions();
            if (obmActions->empty()) {
                return noError;
            }

            req.basic_auth_username = user_->APIToken();
            if (req.basic_auth_username.empty()) {
                return error("cannot push OBM actions without API token");
            }

            // find action that has not been uploaded yet
            for (auto it = obmActions->begin(); it != obmActions->end(); it++) {
                ObmAction *model = *it;
                if (!model->IsMarkedAsDeletedOnServer()) {
                    for_upload = model;
                    break;
                }
            }

            if (!for_upload) {
                return noError;
            }

            Json::Value root = for_upload->SaveToJSON();
            req.relative_url = for_upload->ModelURL();
            req.payload = Json::StyledWriter().write(root);
        }

        logger().debug(req.payload);

        TogglClient toggl_client;
        HTTPSResponse resp = toggl_client.Post(req);
        if (resp.err != noError) {
            // backend responds 204 on success
            if (resp.status_code != 204) {
                return resp.err;
            }
        }

        // mark as deleted to prevent duplicate uploading
        // (and make sure all other actions are uploaded)
        for_upload->MarkAsDeletedOnServer();
        for_upload->Delete();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error SyncerActivity::pullWorkspaces(TogglClient *toggl_client) {
    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error("cannot pull user data without API token");
    }

    std::string json("");

    try {
        std::stringstream ss;
        ss << "/api/v9/me/workspaces";

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = toggl_client->Get(req);
        if (resp.err != noError) {
            if (resp.err.find(kForbiddenError) != std::string::npos) {
                // User has no workspaces
                return error(kMissingWS); // NOLINT
            }
            return resp.err;
        }

        json = resp.body;

        user_->LoadWorkspacesFromJSONString(json);

    }
    catch (const Poco::Exception& exc) {
        return exc.displayText();
    }
    catch (const std::exception& ex) {
        return ex.what();
    }
    catch (const std::string& ex) {
        return ex;
    }
    return noError;
}

error SyncerActivity::pullWorkspacePreferences(TogglClient *toggl_client) {
    locked<std::vector<Workspace*>> workspaces;
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        logger().debug("user mutex lock success - c:pullWorkspacePreferences");

        workspaces = user_->related.WorkspaceList();
    }
    for (std::vector<Workspace*>::const_iterator
         it = workspaces->begin();
         it != workspaces->end();
         it++) {
        Workspace* ws = *it;

        if (!ws->Business())
            continue;

        std::string json("");

        error err = pullWorkspacePreferences(toggl_client, ws, &json);
        if (err != noError) {
            return err;
        }

        if (json.empty())
            continue;

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) {
            return error("Failed to load workspace preferences");
        }

        ws->LoadSettingsFromJson(root);
    }

    return noError;
}

error SyncerActivity::pullWorkspacePreferences(TogglClient *toggl_client, Workspace *workspace, std::string *json) {

    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error("cannot pull user data without API token");
    }

    try {
        std::stringstream ss;
        ss << "/api/v9/workspaces/"
           << workspace->ID()
           << "/preferences";

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = toggl_client->Get(req);
        if (resp.err != noError) {
            return resp.err;
        }

        *json = resp.body;
    }
    catch (const Poco::Exception& exc) {
        return exc.displayText();
    }
    catch (const std::exception& ex) {
        return ex.what();
    }
    catch (const std::string& ex) {
        return ex;
    }
    return noError;
}

error SyncerActivity::pullUserPreferences(TogglClient *toggl_client) {
    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error("cannot pull user data without API token");
    }

    try {
        std::string json("");
        std::stringstream ss;
        ss << "/api/v9/me/preferences/desktop";

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = toggl_client->Get(req);
        if (resp.err != noError) {
            return resp.err;
        }

        json = resp.body;

        if (json.empty())
            return noError;

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json, root)) {
            return error("Failed to load user preferences");
        }

        if (user_->LoadUserPreferencesFromJSON(root)) {
            // Reload list if user preferences
            // have changed (collapse time entries)
            UIElements render;
            render.display_time_entries = true;
            updateUI(render);
        }

        // Show tos accept overlay
        if (root.isMember("ToSAcceptNeeded") && root["ToSAcceptNeeded"].asBool()) {
            overlay_visible_ = true;
            UI()->DisplayTosAccept();
        }
    }
    catch (const Poco::Exception& exc) {
        return exc.displayText();
    }
    catch (const std::exception& ex) {
        return ex.what();
    }
    catch (const std::string& ex) {
        return ex;
    }
    return noError;
}

};
