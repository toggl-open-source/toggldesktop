#include "online_test_app.h"

#include "online_test_dispatcher.h"

#include <Poco/Path.h>

#include <iostream>
#include <locale>
#include <codecvt>

namespace test {

test::App::App() {
    context_ = toggl_context_init("DesktopOnlineTests", "0.1");

    Dispatcher::wireUp(context_, this);

    //std::remove("test.log");
    toggl_set_log_path("test.log");

    std::remove(TESTDB);
    poco_assert(toggl_set_db_path(context_, TESTDB));

    // FIXME better path handling
    Poco::Path path("../../../toggldesktop/src/ssl/cacert.pem");
    toggl_set_cacert_path(context_, path.toString().c_str());

    Dispatcher::dispatch(false);
}

test::App::~App()
{
    toggl_context_clear(context_);
    Dispatcher::dispatch();
}

std::string test::App::randomString(size_t length) {
    auto randchar = []() -> char {
        const char charset[] = "0123456789"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ static_cast<size_t>(rand()) % max_index ];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

std::string test::App::randomUser() {
    srand(time(nullptr));
    return randomString(16) + "@" + "toggldesktop-test.com";
}

std::string test::App::randomPassword() {
    return randomString(16);
}

std::string App::errorSinceLastTime() {
    auto copy = lastError_;
    lastError_.clear();
    return copy;
}

void App::getCountries() {
    toggl_get_countries(context_);
    return; // TODO for some reason this started freezing
    while (countries_.size() <= 0) {
        Dispatcher::dispatch();
    }
}

void test::App::uiStart() {
    toggl_ui_start(context_);
    isStarted_ = true;
    Dispatcher::dispatch(true);
}

bool App::isStarted() const {
    return isStarted_;
}

bool test::App::signup(std::string name, std::string password, const Country &country) {
    auto ret = toggl_signup(context_, name.c_str(), password.c_str(), country.id_);
    Dispatcher::dispatch(true);
    return ret;
}

bool App::signup(std::string name, std::string password) {
    bool ret;
    if (countries_.empty()) {
        ret = toggl_signup(context_, name.c_str(), password.c_str(), 1);
    }
    else {
        // choose a random one
        auto it = countries_.begin();
        std::advance(it, static_cast<size_t>(rand()) % countries_.size());
        ret = signup(name, password, *it);
    }
    Dispatcher::dispatch(true);
    return ret;
}

bool test::App::login(std::string name, std::string password) {
    auto ret = toggl_login(context_, name.c_str(), password.c_str());
    Dispatcher::dispatch(ret);
    return ret;
}

bool test::App::logout() {
    auto ret = toggl_logout(context_);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::isLoggedIn() const {
    return loggedInId_ > 0 && !loggedInEmail_.empty();
}

std::string App::start(const std::string &description) {
    char *str = toggl_start(context_, description.c_str(), "", 0, 0, nullptr, nullptr, 0, 0, 0);
    std::string ret(str);
    free(str);
    Dispatcher::dispatch(!ret.empty());
    return ret;
}

const TimeEntry &App::runningTimeEntry() const {
    return runningTimeEntry_;
}

bool App::stop() {
    auto ret = toggl_stop(context_, 0);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::timeEntry_setDescription(const std::string &guid, const std::string &value) {
    auto ret = toggl_set_time_entry_description(context_, guid.c_str(), value.c_str());
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::timeEntry_setStart(const std::string &guid, const std::string &value) {
    auto ret = toggl_set_time_entry_start(context_, guid.c_str(), value.c_str());
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::timeEntry_setEnd(const std::string &guid, const std::string &value) {
    auto ret = toggl_set_time_entry_end(context_, guid.c_str(), value.c_str());
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::timeEntry_setDuration(const std::string &guid, const std::string &value) {
    auto ret = toggl_set_time_entry_duration(context_, guid.c_str(), value.c_str());
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::timeEntry_setBillable(const std::string &guid, bool value) {
    auto ret = toggl_set_time_entry_billable(context_, guid.c_str(), value);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::timeEntry_setTags(const std::string &guid, const std::list<std::string> &value) {
    std::string merged;
    for (auto it = value.begin(); it != value.end(); it++) {
        merged += *it;
        if (std::next(it) != value.end())
            merged += "\t";
    }
    auto ret = toggl_set_time_entry_tags(context_, guid.c_str(), merged.c_str());
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::timeEntry_setDate(const std::string &guid, int64_t value) {
    auto ret = toggl_set_time_entry_date(context_, guid.c_str(), value);
    Dispatcher::dispatch(ret);
    return ret;
}

std::string App::client_create(const std::string &name) {
    char *str = toggl_create_client(context_, selectedWorkspace_.id_, name.c_str());
    std::string ret(str);
    free(str);
    auto oldSize = clients_.size();
    while (!ret.empty() && clients_.size() == oldSize)
        Dispatcher::dispatch(!ret.empty());
    return ret;
}

std::string App::timeEntry_addProject(const std::string &teGuid, const std::string &cGuid, const std::string &name, const std::string &color) {
    char *str = toggl_add_project(context_, teGuid.c_str(), selectedWorkspace_.id_, 0, cGuid.c_str(), name.c_str(), 1, color.c_str());
    std::string ret(str);
    free(str);
    Dispatcher::dispatch(!ret.empty());
    return ret;
}

bool App::timeEntry_setProject(const std::string &teGuid, const std::string &pGuid) {
    auto ret = toggl_set_time_entry_project(context_, teGuid.c_str(), 0, 0, pGuid.c_str());
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_remindDays(bool remind_mon, bool remind_tue, bool remind_wed, bool remind_thu, bool remind_fri, bool remind_sat, bool remind_sun) {
    auto ret = toggl_set_settings_remind_days(context_, remind_mon, remind_tue, remind_wed, remind_thu, remind_fri, remind_sat, remind_sun);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_remindTimes(const std::string &remind_starts, const std::string &remind_ends) {
    auto ret = toggl_set_settings_remind_times(context_, remind_starts.c_str(), remind_ends.c_str());
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_useIdleDetection(bool use_idle_detection) {
    auto ret = toggl_set_settings_use_idle_detection(context_, use_idle_detection);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_Autotrack(bool value) {
    auto ret = toggl_set_settings_autotrack(context_, value);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_openEditorOnShortcut(bool value) {
    auto ret = toggl_set_settings_open_editor_on_shortcut(context_, value);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_autodetectProxy(bool autodetect_proxy) {
    auto ret = toggl_set_settings_autodetect_proxy(context_, autodetect_proxy);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_menubarTimer(bool menubar_timer) {
    auto ret = toggl_set_settings_menubar_timer(context_, menubar_timer);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_menubarProject(bool menubar_project) {
    auto ret = toggl_set_settings_menubar_project(context_, menubar_project);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_dockIcon(bool dock_icon) {
    auto ret = toggl_set_settings_dock_icon(context_, dock_icon);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_onTop(bool on_top) {
    auto ret = toggl_set_settings_on_top(context_, on_top);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_reminder(bool reminder) {
    auto ret = toggl_set_settings_reminder(context_, reminder);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_pomodoro(bool pomodoro) {
    auto ret = toggl_set_settings_pomodoro(context_, pomodoro);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_pomodoroBreak(bool pomodoro_break) {
    auto ret = toggl_set_settings_pomodoro_break(context_, pomodoro_break);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_stopEntryOnShutdownSleep(bool stop_entry) {
    auto ret = toggl_set_settings_stop_entry_on_shutdown_sleep(context_, stop_entry);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_idleMinutes(uint64_t idle_minutes) {
    auto ret = toggl_set_settings_idle_minutes(context_, idle_minutes);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_focusOnShortcut(bool focus_on_shortcut) {
    auto ret = toggl_set_settings_focus_on_shortcut(context_, focus_on_shortcut);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_reminderMinutes(uint64_t reminder_minutes) {
    auto ret = toggl_set_settings_reminder_minutes(context_, reminder_minutes);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_pomodoroMinutes(uint64_t pomodoro_minutes) {
    auto ret = toggl_set_settings_pomodoro_minutes(context_, pomodoro_minutes);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_pomodoroBreakMinutes(uint64_t pomodoro_break_minutes) {
    auto ret = toggl_set_settings_pomodoro_break_minutes(context_, pomodoro_break_minutes);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_manualMode(bool manual_mode) {
    auto ret = toggl_set_settings_manual_mode(context_, manual_mode);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_analyticsOptedOut(bool analytics_opted_out) {
    auto ret = toggl_set_settings_analytics_opted_out(context_, analytics_opted_out);
    Dispatcher::dispatch(ret);
    return ret;
}

bool App::settings_proxy(bool use_proxy, const std::string &proxy_host, uint64_t proxy_port, const std::string &proxy_username, const std::string &proxy_password) {
    auto ret = toggl_set_proxy_settings(context_, use_proxy, proxy_host.c_str(), proxy_port, proxy_username.c_str(), proxy_password.c_str());
    Dispatcher::dispatch(ret);
    return ret;
}

const std::set<Country> &App::countries() const {
    return countries_;
}

const std::list<TimeEntry> &App::timeEntries() const {
    return timeEntries_;
}

const std::set<Client> &App::clients() const {
    return clients_;
}

const Settings &App::settings() const {
    return settings_;
}

void test::App::on_app(bool open) {
    WHEREAMI;
}

void test::App::on_pomodoro(const std::string &title, const std::string &informative_text) {
    WHEREAMI;
}

void test::App::on_pomodoro_break(const std::string &title, const std::string &informative_text) {
    WHEREAMI;
}

void test::App::on_sync_state(const int64_t sync_state) {
    WHEREAMI;
}

void test::App::on_update(const std::string &url) {
    WHEREAMI;
}

void test::App::on_unsynced_items(const int64_t count) {
    WHEREAMI;
}

void test::App::on_error(const std::string &errmsg, bool user_error) {
    WHEREAMI;
    lastError_ = errmsg;
}

void test::App::on_online_state(const int64_t state) {
    WHEREAMI;
}

void test::App::on_url(const std::string &url) {
    WHEREAMI;
}

void test::App::on_login(bool open, const uint64_t user_id) {
    WHEREAMI;
    if (!user_id)
        settings_ = Settings();
    loggedInId_ = user_id;
    loggedInEmail_ = toggl_get_user_email(context_);
}

void test::App::on_reminder(const std::string &title, const std::string &informative_text) {
    WHEREAMI;
}

void test::App::on_help_articles(const std::list<test::HelpArticle> &list) {
    WHEREAMI;
}

void test::App::on_time_entry_list(bool open, const std::list<test::TimeEntry> &list, bool show_load_more) {
    WHEREAMI;
    timeEntries_ = std::list<test::TimeEntry>(list);
}

void test::App::on_time_entry_autocomplete(const std::list<test::Autocomplete> &list) {
    WHEREAMI;
}

void test::App::on_mini_timer_autocomplete(const std::list<test::Autocomplete> &list) {
    WHEREAMI;
}

void test::App::on_project_autocomplete(const std::list<test::Autocomplete> &list) {
    WHEREAMI;
}

void test::App::on_client_select(const std::list<test::Client> &list) {
    WHEREAMI;
    for (auto i : list)
        clients_.insert(i);
}

void test::App::on_workspace_select(const std::list<test::Workspace> &list) {
    WHEREAMI;
    if (list.size() > 0)
        selectedWorkspace_ = list.front();

    for (auto i : list)
        workspaces_.insert(i);
}

void test::App::on_tags(const std::list<test::Tag> &list) {
    WHEREAMI;
}

void test::App::on_time_entry_editor(bool open, const test::TimeEntry &te, const std::string &focused_field_name) {
    WHEREAMI;
}

void test::App::on_display_settings(bool open, const test::Settings &settings) {
    WHEREAMI;
    settings_ = settings;
}

void test::App::on_project_colors(const std::list<std::string> &color_list, const uint64_t color_count) {
    WHEREAMI;
}

void test::App::on_display_timer_state(const test::TimeEntry &te) {
    WHEREAMI;
    runningTimeEntry_ = te;
}

void test::App::on_display_idle_notification(const std::string &guid, const std::string &since, const std::string &duration, const uint64_t started, const std::string &description, const std::string &project, const std::string &task, const std::string &projectColor) {
    WHEREAMI;
}

void test::App::on_countries(const std::list<test::Country> &list) {
    WHEREAMI;
    for (auto i : list) {
        countries_.insert(i);
    }
}

void App::on_display_overlay(const int64_t type) {
    WHEREAMI;
}

void App::on_display_promotion(const int64_t promotion_type) {
    WHEREAMI;
}

void App::on_display_update_download_state(const std::string &version, const int64_t download_state) {
    WHEREAMI;
}

} // namespace test
