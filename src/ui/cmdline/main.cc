// Copyright 2013 Tanel Lebedev

#include "./main.h"

#include <sstream>

#include "Poco/Message.h"
#include "Poco/Util/Application.h"

namespace command_line_client {

bool syncing = false;

std::string model_change_to_string(
    KopsikModelChange &change) {
    std::stringstream ss;
    ss  << "model_type=" << change.ModelType
        << ", change_type=" << change.ChangeType
        << ", model_id=" << change.ModelID
        << ", GUID=" << change.GUID;
    return ss.str();
}

void main_change_callback(
    KopsikModelChange *change) {
    std::cout << "main_change_callback change="
              << model_change_to_string(*change)
              << std::endl;
}

void main_on_error_callback(
    const char *errmsg) {
    std::cerr << "main_on_error_callback errmsg="
              << std::string(errmsg)
              << std::endl;
}

void main_check_updates_callback(
    const _Bool is_update_available,
    const char *url,
    const char *version) {
    std::cout << "main_check_updates_callback is_update_available="
              << is_update_available
              << " url=" << std::string(url)
              << " version=" << std::string(version)
              << std::endl;
}

void main_online_callback() {
    std::cout << "main_online_callback" << std::endl;
}

void main_user_login_callback(const uint64_t user_id,
                              const char *fullname,
                              const char *timeofdayformat) {
    std::cout << "main_user_login_callback user_id=" << user_id
              << " fullname=" << fullname
              << " timeofdayformat=" << timeofdayformat
              << std::endl;
}

Main::Main()
    : ctx_(0) {
    kopsik_set_log_path("kopsik.log");
    ctx_ = kopsik_context_init("cmdline", "0.0.1");

    poco_check_ptr(ctx_);

    kopsik_context_set_view_item_change_callback(ctx_, main_change_callback);
    kopsik_context_set_error_callback(ctx_, main_on_error_callback);
    kopsik_context_set_check_update_callback(ctx_, main_check_updates_callback);
    kopsik_context_set_online_callback(ctx_, main_online_callback);
    kopsik_context_set_user_login_callback(ctx_, main_user_login_callback);
}

Main::~Main() {
    kopsik_context_clear(ctx_);
}

void Main::exception(const Poco::Exception& exc) {
    std::cerr << exc.displayText() << std::endl;
}

void Main::exception(const std::exception& exc) {
    std::cerr << exc.what() << std::endl;
}

void Main::exception() {
    std::cerr << "unknown exception" << std::endl;
}

void Main::initialize(Poco::Util::Application &self) { // NOLINT
    Poco::Util::Application::initialize(self);
};

void Main::usage() const {
    std::cout << "Recognized commands are: "
              "sync, start, stop, status, list, continue, listen"
              << std::endl;
}

void Main::defineOptions(Poco::Util::OptionSet& options) { // NOLINT
    Poco::Util::Application::defineOptions(options);
    options.addOption(Poco::Util::Option(
        "verbose", "v", "verbose logging, to the console"));
}

int Main::sync() {
    syncing = true;
    kopsik_sync(ctx_);
    while (syncing) {
        Poco::Thread::sleep(1000);
    }
    return Poco::Util::Application::EXIT_OK;
}

int Main::continueTimeEntry() {
    KopsikTimeEntryViewItem *first = 0;
    if (!kopsik_time_entry_view_items(
        ctx_, &first)) {
        kopsik_time_entry_view_item_clear(first);
        return Poco::Util::Application::EXIT_SOFTWARE;
    }

    if (!first) {
        std::cout << "No time entry found to continue." << std::endl;
        kopsik_time_entry_view_item_clear(first);
        return Poco::Util::Application::EXIT_OK;
    }

    if (!kopsik_continue(ctx_, first->GUID)) {
        kopsik_time_entry_view_item_clear(first);
        return Poco::Util::Application::EXIT_SOFTWARE;
    }

    kopsik_time_entry_view_item_clear(first);
    return Poco::Util::Application::EXIT_OK;
}

int Main::status() {
    KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
    _Bool found(false);
    if (!kopsik_running_time_entry_view_item(
        ctx_, te, &found)) {
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

int Main::main(const std::vector<std::string>& args) {
    if (args.empty()) {
        usage();
        return Poco::Util::Application::EXIT_USAGE;
    }

    char* apitoken = getenv("TOGGL_API_TOKEN");
    if (!apitoken) {
        std::cerr << "Please set TOGGL_API_TOKEN in environment"
                  << std::endl;
        return Poco::Util::Application::EXIT_USAGE;
    }

    if (!kopsik_set_db_path(ctx_, "kopsik.db")) {
        return Poco::Util::Application::EXIT_SOFTWARE;
    }

    Poco::ErrorHandler::set(this);

    // Start session in lib
    if (!kopsik_set_api_token(ctx_, apitoken)) {
        return Poco::Util::Application::EXIT_SOFTWARE;
    }

    kopsik_context_start_events(ctx_);

    // Handle commands
    if ("sync" == args[0]) {
        return sync();
    }
    if ("status" == args[0]) {
        return status();
    }
    if ("start" == args[0]) {
        return startTimeEntry();
    }
    if ("stop" == args[0]) {
        return stopTimeEntry();
    }
    if ("list" == args[0]) {
        return listTimeEntries();
    }
    if ("listen" == args[0]) {
        return listenToWebSocket();
    }
    if ("continue" == args[0]) {
        return continueTimeEntry();
    }

    usage();
    return Poco::Util::Application::EXIT_USAGE;
}

std::string Main::timeEntryToString(
    KopsikTimeEntryViewItem *item) {
    std::stringstream ss;
    ss << item->Description;
    if (item->ProjectAndTaskLabel) {
        ss << " [" << item->ProjectAndTaskLabel << "]";
    }
    if (item->Duration) {
        ss << " (" << item->Duration << ")";
    }
    return ss.str();
}

int Main::listenToWebSocket() {
    std::cout << "Listening to websocket.. " << std::endl;
    kopsik_websocket_switch(ctx_, 1);
    while (true) {
        Poco::Thread::sleep(1000);
    }
    kopsik_websocket_switch(ctx_, 0);
    return Poco::Util::Application::EXIT_OK;
}

int Main::listTimeEntries() {
    KopsikTimeEntryViewItem *first = 0;
    if (!kopsik_time_entry_view_items(ctx_, &first)) {
        kopsik_time_entry_view_item_clear(first);
        return Poco::Util::Application::EXIT_SOFTWARE;
    }
    int n = 0;
    while (true) {
        std::cout << timeEntryToString(first) << std::endl;
        n++;
        if (!first->Next) {
            break;
        }
        first = reinterpret_cast<KopsikTimeEntryViewItem *>(first->Next);
    }
    std::cout << "Got " << n << " time entry view items." << std::endl;
    kopsik_time_entry_view_item_clear(first);
    return Poco::Util::Application::EXIT_OK;
}

int Main::startTimeEntry() {
    if (!kopsik_start(
        ctx_, "New time entry", "", 0, 0)) {
        return Poco::Util::Application::EXIT_SOFTWARE;
    }
    return Poco::Util::Application::EXIT_OK;
}

int Main::stopTimeEntry() {
    if (!kopsik_stop(ctx_)) {
        return Poco::Util::Application::EXIT_SOFTWARE;
    }
    return Poco::Util::Application::EXIT_OK;
}

}  // namespace command_line_client
