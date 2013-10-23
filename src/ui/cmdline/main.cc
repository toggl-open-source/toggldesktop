// Copyright 2013 Tanel Lebedev

#include "./main.h"

#include <sstream>

#include "Poco/Message.h"
#include "Poco/Util/Application.h"

#define ERRLEN 1024

namespace command_line_client {

    void Main::usage() {
        std::cout << "Recognized commands are: "
            "sync, start, stop, status, pushable, list, continue, listen"
            << std::endl;
    }

    void Main::defineOptions(Poco::Util::OptionSet& options) {
        Poco::Util::Application::defineOptions(options);
        options.addOption(Poco::Util::Option(
            "verbose", "v", "verbose logging, to the console"));
    }

    std::string KopsikModelChangeToString(
            KopsikModelChange &change) {
        std::stringstream ss;
        ss  << "model_type=" << change.ModelType
            << ", change_type=" << change.ChangeType
            << ", model_id=" << change.ModelID
            << ", GUID=" << change.GUID;
        return ss.str();
    }

    std::string KopsikTimeEntryViewItemToString(
            KopsikTimeEntryViewItem *item) {
        std::stringstream ss;
        ss << "description: " << item->Description;
        if (item->Project) {
            ss << " project: " << item->Project;
        }
        if (item->Duration) {
            ss << " duration: " << item->Duration;
        }
        return ss.str();
    }

    void on_model_change(kopsik_api_result result,
            const char *err_string,
            KopsikModelChange *change) {
        if (KOPSIK_API_SUCCESS != result) {
            std::string err(err_string);
            std::cerr << "on_model_change error! "
                << err << std::endl;
            return;
        }
        std::cout << "on_view_item_change "
            << KopsikModelChangeToString(*change)
            << std::endl;
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

        kopsik_set_db_path(ctx, "kopsik.db");
        kopsik_set_log_path(ctx, "kopsik.log");

        Poco::ErrorHandler::set(this);

        // Start session in lib
        char err[ERRLEN];
        std::fill(err, err + ERRLEN, 0);
        if (KOPSIK_API_SUCCESS != kopsik_set_api_token(
                ctx, err, ERRLEN, apiToken)) {
            std::cerr << err << std::endl;
            return Poco::Util::Application::EXIT_SOFTWARE;
        }

        // Load user that is referenced by the session
        KopsikUser *user = kopsik_user_init();
        if (KOPSIK_API_SUCCESS != kopsik_current_user(
                ctx, err, ERRLEN, user)) {
            std::cerr << err << std::endl;
            return Poco::Util::Application::EXIT_SOFTWARE;
        }
        kopsik_user_clear(user);

        if ("sync" == args[0]) {
            if (KOPSIK_API_SUCCESS != kopsik_sync(ctx, err, ERRLEN, 1)) {
                std::cerr << err << std::endl;
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            std::cout << "Synced." << std::endl;
            return Poco::Util::Application::EXIT_OK;
        }

        if ("status" == args[0]) {
            KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
            int found(0);
            if (KOPSIK_API_SUCCESS != kopsik_running_time_entry_view_item(
                    ctx, err, ERRLEN, te, &found)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_view_item_clear(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            if (found) {
                std::cout << "Tracking: " << te->Description << std::endl;
            } else {
                std::cout << "Not tracking." << std::endl;
            }
            kopsik_time_entry_view_item_clear(te);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("pushable" == args[0]) {
            KopsikPushableModelStats stats;
            if (KOPSIK_API_SUCCESS != kopsik_pushable_models(
                    ctx, err, ERRLEN, &stats)) {
                std::cerr << err << std::endl;
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            std::cout << stats.TimeEntries << " pushable time entries." << std::endl;
            return Poco::Util::Application::EXIT_OK;
        }

        if ("start" == args[0]) {
            KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
            if (KOPSIK_API_SUCCESS != kopsik_start(
                    ctx, err, ERRLEN, "New time entry", te)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_view_item_clear(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            if (KOPSIK_API_SUCCESS != kopsik_push(ctx, err, ERRLEN)) {
                std::cerr << err << std::endl;
            }
            if (te->Description) {
                std::cout << "Started: " << te->Description << std::endl;
            } else {
                std::cout << "Started." << std::endl;
            }
            kopsik_time_entry_view_item_clear(te);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("stop" == args[0]) {
            KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
            int was_found(0);
            if (KOPSIK_API_SUCCESS != kopsik_stop(
                    ctx, err, ERRLEN, te, &was_found)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_view_item_clear(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            if (KOPSIK_API_SUCCESS != kopsik_push(ctx, err, ERRLEN)) {
                std::cerr << err << std::endl;
            }
            if (was_found) {
                if (te->Description) {
                    std::cout << "Stopped: " << te->Description << std::endl;
                } else {
                    std::cout << "Stopped." << std::endl;
                }
            } else {
                std::cout << "No time entry found to stop." << std::endl;
            }
            kopsik_time_entry_view_item_clear(te);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("list" == args[0]) {
            KopsikTimeEntryViewItemList *list =
                kopsik_time_entry_view_item_list_init();
            if (KOPSIK_API_SUCCESS != kopsik_time_entry_view_items(
                    ctx, err, ERRLEN, list)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_view_item_list_clear(list);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            for (unsigned int i = 0; i < list->Length; i++) {
                KopsikTimeEntryViewItem *item = list->ViewItems[i];
                std::cout << KopsikTimeEntryViewItemToString(item) << std::endl;
            }
            std::cout << "Got " << list->Length << " time entry view items."
                << std::endl;
            kopsik_time_entry_view_item_list_clear(list);
            return Poco::Util::Application::EXIT_OK;
        }

        if ("listen" == args[0]) {
            std::cout << "Listening to websocket.. " << std::endl;
            kopsik_set_change_callback(ctx, on_model_change);
            if (KOPSIK_API_SUCCESS != kopsik_websocket_start(ctx, err, ERRLEN)) {
                std::cerr << "Error starting websocket: "
                    << err << std::endl;
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            while (true) {
                Poco::Thread::sleep(1000);
            }
            if (KOPSIK_API_SUCCESS != kopsik_websocket_stop(ctx, err, ERRLEN)) {
                std::cerr << "Error stopping websocket: "
                    << err << std::endl;
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
            return Poco::Util::Application::EXIT_OK;
        }

        if ("continue" == args[0]) {
            KopsikTimeEntryViewItemList *list =
                kopsik_time_entry_view_item_list_init();

            if (KOPSIK_API_SUCCESS != kopsik_time_entry_view_items(
                    ctx, err, ERRLEN, list)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_view_item_list_clear(list);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }

            if (!list->Length) {
                std::cout << "No time entry found to continue." << std::endl;
                kopsik_time_entry_view_item_list_clear(list);
                return Poco::Util::Application::EXIT_OK;
            }

            KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
            if (KOPSIK_API_SUCCESS != kopsik_continue(
                    ctx, err, ERRLEN, list->ViewItems[0]->GUID, te)) {
                std::cerr << err << std::endl;
                kopsik_time_entry_view_item_list_clear(list);
                kopsik_time_entry_view_item_clear(te);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }

            if (KOPSIK_API_SUCCESS != kopsik_push(ctx, err, ERRLEN)) {
                std::cerr << err << std::endl;
            }

            if (te->Description) {
                std::cout << "Started: " << te->Description << std::endl;
            } else {
                std::cout << "Started." << std::endl;
            }

            kopsik_time_entry_view_item_list_clear(list);
            kopsik_time_entry_view_item_clear(te);
            return Poco::Util::Application::EXIT_OK;
        }

        usage();
        return Poco::Util::Application::EXIT_USAGE;
    }
}  // namespace command_line_client
