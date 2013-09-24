// Copyright 2013 Tanel Lebedev

#include "./main.h"

#include <sstream>

#include "./kopsik_api.h"

#include "Poco/Message.h"
#include "Poco/Logger.h"
#include "Poco/Util/Application.h"

#define ERRLEN 1024

namespace command_line_client {

    void Main::usage() {
        Poco::Logger &logger = Poco::Logger::get("");
        logger.information("Recognized commands are: "
            "sync, start, stop, status, dirty");
    }

    int Main::main(const std::vector<std::string>& args) {
        Poco::Logger &logger = Poco::Logger::get("");
        logger.setLevel(Poco::Message::PRIO_DEBUG);

        if (args.empty()) {
            usage();
            return Poco::Util::Application::EXIT_USAGE;
        }

        char* apiToken = getenv("TOGGL_API_TOKEN");
        if (!apiToken) {
            logger.error("Please set TOGGL_API_TOKEN in environment");
            return Poco::Util::Application::EXIT_USAGE;
        }

        char err[ERRLEN];
        if (KOPSIK_API_FAILURE == kopsik_set_api_token(err, ERRLEN, apiToken)) {
            logger.error(err);
            return Poco::Util::Application::EXIT_SOFTWARE;
        }

        if ("sync" == args[0]) {
            if (KOPSIK_API_FAILURE == kopsik_sync(err, ERRLEN)) {
                logger.error(err);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            return Poco::Util::Application::EXIT_OK;
        }

        if ("status" == args[0]) {
            TogglTimeEntry *te = kopsik_time_entry_new();
            int is_tracking(0);
            if (KOPSIK_API_FAILURE == kopsik_running_time_entry(err, ERRLEN,
                    te, &is_tracking)) {
                logger.error(err);
                kopsik_time_entry_delete(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            if (is_tracking) {
                logger.information("Tracking: " + std::string(te->Description));
            } else {
                logger.information("Not tracking.");
            }
            kopsik_time_entry_delete(te);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("dirty" == args[0]) {
            TogglDirtyModels dm;
            if (KOPSIK_API_FAILURE == kopsik_dirty_models(err, ERRLEN, &dm)) {
                logger.error(err);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            std::stringstream ss;
            ss << dm.TimeEntries << " dirty time entries.";
            logger.information(ss.str());
            return Poco::Util::Application::EXIT_OK;
        }

        if ("start" == args[0]) {
            TogglTimeEntry *te = kopsik_time_entry_new();
            if (KOPSIK_API_FAILURE == kopsik_start(err, ERRLEN, te)) {
                logger.error(err);
                kopsik_time_entry_delete(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            if (KOPSIK_API_FAILURE == kopsik_sync(err, ERRLEN)) {
                logger.error(err);
            }
            if (te->Description) {
                logger.information("Started: " + std::string(te->Description));
            } else {
                logger.information("Started.");
            }
            kopsik_time_entry_delete(te);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("stop" == args[0]) {
            TogglTimeEntry *te = kopsik_time_entry_new();
            if (KOPSIK_API_FAILURE == kopsik_stop(err, ERRLEN, te)) {
                logger.error(err);
                kopsik_time_entry_delete(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            if (KOPSIK_API_FAILURE == kopsik_sync(err, ERRLEN)) {
                logger.error(err);
            }
            if (te->Description) {
                logger.information("Stopped: " + std::string(te->Description));
            } else {
                logger.information("Stopped.");
            }
            kopsik_time_entry_delete(te);
            return Poco::Util::Application::EXIT_OK;
        }

        usage();
        return Poco::Util::Application::EXIT_USAGE;
    }
}  // namespace command_line_client
