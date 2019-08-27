#ifndef SRC_PULL_ACTIVITY_H_
#define SRC_PULL_ACTIVITY_H_

namespace toggl {

class PullActivity {

    error pullWorkspacePreferences(TogglClient* toggl_client) {
        std::vector<Workspace*> workspaces;
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            logger().debug("user mutex lock success - c:pullWorkspacePreferences");

            user_->related.WorkspaceList(&workspaces);
        }
        for (std::vector<Workspace*>::const_iterator
                it = workspaces.begin();
                it != workspaces.end();
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

    error pullWorkspacePreferences(
        TogglClient* toggl_client,
        Workspace* workspace,
        std::string* json) {

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

    error pullUserPreferences(
        TogglClient* toggl_client) {
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



    error pullWorkspaces(TogglClient* toggl_client) {
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

    error pullObmExperiments() {
        try {
            if (HTTPSClient::Config.OBMExperimentNrs.empty()) {
                logger().debug("No OBM experiment enabled by UI");
                return noError;
            }

            logger().trace("Fetching OBM experiments from backend");

            std::string apitoken("");
            {
                Poco::Mutex::ScopedLock lock(user_m_);
                if (!user_) {
                    logger().warning("Cannot fetch OBM experiments without user");
                    return noError;
                }
                apitoken = user_->APIToken();
            }

            HTTPSRequest req;
            req.host = urls::API();
            req.relative_url = "/api/v9/me/experiments";
            req.basic_auth_username = apitoken;
            req.basic_auth_password = "api_token";

            TogglClient client(UI());
            HTTPSResponse resp = client.Get(req);
            if (resp.err != noError) {
                return resp.err;
            }

            Json::Value json;
            Json::Reader reader;
            if (!reader.parse(resp.body, json)) {
                return error("Error in OBM experiments response body");
            }

            {
                Poco::Mutex::ScopedLock lock(user_m_);
                if (!user_) {
                    logger().warning("Cannot apply OBM experiments without user");
                    return noError;
                }
                user_->LoadObmExperiments(json);
            }

            return noError;
        } catch(const Poco::Exception& exc) {
            return exc.displayText();
        } catch(const std::exception& ex) {
            return ex.what();
        } catch(const std::string& ex) {
            return ex;
        }
    }


    error pullAllUserData(
        TogglClient *toggl_client) {

        std::string api_token("");
        Poco::Int64 since(0);
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

};

} // namespace toggl

#endif // SRC_PULL_ACTIVITY_H_
