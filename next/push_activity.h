#ifndef SRC_PUSH_ACTIVITY_H_
#define SRC_PUSH_ACTIVITY_H_

namespace toggl {

class PushActivity {


    error pushChanges(
        TogglClient *toggl_client,
        bool *had_something_to_push) {
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
                    logger().warning("cannot push changes when logged out");
                    return noError;
                }

                api_token = user_->APIToken();
                if (api_token.empty()) {
                    return error("cannot push changes without API token");
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

    error pushClients(
        std::vector<Client *> clients,
        std::string api_token,
        TogglClient toggl_client) {
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

    error pushProjects(
        std::vector<Project *> projects,
        std::vector<Client *> clients,
        std::string api_token,
        TogglClient toggl_client) {
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


    error pushEntries(
        std::map<std::string, BaseModel *>,
        std::vector<TimeEntry *> time_entries,
        std::string api_token,
        TogglClient toggl_client) {

        std::string entry_json("");
        std::string error_message("");
        bool error_found = false;
        bool offline = false;

        for (std::vector<TimeEntry *>::const_iterator it =
            time_entries.begin();
                it != time_entries.end(); it++) {
            // Avoid trying to POST when we're offline
            if (offline) {
                // Mark the time entry as unsynced now
                (*it)->SetUnsynced();
                continue;
            }

            Json::Value entryJson = (*it)->SaveToJSON();

            Json::StyledWriter writer;
            entry_json = writer.write(entryJson);

            // std::cout << entry_json;

            HTTPSRequest req;
            req.host = urls::API();
            req.relative_url = (*it)->ModelURL();
            req.payload = entry_json;
            req.basic_auth_username = api_token;
            req.basic_auth_password = "api_token";

            HTTPSResponse resp;

            if ((*it)->NeedsDELETE()) {
                req.payload = "";
                resp = toggl_client.Delete(req);
            } else if ((*it)->ID()) {
                resp = toggl_client.Put(req);
            } else {
                resp = toggl_client.Post(req);
            }

            if (resp.err != noError) {
                // if we're able to solve the error
                if ((*it)->ResolveError(resp.body)) {
                    displayError(save(false));
                }

                // Not found on server. Probably deleted already.
                if ((*it)->isNotFound(resp.body)) {
                    (*it)->MarkAsDeletedOnServer();
                    continue;
                }
                error_found = true;
                error_message = resp.body;
                if (error_message == noError) {
                    error_message = resp.err;
                }
                // Mark the time entry as unsynced now
                (*it)->SetUnsynced();

                offline = IsNetworkingError(resp.err);

                if (offline) {
                    trigger_sync_ = false;
                }

                continue;
            }

            if ((*it)->NeedsDELETE()) {
                // Successfully deleted entry
                (*it)->MarkAsDeletedOnServer();
                continue;
            }

            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(resp.body, root)) {
                return error("error parsing time entry POST response");
            }

            (*it)->LoadFromJSON(root);
        }

        if (error_found) {
            return error_message;
        }
        return noError;
    }


    error pushObmAction() {
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

                if (user_->related.ObmActions.empty()) {
                    return noError;
                }

                req.basic_auth_username = user_->APIToken();
                if (req.basic_auth_username.empty()) {
                    return error("cannot push OBM actions without API token");
                }

                // find action that has not been uploaded yet
                for (std::vector<ObmAction *>::iterator it =
                    user_->related.ObmActions.begin();
                        it != user_->related.ObmActions.end();
                        it++) {
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



    template<typename T>
    void collectPushableModels(
        const std::vector<T *> list,
        std::vector<T *> *result,
        std::map<std::string, BaseModel *> *models) const {

        poco_check_ptr(result);

        for (typename std::vector<T *>::const_iterator it =
            list.begin();
                it != list.end();
                it++) {
            T *model = *it;
            if (!model->NeedsPush()) {
                continue;
            }
            user_->EnsureWID(model);
            model->EnsureGUID();
            result->push_back(model);
            if (models && !model->GUID().empty()) {
                (*models)[model->GUID()] = model;
            }
        }
    }

};

} // namespace toggl

#endif // SRC_PUSH_ACTIVITY_H_
