#include "synchronization.h"

#include "context.h"
#include "net/https_client.h"

#include <Poco/Thread.h>

namespace toggl {

Synchronization::Synchronization(Context *parent)
    : context_(parent)
    , syncer_(this, &Synchronization::syncerActivity)
{

}

void Synchronization::syncerActivity() {
    while (true) {
        // Sleep in increments for faster shutdown.
        for (int i = 0; i < 4; i++) {
            if (syncer_.isStopped()) {
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
                    displayError(err);
                }

                context_->SetOnlineState("Data pulled");

                err = pushChanges(&client, &trigger_sync_);
                trigger_push_ = false;
                if (err != noError) {
                    context_->User()->ConfirmLoadedMore();
                    displayError(err);
                    return;
                } else {
                    context_->SetOnlineState("Data pushed");
                }
                trigger_sync_ = false;

                // Push cached OBM action
                err = pushObmAction();
                if (err != noError) {
                    std::cout << "SYNC: sync-pushObm ERROR\n";
                    logger.error("Error pushing OBM action: ", err);
                }

                displayError(save(false));
            }

        }

        {
            Poco::Mutex::ScopedLock lock(syncer_m_);

            if (trigger_push_) {
                TogglClient client(UI());

                error err = pushChanges(&client, &trigger_sync_);
                if (err != noError) {
                    user_->ConfirmLoadedMore();
                    displayError(err);
                } else {
                    setOnline("Data pushed");
                }
                trigger_push_ = false;

                // Push cached OBM action
                err = pushObmAction();
                if (err != noError) {
                    std::cout << "SYNC: pushObm ERROR\n";
                    logger.error("Error pushing OBM action: ", err);
                }

                displayError(save(false));
            }
        }
    }
}

error Synchronization::pullAllUserData(TogglClient *toggl_client) {

    std::string api_token("");
    Poco::Int64 since(0);
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger.warning("cannot pull user data when logged out");
            return noError;
        }
        api_token = user_->APIToken();
        if (user_->HasValidSinceDate()) {
            since = user_->Since();
        }
    }

    if (api_token.empty()) {
        return error::kCannotLoadUserDataWithoutApiToken;
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
                return error::kCannotLoadUserDataWhenLoggedOut;
            }
            TimeEntry *running_entry = user_->RunningTimeEntry();

            error err = user_->LoadUserAndRelatedDataFromJSONString(user_data_json, !since);

            if (err != noError) {
                return err;
            }
            overlay_visible_ = false;
            TimeEntry *new_running_entry = user_->RunningTimeEntry();

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
        logger.debug("User with related data JSON fetched and parsed in ", stopwatch.elapsed() / 1000, " ms");
    } catch(const Poco::Exception& exc) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    } catch(const std::exception& ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    } catch(const std::string & ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    return noError;
}

error Synchronization::pushChanges(TogglClient *toggl_client, bool *had_something_to_push) {
    try {
        Poco::Stopwatch stopwatch;
        stopwatch.start();

        poco_check_ptr(had_something_to_push);

        *had_something_to_push = true;

        std::map<std::string, BaseModel *> models;

        std::vector<TimeEntry *> time_entries;
        std::vector<Project *> projects;
        std::vector<Client *> clients;

        std::string api_token("");

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger.warning("cannot push changes when logged out");
                return noError;
            }

            api_token = user_->APIToken();
            if (api_token.empty()) {
                return error::kCannotSaveUserDataWithoutApiToken;
            }

            collectPushableModels(
                        user_->related.TimeEntries,
                        &time_entries,
                        &models);
            collectPushableModels(
                        user_->related.Projects,
                        &projects,
                        &models);
            collectPushableModels(
                        user_->related.Clients,
                        &clients,
                        &models);
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
            if (err == error::kClientNameAlreadyExists) {
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
            if (err == error::kProjectNameAlreadyExists) {
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
        logger.debug(ss.str());
    } catch(const Poco::Exception& exc) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    } catch(const std::exception& ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    } catch(const std::string & ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    return noError;
}

error Synchronization::pushClients(const std::vector<Client *> &clients, const std::string &api_token, const TogglClient &toggl_client) {
    std::string client_json("");
    error err = noError;
    for (std::vector<Client *>::const_iterator it =
         clients.begin();
         it != clients.end(); ++it) {
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
            if ((*it)->ResolveError(Error::fromServerError(resp.body))) {
                displayError(save(false));
            }
            continue;
        }

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(resp.body, root)) {
            err = error::kFailedToParseData;
            continue;
        }

        (*it)->LoadFromJSON(root);
    }

    return err;
}

error Synchronization::pushProjects(const std::vector<Project *> &projects, const std::vector<Client *> &clients, const std::string &api_token, const TogglClient &toggl_client) {
    error err = noError;
    std::string project_json("");
    for (std::vector<Project *>::const_iterator it =
         projects.begin();
         it != projects.end(); ++it) {
        if (!(*it)->CID() && !(*it)->ClientGUID().empty()) {
            // Find client id
            for (std::vector<Client *>::const_iterator itc =
                 clients.begin();
                 itc != clients.end(); ++itc) {
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
            if ((*it)->ResolveError(Error::fromServerError(resp.body))) {
                displayError(save(false));
            }
            continue;
        }

        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(resp.body, root)) {
            err = error::kFailedToParseData;
            continue;
        }

        (*it)->LoadFromJSON(root);
    }

    return err;
}

error Synchronization::pushObmAction() {
    try {
        ObmAction *for_upload = nullptr;
        HTTPSRequest req;
        req.host = urls::API();
        req.basic_auth_password = "api_token";

        // Get next OBM action for upload
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger.warning("cannot push changes when logged out");
                return noError;
            }

            if (user_->related.ObmActions.empty()) {
                return noError;
            }

            req.basic_auth_username = user_->APIToken();
            if (req.basic_auth_username.empty()) {
                return error::kCannotSaveUserDataWithoutApiToken;
            }

            // find action that has not been uploaded yet
            for (std::vector<ObmAction *>::iterator it =
                 user_->related.ObmActions.begin();
                 it != user_->related.ObmActions.end();
                 ++it) {
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

        logger.debug(req.payload);

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
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    } catch(const std::exception& ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    } catch(const std::string & ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    return noError;
}

error Synchronization::pullWorkspaces(TogglClient *toggl_client) {
    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error::kCannotLoadUserDataWithoutApiToken;
    }

    std::string json("");

    try {
        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/me/workspaces";
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = toggl_client->Get(req);
        if (resp.err != noError) {
            if (resp.err == error::kForbiddenError) {
                // User has no workspaces
                return error::kMissingWS; // NOLINT
            }
            return resp.err;
        }

        json = resp.body;

        user_->LoadWorkspacesFromJSONString(json);

    }
    catch (const Poco::Exception& exc) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    catch (const std::exception& ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    catch (const std::string & ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    return noError;
}

error Synchronization::pullWorkspacePreferences(TogglClient *toggl_client) {
    std::vector<Workspace*> workspaces;
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        logger.debug("user mutex lock success - c:pullWorkspacePreferences");

        context_->user_->related.WorkspaceList(&workspaces);
    }
    for (std::vector<Workspace*>::const_iterator
         it = workspaces.begin();
         it != workspaces.end();
         ++it) {
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
            return error::kFailedToParseData;
        }

        ws->LoadSettingsFromJson(root);
    }

    return noError;
}

error Synchronization::pullWorkspacePreferences(TogglClient *toggl_client, Workspace *workspace, std::string *json) {

    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error::kCannotLoadUserDataWithoutApiToken;
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
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    catch (const std::exception& ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    catch (const std::string & ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    return noError;
}

error Synchronization::pullUserPreferences(TogglClient *toggl_client) {
    std::string api_token = user_->APIToken();

    if (api_token.empty()) {
        return error::kCannotLoadUserDataWithoutApiToken;
    }

    try {
        std::string json("");

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/me/preferences/desktop";
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
            return error::kFailedToParseData;
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
            context_->UI()->DisplayTosAccept();
        }
    }
    catch (const Poco::Exception& exc) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    catch (const std::exception& ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    catch (const std::string & ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    return noError;
}

error Synchronization::save(const bool push_changes) {
    logger.debug("save");
    try {
        std::vector<ModelChange> changes;

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            error err = db()->SaveUser(user_, true, &changes);
            if (err != noError) {
                return err;
            }
        }

        UIElements render;
        render.display_unsynced_items = true;
        render.display_timer_state = true;
        render.ApplyChanges(time_entry_editor_guid_, changes);
        context_->updateUI(render);

        if (push_changes) {
            logger.debug("onPushChanges executing");

            // Always sync asyncronously with syncerActivity
            trigger_push_ = true;
            if (!syncer_.isRunning()) {
                syncer_.start();
            }
        }
    } catch(const Poco::Exception& exc) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    } catch(const std::exception& ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    } catch(const std::string & ex) {
        return error::REMOVE_LATER_EXCEPTION_HANDLER;
    }
    return noError;
}

Logger Synchronization::logger() const {
    return Logger("Synchronization");
}

error Synchronization::displayError(const error &err)
{

}

}
