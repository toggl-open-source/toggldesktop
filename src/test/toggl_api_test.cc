// Copyright 2014 Toggl Desktop developers.

#include <vector>

#include "gtest/gtest.h"

#include "./../proxy.h"
#include "./../settings.h"
#include "./../time_entry.h"
#include "./../toggl_api.h"
#include "./../toggl_api_private.h"
#include "./test_data.h"

#include "Poco/DateTime.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/LocalDateTime.h"
#include "Poco/Path.h"

namespace toggl {

namespace testing {

namespace testresult {

// on_url
std::string url("");

// on_reminder
std::string reminder_title("");
std::string reminder_informative_text("");
std::string error("");

// on_online_state
int64_t online_state(0);

// on_login
uint64_t user_id(0);

// on_display_settings
_Bool open_settings(false);
Settings settings;
bool use_proxy(false);
Proxy proxy;

// on_display_idle_notification
std::string idle_guid("");
std::string idle_since("");
std::string idle_duration("");
uint64_t idle_started(0);
std::string idle_description("");

// on_display_timer_state
TimeEntry timer_state;

// on_project_autocomplete
std::vector<std::string> projects;

// on_client_select
std::vector<std::string> clients;

// on_time_entry_list
std::vector<TimeEntry> time_entries;

TimeEntry time_entry_by_guid(const std::string guid) {
    TimeEntry te;
    for (std::size_t i = 0; i < testing::testresult::time_entries.size();
            i++) {
        if (testing::testresult::time_entries[i].GUID() == guid) {
            te = testing::testresult::time_entries[i];
            break;
        }
    }
    return te;
}

// on_time_entry_editor
TimeEntry editor_state;
_Bool editor_open(false);
std::string editor_focused_field_name("");

bool on_app_open;

int64_t sync_state;

int64_t unsynced_item_count;

std::string update_url;

}  // namespace testresult

void on_app(const _Bool open) {
    testresult::on_app_open = open;
}

void on_sync_state(const int64_t sync_state) {
    testresult::sync_state = sync_state;
}

void on_update(const char_t *url) {
    testresult::update_url = std::string(url);
}

void on_unsynced_items(const int64_t count) {
    testresult::unsynced_item_count = count;
}

void on_error(
    const char *errmsg,
    const _Bool user_error) {
    if (errmsg) {
        testresult::error = std::string(errmsg);
        return;
    }
    testresult::error = std::string("");
}

void on_online_state(const int64_t state) {
    testresult::online_state = state;
}

void on_url(const char *url) {
    testresult::url = std::string(url);
}

void on_login(const _Bool open, const uint64_t user_id) {
    testresult::user_id = user_id;
}

void on_reminder(const char *title, const char *informative_text) {
    testresult::reminder_title = std::string(title);
    testresult::reminder_informative_text = std::string(informative_text);
}

void on_time_entry_list(
    const _Bool open,
    TogglTimeEntryView *first) {
    testing::testresult::time_entries.clear();
    TogglTimeEntryView *it = first;
    while (it) {
        TimeEntry te;
        te.SetGUID(it->GUID);
        te.SetDurationInSeconds(it->DurationInSeconds);
        te.SetDescription(it->Description);
        te.SetStart(it->Started);
        te.SetStop(it->Ended);
        testing::testresult::time_entries.push_back(te);
        it = reinterpret_cast<TogglTimeEntryView *>(it->Next);
    }
}

void on_time_entry_autocomplete(TogglAutocompleteView *first) {
}

void on_mini_timer_autocomplete(TogglAutocompleteView *first) {
}

void on_project_autocomplete(TogglAutocompleteView *first) {
    testing::testresult::projects.clear();
    TogglAutocompleteView *it = first;
    while (it) {
        testing::testresult::projects.push_back(
            std::string(it->ProjectLabel));
        it = reinterpret_cast<TogglAutocompleteView *>(it->Next);
    }
}

void on_client_select(TogglGenericView *first) {
    testing::testresult::clients.clear();
    TogglGenericView *it = first;
    while (it) {
        testing::testresult::clients.push_back(std::string(it->Name));
        it = reinterpret_cast<TogglGenericView *>(it->Next);
    }
}

void on_workspace_select(TogglGenericView *first) {
}

void on_tags(TogglGenericView *first) {
}

void on_time_entry_editor(
    const _Bool open,
    TogglTimeEntryView *te,
    const char *focused_field_name) {
    testing::testresult::editor_state = TimeEntry();
    testing::testresult::editor_state.SetGUID(te->GUID);
    testing::testresult::editor_open = open;
    testing::testresult::editor_focused_field_name =
        std::string(focused_field_name);
}

void on_display_settings(
    const _Bool open,
    TogglSettingsView *settings) {

    testing::testresult::open_settings = open;

    testing::testresult::settings.use_idle_detection =
        settings->UseIdleDetection;
    testing::testresult::settings.menubar_project = settings->MenubarProject;
    testing::testresult::settings.autodetect_proxy = settings->AutodetectProxy;
    testing::testresult::settings.menubar_timer = settings->MenubarTimer;
    testing::testresult::settings.reminder = settings->Reminder;
    testing::testresult::settings.dock_icon = settings->DockIcon;
    testing::testresult::settings.on_top = settings->OnTop;
    testing::testresult::settings.idle_minutes = settings->IdleMinutes;
    testing::testresult::settings.reminder_minutes = settings->ReminderMinutes;
    testing::testresult::settings.focus_on_shortcut = settings->FocusOnShortcut;
    testing::testresult::settings.manual_mode = settings->ManualMode;

    testing::testresult::use_proxy = settings->UseProxy;

    testing::testresult::proxy.SetHost(std::string(settings->ProxyHost));
    testing::testresult::proxy.SetPort(settings->ProxyPort);
    testing::testresult::proxy.SetUsername(
        std::string(settings->ProxyUsername));
    testing::testresult::proxy.SetPassword(
        std::string(settings->ProxyPassword));
}

void on_display_timer_state(TogglTimeEntryView *te) {
    testing::testresult::timer_state = TimeEntry();
    if (te) {
        testing::testresult::timer_state.SetStart(te->Started);
        testing::testresult::timer_state.SetGUID(te->GUID);
        testing::testresult::timer_state.SetDurationInSeconds(
            te->DurationInSeconds);
        testing::testresult::timer_state.SetDescription(te->Description);
        if (te->Tags) {
            testing::testresult::timer_state.SetTags(te->Tags);
        }
        testing::testresult::timer_state.SetBillable(te->Billable);
    }
}

void on_display_idle_notification(
    const char *guid,
    const char *since,
    const char *duration,
    const uint64_t started,
    const char *description) {
    testing::testresult::idle_since = std::string(since);
    testing::testresult::idle_started = started;
    testing::testresult::idle_duration = std::string(duration);
    testing::testresult::idle_guid = std::string(guid);
    testing::testresult::idle_description = std::string(description);
}

void on_apply_settings(
    TogglSettingsView *settings) {
    testing::testresult::use_proxy = settings->UseProxy;
    testing::testresult::settings.use_idle_detection =
        settings->UseIdleDetection;
    testing::testresult::settings.menubar_timer =
        settings->MenubarTimer;
    testing::testresult::settings.dock_icon = settings->DockIcon;
    testing::testresult::settings.on_top = settings->OnTop;
}

class App {
 public:
    App() {
        Poco::File f(TESTDB);
        if (f.exists()) {
            f.remove(false);
        }

        toggl_set_log_path("test.log");

        ctx_ = toggl_context_init("tests", "0.1");

        poco_assert(toggl_set_db_path(ctx_, TESTDB));

        Poco::Path path("src/ssl/cacert.pem");
        toggl_set_cacert_path(ctx_, path.toString().c_str());

        toggl_on_show_app(ctx_, on_app);
        toggl_on_sync_state(ctx_, on_sync_state);
        toggl_on_unsynced_items(ctx_, on_unsynced_items);
        toggl_on_update(ctx_, on_update);
        toggl_on_error(ctx_, on_error);
        toggl_on_online_state(ctx_, on_online_state);
        toggl_on_login(ctx_, on_login);
        toggl_on_url(ctx_, on_url);
        toggl_on_reminder(ctx_, on_reminder);
        toggl_on_time_entry_list(ctx_, on_time_entry_list);
        toggl_on_time_entry_autocomplete(ctx_, on_time_entry_autocomplete);
        toggl_on_mini_timer_autocomplete(ctx_, on_mini_timer_autocomplete);
        toggl_on_project_autocomplete(ctx_, on_project_autocomplete);
        toggl_on_workspace_select(ctx_, on_workspace_select);
        toggl_on_client_select(ctx_, on_client_select);
        toggl_on_tags(ctx_, on_tags);
        toggl_on_time_entry_editor(ctx_, on_time_entry_editor);
        toggl_on_settings(ctx_, on_display_settings);
        toggl_on_timer_state(ctx_, on_display_timer_state);
        toggl_on_idle_notification(ctx_, on_display_idle_notification);

        poco_assert(toggl_ui_start(ctx_));
    }
    ~App() {
        toggl_context_clear(ctx_);
        ctx_ = 0;
    }

    void *ctx() {
        return ctx_;
    }

 private:
    void *ctx_;
};

}  // namespace testing

TEST(TogglApiTest, toggl_context_init) {
    testing::App app;
}

TEST(TogglApiTest, testing_sleep) {
    time_t start = time(0);
    testing_sleep(1);
    int elapsed_seconds = time(0) - start;
    ASSERT_EQ(1, elapsed_seconds);
}

TEST(TogglApiTest, toggl_run_script) {
    testing::App app;
    int64_t err(0);
    char *s = toggl_run_script(app.ctx(), "print 'test'", &err);
    std::string res(s);
    free(s);
    ASSERT_EQ(0, err);
    ASSERT_EQ("0 value(s) returned\n\n\n", res);
}

TEST(TogglApiTest, toggl_run_script_with_invalid_script) {
    testing::App app;
    int64_t err(0);
    char *s = toggl_run_script(app.ctx(), "foo bar", &err);
    std::string res(s);
    free(s);
    ASSERT_NE(0, err);
    ASSERT_EQ("[string \"foo bar\"]:1: syntax error near 'bar'", res);
}

TEST(TogglApiTest, toggl_set_settings) {
    testing::App app;

    // set to false/null

    ASSERT_TRUE(toggl_set_settings_menubar_project(app.ctx(), false));
    ASSERT_FALSE(testing::testresult::settings.menubar_project);

    ASSERT_TRUE(toggl_set_settings_autodetect_proxy(app.ctx(), false));
    ASSERT_FALSE(testing::testresult::settings.autodetect_proxy);

    ASSERT_TRUE(toggl_set_settings_use_idle_detection(app.ctx(), false));
    ASSERT_FALSE(testing::testresult::settings.use_idle_detection);

    ASSERT_TRUE(toggl_set_settings_menubar_timer(app.ctx(), false));
    ASSERT_FALSE(testing::testresult::settings.menubar_timer);

    ASSERT_TRUE(toggl_set_settings_dock_icon(app.ctx(), false));
    ASSERT_FALSE(testing::testresult::settings.dock_icon);

    ASSERT_TRUE(toggl_set_settings_on_top(app.ctx(), false));
    ASSERT_FALSE(testing::testresult::settings.on_top);

    ASSERT_TRUE(toggl_set_settings_reminder(app.ctx(), false));
    ASSERT_FALSE(testing::testresult::settings.reminder);

    ASSERT_TRUE(toggl_set_settings_idle_minutes(app.ctx(), 0));
    ASSERT_EQ(Poco::UInt64(1), testing::testresult::settings.idle_minutes);

    ASSERT_TRUE(toggl_set_settings_reminder_minutes(app.ctx(), 0));
    ASSERT_EQ(Poco::UInt64(1), testing::testresult::settings.reminder_minutes);

    ASSERT_TRUE(toggl_set_settings_focus_on_shortcut(app.ctx(), false));
    ASSERT_FALSE(testing::testresult::settings.focus_on_shortcut);

    ASSERT_TRUE(toggl_set_settings_manual_mode(app.ctx(), false));
    ASSERT_FALSE(testing::testresult::settings.manual_mode);

    // set to true / not null

    ASSERT_TRUE(toggl_set_settings_menubar_project(app.ctx(), true));
    ASSERT_TRUE(testing::testresult::settings.menubar_project);

    ASSERT_TRUE(toggl_set_settings_autodetect_proxy(app.ctx(), true));
    ASSERT_TRUE(testing::testresult::settings.autodetect_proxy);

    ASSERT_TRUE(toggl_set_settings_use_idle_detection(app.ctx(), true));
    ASSERT_TRUE(testing::testresult::settings.use_idle_detection);

    ASSERT_TRUE(toggl_set_settings_menubar_timer(app.ctx(), true));
    ASSERT_TRUE(testing::testresult::settings.menubar_timer);

    ASSERT_TRUE(toggl_set_settings_dock_icon(app.ctx(), true));
    ASSERT_TRUE(testing::testresult::settings.dock_icon);

    ASSERT_TRUE(toggl_set_settings_on_top(app.ctx(), true));
    ASSERT_TRUE(testing::testresult::settings.on_top);

    ASSERT_TRUE(toggl_set_settings_reminder(app.ctx(), true));
    ASSERT_TRUE(testing::testresult::settings.reminder);

    ASSERT_TRUE(toggl_set_settings_idle_minutes(app.ctx(), 123));
    ASSERT_EQ(Poco::UInt64(123), testing::testresult::settings.idle_minutes);

    ASSERT_TRUE(toggl_set_settings_reminder_minutes(app.ctx(), 222));
    ASSERT_EQ(Poco::UInt64(222),
              testing::testresult::settings.reminder_minutes);

    ASSERT_TRUE(toggl_set_settings_focus_on_shortcut(app.ctx(), true));
    ASSERT_TRUE(testing::testresult::settings.focus_on_shortcut);

    ASSERT_TRUE(toggl_set_settings_manual_mode(app.ctx(), true));
    ASSERT_TRUE(testing::testresult::settings.manual_mode);
}

TEST(TogglApiTest, toggl_set_proxy_settings) {
    testing::App app;

    ASSERT_TRUE(toggl_set_proxy_settings(
        app.ctx(), 1, "localhost", 8000, "johnsmith", "secret"));

    ASSERT_TRUE(testing::testresult::use_proxy);
    ASSERT_EQ(std::string("localhost"),
              std::string(testing::testresult::proxy.Host()));
    ASSERT_EQ(8000,
              static_cast<int>(testing::testresult::proxy.Port()));
    ASSERT_EQ(std::string("johnsmith"),
              std::string(testing::testresult::proxy.Username()));
    ASSERT_EQ(std::string("secret"),
              std::string(testing::testresult::proxy.Password()));
}

TEST(TogglApiTest, toggl_set_window_settings) {
    testing::App app;

    int64_t x(1), y(2), h(3), w(4);
    testing::testresult::error = "";
    if (!toggl_set_window_settings(app.ctx(), x, y, h, w)) {
        ASSERT_EQ(noError, testing::testresult::error);
    }

    int64_t x1(1), y1(2), h1(3), w1(4);
    testing::testresult::error = "";
    if (!toggl_window_settings(app.ctx(), &x1, &y1, &h1, &w1)) {
        ASSERT_EQ(noError, testing::testresult::error);
    }

    ASSERT_EQ(x, x1);
    ASSERT_EQ(y, y1);
    ASSERT_EQ(h, h1);
    ASSERT_EQ(w, w1);
}

TEST(TogglApiTest, toggl_get_user_fullname) {
    testing::App app;

    char *str = toggl_get_user_fullname(app.ctx());
    ASSERT_EQ("", std::string(str));
    free(str);

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    str = toggl_get_user_fullname(app.ctx());
    ASSERT_EQ("John Smith", std::string(str));
    free(str);
}

TEST(TogglApiTest, toggl_get_user_email) {
    testing::App app;

    char *str = toggl_get_user_email(app.ctx());
    ASSERT_EQ("", std::string(str));
    free(str);

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    str = toggl_get_user_email(app.ctx());
    ASSERT_EQ("johnsmith@toggl.com", std::string(str));
    free(str);
}

TEST(TogglApiTest, toggl_show_app) {
    testing::App app;

    testing::testresult::on_app_open = false;

    toggl_show_app(app.ctx());

    ASSERT_TRUE(testing::testresult::on_app_open);
}

TEST(TogglApiTest, toggl_set_update_channel) {
    testing::App app;

    std::string default_channel("stable");

    // Also check that the API itself thinks the default channel is
    char *str = toggl_get_update_channel(app.ctx());
    ASSERT_TRUE(default_channel == std::string(str)
                || std::string("beta") == std::string(str));
    free(str);

    ASSERT_FALSE(toggl_set_update_channel(app.ctx(), "invalid"));

    // The channel should be the same in the API
    str = toggl_get_update_channel(app.ctx());
    ASSERT_TRUE(default_channel == std::string(str) ||
                "beta" == std::string(str));
    free(str);

    ASSERT_TRUE(toggl_set_update_channel(app.ctx(), "beta"));

    // The channel should have been changed in the API
    str = toggl_get_update_channel(app.ctx());
    ASSERT_EQ(std::string("beta"), std::string(str));
    free(str);

    ASSERT_TRUE(toggl_set_update_channel(app.ctx(), "dev"));

    // The channel should have been changed in the API
    str = toggl_get_update_channel(app.ctx());
    ASSERT_EQ(std::string("dev"), std::string(str));
    free(str);

    ASSERT_TRUE(toggl_set_update_channel(app.ctx(), "stable"));

    // The channel should have been changed in the API
    str = toggl_get_update_channel(app.ctx());
    ASSERT_EQ(std::string("stable"), std::string(str));
    free(str);
}

TEST(TogglApiTest, toggl_set_log_level) {
    toggl_set_log_level("trace");
}

TEST(TogglApiTest, toggl_format_tracking_time_duration) {
    char *str = toggl_format_tracking_time_duration(10);
    ASSERT_EQ("10 sec", std::string(str));
    free(str);

    str = toggl_format_tracking_time_duration(60);
    ASSERT_EQ("01:00 min", std::string(str));
    free(str);

    str = toggl_format_tracking_time_duration(65);
    ASSERT_EQ("01:05 min", std::string(str));
    free(str);

    str = toggl_format_tracking_time_duration(3600);
    ASSERT_EQ("01:00:00", std::string(str));
    free(str);

    str = toggl_format_tracking_time_duration(5400);
    ASSERT_EQ("01:30:00", std::string(str));
    free(str);

    str = toggl_format_tracking_time_duration(5410);
    ASSERT_EQ("01:30:10", std::string(str));
    free(str);
}

TEST(TogglApiTest, toggl_format_tracked_time_duration) {
    char *str  = toggl_format_tracked_time_duration(10);
    ASSERT_EQ("0:00", std::string(str));
    free(str);

    str = toggl_format_tracked_time_duration(60);
    ASSERT_EQ("0:01", std::string(str));
    free(str);

    str = toggl_format_tracked_time_duration(65);
    ASSERT_EQ("0:01", std::string(str));
    free(str);

    str = toggl_format_tracked_time_duration(3600);
    ASSERT_EQ("1:00", std::string(str));
    free(str);

    str = toggl_format_tracked_time_duration(5400);
    ASSERT_EQ("1:30", std::string(str));
    free(str);

    str = toggl_format_tracked_time_duration(5410);
    ASSERT_EQ("1:30", std::string(str));
    free(str);
}

TEST(TogglApiTest, toggl_password_forgot) {
    testing::App app;
    toggl_password_forgot(app.ctx());
    ASSERT_EQ("https://www.toggl.com/forgot-password",
              testing::testresult::url);
}

TEST(TogglApiTest, toggl_set_environment) {
    testing::App app;

    toggl_set_environment(app.ctx(), "test");

    char *env = toggl_environment(app.ctx());
    std::string res(env);
    free(env);
    ASSERT_EQ("test", res);
}

TEST(TogglApiTest, toggl_set_update_path) {
    testing::App app;

    toggl_set_update_path(app.ctx(), "/tmp/");

    char *s = toggl_update_path(app.ctx());
    std::string path(s);
    free(s);

    ASSERT_EQ("/tmp/", path);
}

TEST(TogglApiTest, testing_set_logged_in_user) {
    std::string json = loadTestData();
    testing::App app;
    testing::testresult::error = "";
    _Bool res = testing_set_logged_in_user(app.ctx(), json.c_str());
    ASSERT_EQ(noError, testing::testresult::error);
    ASSERT_TRUE(res);
    ASSERT_EQ(uint64_t(10471231), testing::testresult::user_id);
}

TEST(TogglApiTest, toggl_disable_update_check) {
    testing::App app;
    toggl_disable_update_check(app.ctx());
}

TEST(TogglApiTest, toggl_logout) {
    std::string json = loadTestData();
    testing::App app;
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    ASSERT_EQ(uint64_t(10471231), testing::testresult::user_id);
    ASSERT_TRUE(toggl_logout(app.ctx()));
    ASSERT_EQ(uint64_t(0), testing::testresult::user_id);
}

TEST(TogglApiTest, toggl_clear_cache) {
    std::string json = loadTestData();
    testing::App app;
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    ASSERT_EQ(uint64_t(10471231), testing::testresult::user_id);
    ASSERT_TRUE(toggl_clear_cache(app.ctx()));
    ASSERT_EQ(uint64_t(0), testing::testresult::user_id);
}

TEST(TogglApiTest, toggl_debug) {
    toggl_debug("Test 123");
}

TEST(TogglApiTest, toggl_set_idle_seconds) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    toggl_set_idle_seconds(app.ctx(), 0);
    ASSERT_EQ("", testing::testresult::idle_since);
    ASSERT_EQ(uint64_t(0), testing::testresult::idle_started);
    ASSERT_EQ("", testing::testresult::idle_duration);
    ASSERT_EQ("", testing::testresult::idle_guid);

    toggl_set_idle_seconds(app.ctx(), (5*60)-1);
    ASSERT_EQ("", testing::testresult::idle_since);
    ASSERT_EQ(uint64_t(0), testing::testresult::idle_started);
    ASSERT_EQ("", testing::testresult::idle_duration);
    ASSERT_EQ("", testing::testresult::idle_guid);

    toggl_set_idle_seconds(app.ctx(), (5*60));
    ASSERT_EQ("", testing::testresult::idle_since);
    ASSERT_EQ(uint64_t(0), testing::testresult::idle_started);
    ASSERT_EQ("", testing::testresult::idle_duration);
    ASSERT_EQ("", testing::testresult::idle_guid);

    toggl_set_idle_seconds(app.ctx(), 0);
    ASSERT_EQ("", testing::testresult::idle_since);
    ASSERT_EQ(uint64_t(0), testing::testresult::idle_started);
    ASSERT_EQ("", testing::testresult::idle_duration);
    ASSERT_EQ("", testing::testresult::idle_guid);

    ASSERT_TRUE(toggl_start(app.ctx(), "test", "", 0, 0));

    toggl_set_idle_seconds(app.ctx(), 5*60);
    ASSERT_EQ("", testing::testresult::idle_since);
    ASSERT_EQ(uint64_t(0), testing::testresult::idle_started);
    ASSERT_EQ("", testing::testresult::idle_duration);
    ASSERT_EQ("", testing::testresult::idle_guid);

    toggl_set_idle_seconds(app.ctx(), 0);
    ASSERT_EQ("", testing::testresult::idle_since);

    ASSERT_TRUE(toggl_set_time_entry_duration(app.ctx(),
                testing::testresult::timer_state.GUID().c_str(),
                "301 seconds"));

    toggl_set_idle_seconds(app.ctx(), 5*60);
    toggl_set_idle_seconds(app.ctx(), 0);
    ASSERT_NE("", testing::testresult::idle_since);

    ASSERT_NE(std::string::npos,
              testing::testresult::idle_since.find("You have been idle since"));
    ASSERT_NE(uint64_t(0), testing::testresult::idle_started);
    ASSERT_EQ("(5 minutes)", testing::testresult::idle_duration);
    ASSERT_NE("", testing::testresult::idle_guid);
}

TEST(TogglApiTest, toggl_open_in_browser) {
    testing::App app;
    toggl_open_in_browser(app.ctx());
}

TEST(TogglApiTest, toggl_get_support) {
    testing::App app;
    toggl_get_support(app.ctx());
    ASSERT_EQ("http://support.toggl.com/toggl-on-my-desktop/",
              testing::testresult::url);
}

TEST(TogglApiTest, toggl_login) {
    testing::App app;
    toggl_login(app.ctx(), "username", "password");
}

TEST(TogglApiTest, toggl_google_login) {
    testing::App app;
    toggl_google_login(app.ctx(), "token");
}

TEST(TogglApiTest, toggl_sync) {
    testing::App app;
    toggl_sync(app.ctx());
}

TEST(TogglApiTest, toggl_add_project) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    std::string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
    uint64_t wid = 0;
    uint64_t cid = 0;
    std::string project_name("");
    _Bool is_private = false;

    testing::testresult::error = "";
    _Bool res = toggl_add_project(app.ctx(),
                                  guid.c_str(),
                                  wid,
                                  cid,
                                  project_name.c_str(),
                                  is_private);
    ASSERT_EQ("Please select a workspace",
              testing::testresult::error);
    ASSERT_FALSE(res);

    wid = 123456789;
    res = toggl_add_project(app.ctx(),
                            guid.c_str(),
                            wid,
                            cid,
                            project_name.c_str(),
                            is_private);
    ASSERT_EQ("Project name must not be empty",
              testing::testresult::error);
    ASSERT_FALSE(res);

    project_name = "A new project";
    testing::testresult::error = "";
    res = toggl_add_project(app.ctx(),
                            guid.c_str(),
                            wid,
                            cid,
                            project_name.c_str(),
                            is_private);
    ASSERT_EQ("", testing::testresult::error);
    ASSERT_TRUE(res);

    bool found(false);
    for (std::size_t i = 0; i < testing::testresult::projects.size(); i++) {
        if (project_name == testing::testresult::projects[i]) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
}

TEST(TogglApiTest, toggl_create_project) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    uint64_t wid = 0;
    uint64_t cid = 0;
    std::string project_name("");
    _Bool is_private = false;

    testing::testresult::error = "";
    _Bool res = toggl_create_project(app.ctx(),
                                     wid,
                                     cid,
                                     project_name.c_str(),
                                     is_private);
    ASSERT_EQ("Please select a workspace",
              testing::testresult::error);
    ASSERT_FALSE(res);

    wid = 123456789;
    res = toggl_create_project(app.ctx(),
                               wid,
                               cid,
                               project_name.c_str(),
                               is_private);
    ASSERT_EQ("Project name must not be empty",
              testing::testresult::error);
    ASSERT_FALSE(res);

    project_name = "A new project";
    testing::testresult::error = "";
    res = toggl_create_project(app.ctx(),
                               wid,
                               cid,
                               project_name.c_str(),
                               is_private);
    ASSERT_EQ("", testing::testresult::error);
    ASSERT_TRUE(res);

    bool found(false);
    for (std::size_t i = 0; i < testing::testresult::projects.size(); i++) {
        if (project_name == testing::testresult::projects[i]) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
}

TEST(TogglApiTest, toggl_create_client) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    uint64_t wid = 0;
    std::string client_name("");

    testing::testresult::error = "";
    _Bool res = toggl_create_client(app.ctx(),
                                    wid,
                                    client_name.c_str());
    ASSERT_EQ("Please select a workspace", testing::testresult::error);
    ASSERT_FALSE(res);

    wid = 123456789;
    res = toggl_create_client(app.ctx(),
                              wid,
                              client_name.c_str());
    ASSERT_EQ("Client name must not be empty",
              testing::testresult::error);
    ASSERT_FALSE(res);

    client_name = "A new client";
    testing::testresult::error = "";
    res = toggl_create_client(app.ctx(),
                              wid,
                              client_name.c_str());
    ASSERT_EQ("", testing::testresult::error);
    ASSERT_TRUE(res);

    bool found(false);
    for (std::size_t i = 0; i < testing::testresult::clients.size(); i++) {
        if (client_name == testing::testresult::clients[i]) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
}

TEST(TogglApiTest, toggl_continue) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    std::string guid("6c97dc31-582e-7662-1d6f-5e9d623b1685");

    testing::testresult::error = "";
    ASSERT_TRUE(toggl_continue(app.ctx(), guid.c_str()));
    ASSERT_NE(guid, testing::testresult::timer_state.GUID());
    ASSERT_EQ("More work", testing::testresult::timer_state.Description());
}

TEST(TogglApiTest, toggl_check_view_struct_size) {
    toggl_check_view_struct_size(
        sizeof(TogglTimeEntryView),
        sizeof(TogglAutocompleteView),
        sizeof(TogglGenericView),
        sizeof(TogglSettingsView));
}

TEST(TogglApiTest, toggl_view_time_entry_list) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    toggl_view_time_entry_list(app.ctx());
    ASSERT_EQ(std::size_t(5), testing::testresult::time_entries.size());
}

TEST(TogglApiTest, toggl_edit) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    std::string guid("6a958efd-0e9a-d777-7e19-001b2d7ced92");
    _Bool edit_running_time_entry = false;
    std::string focused_field("description");
    toggl_edit(app.ctx(), guid.c_str(), edit_running_time_entry,
               focused_field.c_str());
    ASSERT_EQ(guid, testing::testresult::editor_state.GUID());
}

TEST(TogglApiTest, toggl_set_online) {
    // App can trigger online state before lib is initialized
    toggl_set_online(0);

    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    toggl_set_online(app.ctx());
}

TEST(TogglApiTest, toggl_set_sleep) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    toggl_set_sleep(app.ctx());
}

TEST(TogglApiTest, toggl_set_wake) {
    // App can trigger awake state from computer before lib is initialized
    toggl_set_wake(0);

    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    toggl_set_wake(app.ctx());
}

TEST(TogglApiTest, toggl_timeline_toggle_recording) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    ASSERT_TRUE(toggl_timeline_is_recording_enabled(app.ctx()));

    ASSERT_TRUE(toggl_timeline_toggle_recording(app.ctx(), true));
    ASSERT_TRUE(toggl_timeline_is_recording_enabled(app.ctx()));

    ASSERT_TRUE(toggl_timeline_toggle_recording(app.ctx(), false));
    ASSERT_FALSE(toggl_timeline_is_recording_enabled(app.ctx()));
}

TEST(TogglApiTest, toggl_edit_preferences) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::open_settings = false;
    toggl_edit_preferences(app.ctx());
    ASSERT_TRUE(testing::testresult::open_settings);
}

TEST(TogglApiTest, toggl_continue_latest) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::error = noError;
    bool res = toggl_continue_latest(app.ctx());
    ASSERT_EQ(noError, testing::testresult::error);
    ASSERT_TRUE(res);
    ASSERT_EQ("arendus käib", testing::testresult::timer_state.Description());
}

TEST(TogglApiTest, toggl_delete_time_entry) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    toggl_view_time_entry_list(app.ctx());
    ASSERT_EQ(std::size_t(5), testing::testresult::time_entries.size());

    std::string guid("6a958efd-0e9a-d777-7e19-001b2d7ced92");
    ASSERT_TRUE(toggl_delete_time_entry(app.ctx(), guid.c_str()));

    toggl_view_time_entry_list(app.ctx());
    ASSERT_EQ(std::size_t(4), testing::testresult::time_entries.size());
}

TEST(TogglApiTest, toggl_set_time_entry_duration) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    std::string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";

    ASSERT_TRUE(toggl_set_time_entry_duration(app.ctx(),
                guid.c_str(), "2 hours"));

    toggl_view_time_entry_list(app.ctx());
    TimeEntry te;
    for (std::size_t i = 0; i < testing::testresult::time_entries.size();
            i++) {
        if (testing::testresult::time_entries[i].GUID() == guid) {
            te = testing::testresult::time_entries[i];
            break;
        }
    }
    ASSERT_EQ(guid, te.GUID());
    ASSERT_EQ(7200, te.DurationInSeconds());
}

TEST(TogglApiTest, toggl_set_time_entry_description) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    std::string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";

    ASSERT_TRUE(toggl_set_time_entry_description(app.ctx(),
                guid.c_str(), "this is a nuclear test"));

    toggl_view_time_entry_list(app.ctx());
    TimeEntry te;
    for (std::size_t i = 0; i < testing::testresult::time_entries.size();
            i++) {
        if (testing::testresult::time_entries[i].GUID() == guid) {
            te = testing::testresult::time_entries[i];
            break;
        }
    }
    ASSERT_EQ(guid, te.GUID());
    ASSERT_EQ("this is a nuclear test", te.Description());
}

TEST(TogglApiTest, toggl_stop) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::timer_state = TimeEntry();

    ASSERT_TRUE(toggl_start(app.ctx(), "test", "", 0, 0));
    ASSERT_FALSE(testing::testresult::timer_state.GUID().empty());

    ASSERT_TRUE(toggl_stop(app.ctx()));
    ASSERT_TRUE(testing::testresult::timer_state.GUID().empty());
}

TEST(TogglApiTest, toggl_start) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::timer_state = TimeEntry();

    ASSERT_TRUE(toggl_start(app.ctx(), "test", "", 0, 0));
    ASSERT_FALSE(testing::testresult::timer_state.GUID().empty());
}

TEST(TogglApiTest, toggl_set_time_entry_billable) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::timer_state = TimeEntry();

    ASSERT_TRUE(toggl_start(app.ctx(), "test", "", 0, 0));

    std::string guid = testing::testresult::timer_state.GUID();
    ASSERT_FALSE(guid.empty());

    ASSERT_TRUE(toggl_set_time_entry_billable(app.ctx(), guid.c_str(), true));
    ASSERT_TRUE(testing::testresult::timer_state.Billable());

    ASSERT_TRUE(toggl_set_time_entry_billable(app.ctx(), guid.c_str(), false));
    ASSERT_FALSE(testing::testresult::timer_state.Billable());
}

TEST(TogglApiTest, toggl_set_time_entry_tags) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::timer_state = TimeEntry();

    ASSERT_TRUE(toggl_start(app.ctx(), "test", "", 0, 0));

    std::string guid = testing::testresult::timer_state.GUID();
    ASSERT_FALSE(guid.empty());

    ASSERT_TRUE(toggl_set_time_entry_tags(app.ctx(), guid.c_str(), "a|b|c"));
    ASSERT_EQ("a|b|c", testing::testresult::timer_state.Tags());

    ASSERT_TRUE(toggl_set_time_entry_tags(app.ctx(), guid.c_str(), "a"));
    ASSERT_EQ("a", testing::testresult::timer_state.Tags());

    ASSERT_TRUE(toggl_set_time_entry_tags(app.ctx(), guid.c_str(), ""));
    ASSERT_EQ("", testing::testresult::timer_state.Tags());
}

TEST(TogglApiTest, toggl_parse_duration_string_into_seconds) {
    int64_t seconds = toggl_parse_duration_string_into_seconds("15 seconds");
    ASSERT_EQ(15, seconds);
}

TEST(TogglApiTest,
     toggl_parse_duration_string_into_seconds_with_no_duration_string) {
    int64_t seconds = toggl_parse_duration_string_into_seconds(0);
    ASSERT_EQ(0, seconds);
}

TEST(TogglApiTest, toggl_discard_time_at_with_no_guid) {
    testing::App app;

    ASSERT_FALSE(toggl_discard_time_at(app.ctx(), 0, time(0), false));
}

TEST(TogglApiTest, toggl_discard_time_at_with_no_stop_time) {
    testing::App app;

    ASSERT_FALSE(toggl_discard_time_at(app.ctx(), "some fake guid", 0, false));
}

TEST(TogglApiTest, toggl_discard_time_at) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::timer_state = TimeEntry();

    // Start a time entry

    ASSERT_TRUE(toggl_start(app.ctx(), "test", "", 0, 0));

    Poco::UInt64 started = time(0);
    std::string guid = testing::testresult::timer_state.GUID();
    ASSERT_FALSE(guid.empty());

    // Discard the time entry at some point

    Poco::UInt64 stopped = time(0);
    ASSERT_TRUE(toggl_discard_time_at(app.ctx(), guid.c_str(), stopped, false));
    ASSERT_NE(guid, testing::testresult::timer_state.GUID());
    ASSERT_TRUE(testing::testresult::timer_state.GUID().empty());

    TimeEntry te;
    for (std::size_t i = 0; i < testing::testresult::time_entries.size();
            i++) {
        if (testing::testresult::time_entries[i].GUID() == guid) {
            te = testing::testresult::time_entries[i];
            break;
        }
    }
    ASSERT_EQ(guid, te.GUID());
    ASSERT_TRUE(started == te.Start() || started + 1 == te.Start());
    ASSERT_TRUE(stopped == te.Stop() || stopped + 1 == te.Stop());

    // Start another time entry

    ASSERT_TRUE(toggl_start(app.ctx(), "test 2", "", 0, 0));

    started = time(0);
    guid = testing::testresult::timer_state.GUID();
    ASSERT_FALSE(guid.empty());

    // Discard the time entry, by creating a new one

    stopped = time(0);
    ASSERT_TRUE(toggl_discard_time_at(app.ctx(), guid.c_str(), stopped, true));
    ASSERT_NE(guid, testing::testresult::timer_state.GUID());

    te = TimeEntry();
    for (std::size_t i = 0; i < testing::testresult::time_entries.size();
            i++) {
        if (testing::testresult::time_entries[i].GUID() == guid) {
            te = testing::testresult::time_entries[i];
            break;
        }
    }
    ASSERT_EQ(guid, te.GUID());
    ASSERT_TRUE(started == te.Start() || started + 1 == te.Start());
    ASSERT_TRUE(stopped == te.Stop() || stopped + 1 == te.Stop());

    // Check that a new time entry was created

    ASSERT_TRUE(!testing::testresult::timer_state.GUID().empty());
    ASSERT_EQ(stopped, testing::testresult::timer_state.Start());
    ASSERT_TRUE(testing::testresult::timer_state.IsTracking());
    ASSERT_EQ("", testing::testresult::timer_state.Description());
}

TEST(TogglApiTest, toggl_feedback_send) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    ASSERT_TRUE(toggl_feedback_send(app.ctx(),
                                    "Help", "I need help", ""));
}

TEST(TogglApiTest, toggl_set_time_entry_date) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    std::string guid("07fba193-91c4-0ec8-2894-820df0548a8f");

    toggl::TimeEntry te = testing::testresult::time_entry_by_guid(guid);
    Poco::DateTime datetime(Poco::Timestamp::fromEpochTime(te.Start()));
    ASSERT_EQ(2013, datetime.year());
    ASSERT_EQ(9, datetime.month());
    ASSERT_EQ(5, datetime.day());
    ASSERT_EQ(6, datetime.hour());
    ASSERT_EQ(33, datetime.minute());
    ASSERT_EQ(50, datetime.second());

    // 10/27/2014 @ 12:51pm in UTC.
    int unix_timestamp(1414414311);
    ASSERT_TRUE(toggl_set_time_entry_date(app.ctx(),
                                          guid.c_str(),
                                          unix_timestamp));

    te = testing::testresult::time_entry_by_guid(guid);
    datetime = Poco::DateTime(Poco::Timestamp::fromEpochTime(te.Start()));
    ASSERT_EQ(2014, datetime.year());
    ASSERT_EQ(10, datetime.month());
    ASSERT_EQ(27, datetime.day());
    ASSERT_EQ(33, datetime.minute());
    ASSERT_EQ(50, datetime.second());
}

TEST(TogglApiTest, toggl_set_time_entry_start) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    std::string guid("07fba193-91c4-0ec8-2894-820df0548a8f");

    toggl::TimeEntry te = testing::testresult::time_entry_by_guid(guid);
    Poco::DateTime datetime(Poco::Timestamp::fromEpochTime(te.Start()));
    ASSERT_EQ(2013, datetime.year());
    ASSERT_EQ(9, datetime.month());
    ASSERT_EQ(5, datetime.day());
    ASSERT_EQ(6, datetime.hour());
    ASSERT_EQ(33, datetime.minute());
    ASSERT_EQ(50, datetime.second());

    ASSERT_TRUE(toggl_set_time_entry_start(app.ctx(), guid.c_str(), "12:34"));

    te = testing::testresult::time_entry_by_guid(guid);
    Poco::LocalDateTime local =
        Poco::DateTime(Poco::Timestamp::fromEpochTime(te.Start()));
    ASSERT_EQ(2013, local.year());
    ASSERT_EQ(9, local.month());
    ASSERT_EQ(5, local.day());
    ASSERT_EQ(12, local.hour());
    ASSERT_EQ(34, local.minute());
    ASSERT_EQ(50, local.second());

    // Setting an invalid value should not crash the app
    ASSERT_FALSE(toggl_set_time_entry_start(app.ctx(), guid.c_str(), "12:558"));
}

TEST(TogglApiTest, toggl_set_time_entry_end) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    std::string guid("07fba193-91c4-0ec8-2894-820df0548a8f");

    toggl::TimeEntry te = testing::testresult::time_entry_by_guid(guid);
    Poco::DateTime datetime(Poco::Timestamp::fromEpochTime(te.Stop()));
    ASSERT_EQ(2013, datetime.year());
    ASSERT_EQ(9, datetime.month());
    ASSERT_EQ(5, datetime.day());
    ASSERT_EQ(8, datetime.hour());
    ASSERT_EQ(19, datetime.minute());
    ASSERT_EQ(46, datetime.second());

    ASSERT_TRUE(toggl_set_time_entry_end(app.ctx(), guid.c_str(), "18:29"));

    te = testing::testresult::time_entry_by_guid(guid);
    Poco::LocalDateTime local =
        Poco::DateTime(Poco::Timestamp::fromEpochTime(te.Stop()));
    ASSERT_EQ(2013, local.year());
    ASSERT_EQ(9, local.month());
    ASSERT_EQ(5, local.day());
    ASSERT_EQ(18, local.hour());
    ASSERT_EQ(29, local.minute());
    ASSERT_EQ(46, local.second());

    // Setting an invalid value should not crash the app
    ASSERT_FALSE(toggl_set_time_entry_end(app.ctx(), guid.c_str(), "12:558"));
}

TEST(TogglApiTest, toggl_set_time_entry_end_prefers_same_day) {
    testing::App app;
    std::string json =
        loadTestDataFile("../testdata/time_entry_ending_tomorrow.json");
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    const std::string guid("07fba193-91c4-0ec8-2894-820df0548a8f");

    // Set start time so it will be local time
    ASSERT_TRUE(toggl_set_time_entry_date(app.ctx(), guid.c_str(), time(0)));
    ASSERT_TRUE(toggl_set_time_entry_start(app.ctx(), guid.c_str(), "06:33"));

    ASSERT_TRUE(toggl_set_time_entry_end(app.ctx(), guid.c_str(), "06:34"));

    toggl::TimeEntry te = testing::testresult::time_entry_by_guid(guid);

    Poco::DateTime start(Poco::Timestamp::fromEpochTime(te.Start()));
    Poco::DateTime end(Poco::Timestamp::fromEpochTime(te.Stop()));
    ASSERT_EQ(start.year(), end.year());
    ASSERT_EQ(start.month(), end.month());
    ASSERT_EQ(start.day(), end.day());
}

TEST(ProxyTest, IsConfigured) {
    Proxy p;
    ASSERT_FALSE(p.IsConfigured());

    p.SetHost("localhost");
    p.SetPort(123);
    ASSERT_TRUE(p.IsConfigured());
}

TEST(ProxyTest, HasCredentials) {
    Proxy p;
    ASSERT_FALSE(p.HasCredentials());

    p.SetUsername("foo");
    p.SetPassword("bar");
    ASSERT_TRUE(p.HasCredentials());
}

TEST(ProxyTest, String) {
    Proxy p;
    ASSERT_NE("", p.String());
}

}  // namespace toggl
