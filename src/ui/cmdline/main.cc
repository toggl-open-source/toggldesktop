// Copyright 2013 Tanel Lebedev

#include "./main.h"

#include <sstream>

#include "Poco/Message.h"
#include "Poco/Util/Application.h"

#define ERRLEN 1024

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
    kopsik_api_result result,
    const char *errmsg,
    KopsikModelChange *change) {
  if (KOPSIK_API_SUCCESS != result) {
    std::cerr << "main_change_callback errmsg="
              << std::string(errmsg)
              << std::endl;
    return;
  }
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
    kopsik_api_result result,
    const char *errmsg,
    const int is_update_available,
    const char *url,
    const char *version) {
  if (KOPSIK_API_SUCCESS != result) {
    std::cerr << "main_check_updates_callback errmsg="
              << std::string(errmsg)
              << std::endl;
    return;
  }
  std::cout << "main_check_updates_callback is_update_available="
            << is_update_available
            << " url=" << std::string(url)
            << " version=" << std::string(version)
            << std::endl;
}

void main_online_callback() {
  std::cout << "main_online_callback" << std::endl;
}

void on_sync_result(
    kopsik_api_result result,
    const char *errmsg) {
  if (KOPSIK_API_SUCCESS != result) {
    std::cerr << "Error " << std::string(errmsg) << std::endl;
    syncing = false;
    return;
  }
  std::cout << "Success" << std::endl;
  syncing = false;
}

Main::Main()
    : ctx_(0) {
  kopsik_set_log_path("kopsik.log");
  ctx_ = kopsik_context_init(
    "cmdline",
    "0.0.1",
    main_change_callback,
    main_on_error_callback,
    main_check_updates_callback,
    main_online_callback);
  poco_assert(ctx_);
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
    "sync, start, stop, status, pushable, list, continue, listen"
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
  char errmsg[ERRLEN];
  KopsikTimeEntryViewItem *first = 0;
  if (KOPSIK_API_SUCCESS != kopsik_time_entry_view_items(
      ctx_, errmsg, ERRLEN, &first)) {
    std::cerr << std::string(errmsg) << std::endl;
    kopsik_time_entry_view_item_clear(first);
    return Poco::Util::Application::EXIT_SOFTWARE;
  }

  if (!first) {
    std::cout << "No time entry found to continue." << std::endl;
    kopsik_time_entry_view_item_clear(first);
    return Poco::Util::Application::EXIT_OK;
  }

  KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
  if (KOPSIK_API_SUCCESS != kopsik_continue(
      ctx_, errmsg, ERRLEN, first->GUID, te)) {
    std::cerr << std::string(errmsg) << std::endl;
    kopsik_time_entry_view_item_clear(first);
    kopsik_time_entry_view_item_clear(te);
    return Poco::Util::Application::EXIT_SOFTWARE;
  }

  kopsik_time_entry_view_item_clear(first);
  kopsik_time_entry_view_item_clear(te);
  return Poco::Util::Application::EXIT_OK;
}

int Main::status() {
  KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
  int found(0);
  char errmsg[ERRLEN];
  if (KOPSIK_API_SUCCESS != kopsik_running_time_entry_view_item(
      ctx_, errmsg, ERRLEN, te, &found)) {
    std::cerr << std::string(errmsg) << std::endl;
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

  char errmsg[ERRLEN];
  if (KOPSIK_API_SUCCESS != kopsik_set_db_path(
      ctx_, errmsg, ERRLEN, "kopsik.db")) {
    std::cerr << errmsg << std::endl;
    return Poco::Util::Application::EXIT_SOFTWARE;
  }

  Poco::ErrorHandler::set(this);

  // Start session in lib
  if (KOPSIK_API_SUCCESS != kopsik_set_api_token(
      ctx_, errmsg, ERRLEN, apitoken)) {
    std::cerr << errmsg << std::endl;
    return Poco::Util::Application::EXIT_SOFTWARE;
  }

  // Load user that is referenced by the session
  KopsikUser *user = kopsik_user_init();
  if (KOPSIK_API_SUCCESS != kopsik_current_user(
      ctx_, errmsg, ERRLEN, user)) {
    std::cerr << std::string(errmsg) << std::endl;
    return Poco::Util::Application::EXIT_SOFTWARE;
  }
  kopsik_user_clear(user);

  // Handle commands
  if ("sync" == args[0]) {
    return sync();
  }
  if ("status" == args[0]) {
    return status();
  }
  if ("pushable" == args[0]) {
    return showPushableData();
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

int Main::showPushableData() {
  KopsikPushableModelStats stats;
  char errmsg[ERRLEN];
  if (KOPSIK_API_SUCCESS != kopsik_pushable_models(
          ctx_, errmsg, ERRLEN, &stats)) {
      std::cerr << std::string(errmsg) << std::endl;
      return Poco::Util::Application::EXIT_SOFTWARE;
  }
  std::cout << stats.TimeEntries
            << " pushable time entries."
            << std::endl;
  return Poco::Util::Application::EXIT_OK;
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
  char errmsg[ERRLEN];
  if (KOPSIK_API_SUCCESS != kopsik_time_entry_view_items(
      ctx_, errmsg, ERRLEN, &first)) {
    std::cerr << std::string(errmsg) << std::endl;
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
  KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
  char errmsg[ERRLEN];
  if (KOPSIK_API_SUCCESS != kopsik_start(
    ctx_, errmsg, ERRLEN, "New time entry", "", 0, 0, te)) {
    std::cerr << errmsg << std::endl;
    kopsik_time_entry_view_item_clear(te);
    return Poco::Util::Application::EXIT_SOFTWARE;
  }
  kopsik_time_entry_view_item_clear(te);
  return Poco::Util::Application::EXIT_OK;
}

int Main::stopTimeEntry() {
  KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
  int was_found(0);
  char errmsg[ERRLEN];
  if (KOPSIK_API_SUCCESS != kopsik_stop(
      ctx_, errmsg, ERRLEN, te, &was_found)) {
    std::cerr << errmsg << std::endl;
    kopsik_time_entry_view_item_clear(te);
    return Poco::Util::Application::EXIT_SOFTWARE;
  }
  if (!was_found) {
    std::cout << "No time entry found to stop." << std::endl;
  }
  kopsik_time_entry_view_item_clear(te);
  return Poco::Util::Application::EXIT_OK;
}

}  // namespace command_line_client
