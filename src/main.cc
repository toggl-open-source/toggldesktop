// Copyright 2013 Tanel Lebedev

#include "./main.h"

#include <sstream>
#include <iostream> // NOLINT

#include "Poco/Message.h"
#include "Poco/Util/Application.h"

#define ERRLEN 1024

namespace command_line_client {

    void Main::usage() {
        std::cout << "Recognized commands are: "
            "sync, start, stop, status, dirty, list" << std::endl;
    }

    int Main::main(const std::vector<std::string>& args) {
        if (args.empty()) {
            usage();
            return Poco::Util::Application::EXIT_USAGE;
        }

        char* apiToken = getenv("TOGGL_API_TOKEN");
        if (!apiToken) {
            std::cerr << "Please set TOGGL_API_TOKEN in environment" <<
                std::endl;
            return Poco::Util::Application::EXIT_USAGE;
        }

        kopsik_set_db_path(ctx_, "kopsik.db");
        kopsik_set_log_path(ctx_, "kopsik.log");

        char err[ERRLEN];
        if (KOPSIK_API_FAILURE == kopsik_set_api_token(
                ctx_, err, ERRLEN, apiToken)) {
            std::cerr << err << std::endl;
            return Poco::Util::Application::EXIT_SOFTWARE;
        }

        if ("sync" == args[0]) {
            if (KOPSIK_API_FAILURE == kopsik_sync(ctx_, err, ERRLEN)) {
                std::cerr << err << std::endl;
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            std::cout << "Synced." << std::endl;
            return Poco::Util::Application::EXIT_OK;
        }

        if ("status" == args[0]) {
            TogglTimeEntry *te = kopsik_time_entry_init();
            int is_tracking(0);
            if (KOPSIK_API_FAILURE == kopsik_running_time_entry(
                    ctx_, err, ERRLEN, te, &is_tracking)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_clear(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            if (is_tracking) {
                std::cout << "Tracking: " << te->Description << std::endl;
            } else {
                std::cout << "Not tracking." << std::endl;
            }
            kopsik_time_entry_clear(te);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("dirty" == args[0]) {
            TogglDirtyModels dm;
            if (KOPSIK_API_FAILURE == kopsik_dirty_models(
                    ctx_, err, ERRLEN, &dm)) {
                std::cerr << err << std::endl;
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            std::cout << dm.TimeEntries << " dirty time entries." << std::endl;
            return Poco::Util::Application::EXIT_OK;
        }

        if ("start" == args[0]) {
            TogglTimeEntry *te = kopsik_time_entry_init();
            if (KOPSIK_API_FAILURE == kopsik_start(ctx_, err, ERRLEN, te)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_clear(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            if (KOPSIK_API_FAILURE == kopsik_sync(ctx_, err, ERRLEN)) {
                std::cerr << err << std::endl;
            }
            if (te->Description) {
                std::cout << "Started: " << te->Description << std::endl;
            } else {
                std::cout << "Started." << std::endl;
            }
            kopsik_time_entry_clear(te);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("stop" == args[0]) {
            TogglTimeEntry *te = kopsik_time_entry_init();
            if (KOPSIK_API_FAILURE == kopsik_stop(
                    ctx_, err, ERRLEN, te)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_clear(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            if (KOPSIK_API_FAILURE == kopsik_sync(ctx_, err, ERRLEN)) {
                std::cerr << err << std::endl;
            }
            if (te->Description) {
                std::cout << "Stopped: " << te->Description << std::endl;
            } else {
                std::cout << "Stopped." << std::endl;
            }
            kopsik_time_entry_clear(te);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("list" == args[0]) {
            TogglTimeEntryList *list = kopsik_time_entry_list_init();
            if (KOPSIK_API_FAILURE == kopsik_time_entries(
                    ctx_, err, ERRLEN, list)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_list_clear(list);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            std::cout << "Found " << list->length << " time entries."
                << std::endl;
            kopsik_time_entry_list_clear(list);
            return Poco::Util::Application::EXIT_OK;
        }

        usage();
        return Poco::Util::Application::EXIT_USAGE;
    }
}  // namespace command_line_client
