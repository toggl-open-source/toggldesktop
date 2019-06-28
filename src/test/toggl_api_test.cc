// Copyright 2014 Toggl Desktop developers.

#include <vector>

#include "gtest/gtest.h"

#include "./../https_client.h"
#include "./../obm_action.h"
#include "./../proxy.h"
#include "./../settings.h"
#include "./../time_entry.h"
#include "./../toggl_api.h"
#include "./../toggl_api_private.h"
#include "./test_data.h"

#include <iostream>   // NOLINT

#include "Poco/DateTime.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/LocalDateTime.h"
#include "Poco/Path.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"

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
bool_t open_settings(false);
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

// on_project_colors
std::vector<std::string> project_colors;

// on_obm_experiment
std::vector<ObmExperiment> obm_experiments;

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
bool_t editor_open(false);
std::string editor_focused_field_name("");

// toggl_on_help_articles
std::vector<std::string> help_article_names;

bool on_app_open;

int64_t sync_state;

int64_t unsynced_item_count;

std::string update_url;

}  // namespace testresult

void on_app(const bool_t open) {
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
    const bool_t user_error) {
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

void on_login(const bool_t open, const uint64_t user_id) {
    testresult::user_id = user_id;
}

void on_reminder(const char *title, const char *informative_text) {
    testresult::reminder_title = std::string(title);
    testresult::reminder_informative_text = std::string(informative_text);
}

void on_help_articles(TogglHelpArticleView *first) {
    testing::testresult::help_article_names.clear();
    TogglHelpArticleView *it = first;
    while (it) {
        std::string name(it->Name);
        testing::testresult::help_article_names.push_back(name);
        it = reinterpret_cast<TogglHelpArticleView *>(it->Next);
    }
}

void on_time_entry_list(
    const bool_t open,
    TogglTimeEntryView *first,
    const bool_t show_load_more) {
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
    const bool_t open,
    TogglTimeEntryView *te,
    const char *focused_field_name) {
    testing::testresult::editor_state = TimeEntry();
    testing::testresult::editor_state.SetGUID(te->GUID);
    testing::testresult::editor_open = open;
    testing::testresult::editor_focused_field_name =
        std::string(focused_field_name);
}

void on_display_settings(
    const bool_t open,
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
    testing::testresult::settings.autotrack = settings->Autotrack;

    testing::testresult::use_proxy = settings->UseProxy;

    testing::testresult::proxy.SetHost(std::string(settings->ProxyHost));
    testing::testresult::proxy.SetPort(settings->ProxyPort);
    testing::testresult::proxy.SetUsername(
        std::string(settings->ProxyUsername));
    testing::testresult::proxy.SetPassword(
        std::string(settings->ProxyPassword));

    testing::testresult::settings.remind_starts = settings->RemindStarts;
    testing::testresult::settings.remind_ends = settings->RemindEnds;
    testing::testresult::settings.remind_mon = settings->RemindMon;
    testing::testresult::settings.remind_tue = settings->RemindTue;
    testing::testresult::settings.remind_wed = settings->RemindWed;
    testing::testresult::settings.remind_thu = settings->RemindThu;
    testing::testresult::settings.remind_fri = settings->RemindFri;
    testing::testresult::settings.remind_sat = settings->RemindSat;
    testing::testresult::settings.remind_sun = settings->RemindSun;
}

void on_project_colors(
    string_list_t color_list,
    const uint64_t color_count) {
    testresult::project_colors.clear();
    for (uint64_t i = 0; i < color_count; i++) {
        testresult::project_colors.push_back(std::string(color_list[i]));
    }
}

void on_obm_experiment(
    const uint64_t nr,
    const bool_t included,
    const bool_t seen) {
    ObmExperiment experiment;
    experiment.SetNr(nr);
    experiment.SetIncluded(included);
    experiment.SetHasSeen(seen);
    testresult::obm_experiments.push_back(experiment);
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
        testing::testresult::timer_state.SetPID(te->PID);
        testing::testresult::timer_state.SetTID(te->TID);
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
        toggl_on_project_colors(ctx_, on_project_colors);
        toggl_on_help_articles(ctx_, on_help_articles);
        toggl_on_obm_experiment(ctx_, on_obm_experiment);

        poco_assert(!toggl_ui_start(ctx_));
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

// For testing API concurrently
class ApiClient : public Poco::Runnable {
 public:
    explicit ApiClient(testing::App *app, const std::string name)
        : app_(app)
    , name_(name)
    , finished_(false) {}

    void run() {
        std::cout << "runnable " << name_ << " running" << std::endl;

        for (int i = 0; i < 100; i++) {
            char_t *guid = toggl_start(app_->ctx(), "test", "", 0, 0, 0, 0,
                                       false);
            ASSERT_TRUE(guid);

            ASSERT_TRUE(toggl_stop(app_->ctx(), false));

            toggl_edit(app_->ctx(), guid, true, "");

            ASSERT_TRUE(toggl_delete_time_entry(app_->ctx(), guid));

            free(guid);
        }
        finished_ = true;
    }

    bool finished() const {
        return finished_;
    }

 private:
    testing::App *app_;
    std::string name_;
    bool finished_;
};

}  // namespace testing

TEST(toggl_api, toggl_context_init) {
    testing::App app;
}

TEST(toggl_api, testing_sleep) {
    time_t start = time(0);
    testing_sleep(1);
    int elapsed_seconds = time(0) - start;
    ASSERT_GE(elapsed_seconds, 1);
    ASSERT_LT(elapsed_seconds, 2);
}

TEST(toggl_api, toggl_run_script) {
    testing::App app;
    int64_t err(0);
    char *s = toggl_run_script(app.ctx(), "print 'test'", &err);
    std::string res(s);
    free(s);
    ASSERT_EQ(0, err);
    ASSERT_EQ("0 value(s) returned\n\n\n", res);
}

TEST(toggl_api, toggl_run_script_with_invalid_script) {
    testing::App app;
    int64_t err(0);
    char *s = toggl_run_script(app.ctx(), "foo bar", &err);
    std::string res(s);
    free(s);
    ASSERT_NE(0, err);
    ASSERT_EQ("[string \"foo bar\"]:1: syntax error near 'bar'", res);
}

TEST(toggl_api, toggl_add_obm_experiment_nr) {
    testing::App app;

    toggl_add_obm_experiment_nr(123);
    ASSERT_EQ("tests/0.1-obm-123", toggl::HTTPSClient::Config.UserAgent());

    toggl_add_obm_experiment_nr(0);
    ASSERT_EQ("tests/0.1", toggl::HTTPSClient::Config.UserAgent());
}

TEST(toggl_api, toggl_set_settings) {
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

    testing::testresult::error = noError;
    ASSERT_TRUE(toggl_set_settings_autotrack(app.ctx(), true));
    ASSERT_EQ(noError, testing::testresult::error);
    ASSERT_TRUE(testing::testresult::settings.autotrack);
}

TEST(toggl_api, toggl_set_proxy_settings) {
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

TEST(toggl_api, toggl_set_settings_remind_days) {
    testing::App app;

    testing::testresult::error = noError;
    bool_t res = toggl_set_settings_remind_days(
        app.ctx(), true, true, true, true, true, true, true);
    ASSERT_EQ(noError, testing::testresult::error);
    ASSERT_TRUE(res);

    ASSERT_TRUE(testing::testresult::settings.remind_mon);
    ASSERT_TRUE(testing::testresult::settings.remind_tue);
    ASSERT_TRUE(testing::testresult::settings.remind_wed);
    ASSERT_TRUE(testing::testresult::settings.remind_thu);
    ASSERT_TRUE(testing::testresult::settings.remind_fri);
    ASSERT_TRUE(testing::testresult::settings.remind_sat);
    ASSERT_TRUE(testing::testresult::settings.remind_sun);

    ASSERT_TRUE(toggl_set_settings_remind_days(
        app.ctx(), false, false, false, false, false, false, false));

    ASSERT_FALSE(testing::testresult::settings.remind_mon);
    ASSERT_FALSE(testing::testresult::settings.remind_tue);
    ASSERT_FALSE(testing::testresult::settings.remind_wed);
    ASSERT_FALSE(testing::testresult::settings.remind_thu);
    ASSERT_FALSE(testing::testresult::settings.remind_fri);
    ASSERT_FALSE(testing::testresult::settings.remind_sat);
    ASSERT_FALSE(testing::testresult::settings.remind_sun);
}

TEST(toggl_api, toggl_set_settings_remind_times) {
    testing::App app;

    ASSERT_TRUE(toggl_set_settings_remind_times(app.ctx(), "", ""));

    ASSERT_EQ(std::string(""), testing::testresult::settings.remind_starts);
    ASSERT_EQ(std::string(""), testing::testresult::settings.remind_ends);

    ASSERT_TRUE(toggl_set_settings_remind_times(app.ctx(), "09:30", "17:30"));

    ASSERT_EQ(std::string("09:30"),
              testing::testresult::settings.remind_starts);
    ASSERT_EQ(std::string("17:30"),
              testing::testresult::settings.remind_ends);
}

TEST(toggl_api, toggl_set_window_settings) {
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

TEST(toggl_api, toggl_get_user_fullname) {
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

TEST(toggl_api, toggl_get_user_email) {
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

TEST(toggl_api, toggl_show_app) {
    testing::App app;

    testing::testresult::on_app_open = false;

    toggl_show_app(app.ctx());

    ASSERT_TRUE(testing::testresult::on_app_open);
}

TEST(toggl_api, toggl_set_update_channel) {
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

TEST(toggl_api, toggl_set_log_level) {
    toggl_set_log_level("trace");
}

TEST(toggl_api, toggl_format_tracking_time_duration) {
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

TEST(toggl_api, toggl_format_tracked_time_duration) {
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

TEST(toggl_api, toggl_password_forgot) {
    testing::App app;
    toggl_password_forgot(app.ctx());
    ASSERT_EQ("https://toggl.com/forgot-password?desktop=true",
              testing::testresult::url);
}

TEST(toggl_api, toggl_set_environment) {
    testing::App app;

    toggl_set_environment(app.ctx(), "test");

    char *env = toggl_environment(app.ctx());
    std::string res(env);
    free(env);
    ASSERT_EQ("test", res);
}

TEST(toggl_api, toggl_set_update_path) {
    testing::App app;

    toggl_set_update_path(app.ctx(), "/tmp/");

    char *s = toggl_update_path(app.ctx());
    std::string path(s);
    free(s);

    ASSERT_EQ("/tmp/", path);
}

TEST(toggl_api, testing_set_logged_in_user) {
    std::string json = loadTestData();
    testing::App app;
    testing::testresult::error = "";
    bool_t res = testing_set_logged_in_user(app.ctx(), json.c_str());
    ASSERT_EQ(noError, testing::testresult::error);
    ASSERT_TRUE(res);
    ASSERT_EQ(uint64_t(10471231), testing::testresult::user_id);
}

TEST(toggl_api, toggl_disable_update_check) {
    testing::App app;
    toggl_disable_update_check(app.ctx());
}

TEST(toggl_api, toggl_logout) {
    std::string json = loadTestData();
    testing::App app;
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    ASSERT_EQ(uint64_t(10471231), testing::testresult::user_id);
    ASSERT_TRUE(toggl_logout(app.ctx()));
    ASSERT_EQ(uint64_t(0), testing::testresult::user_id);
}

TEST(toggl_api, toggl_clear_cache) {
    std::string json = loadTestData();
    testing::App app;
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    ASSERT_EQ(uint64_t(10471231), testing::testresult::user_id);
    ASSERT_TRUE(toggl_clear_cache(app.ctx()));
    ASSERT_EQ(uint64_t(0), testing::testresult::user_id);
}

TEST(toggl_api, toggl_debug) {
    toggl_set_log_path("test.log");
    toggl_debug("Test 123");
}

TEST(toggl_api, toggl_set_idle_seconds) {
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

    char_t *guid = toggl_start(app.ctx(), "test", "", 0, 0, 0, 0, false);
    ASSERT_TRUE(guid);
    free(guid);

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

TEST(toggl_api, toggl_open_in_browser) {
    testing::App app;
    toggl_open_in_browser(app.ctx());
}

TEST(toggl_api, toggl_get_support) {
    testing::App app;
    toggl_get_support(app.ctx(), 0);
    ASSERT_EQ("http://support.toggl.com/toggl-on-my-desktop/",
              testing::testresult::url);
}

TEST(toggl_api, toggl_login) {
    testing::App app;
    toggl_login(app.ctx(), "username", "password");
}

TEST(toggl_api, toggl_google_login) {
    testing::App app;
    toggl_google_login(app.ctx(), "token");
}

TEST(toggl_api, toggl_sync) {
    testing::App app;
    toggl_sync(app.ctx());
}

TEST(toggl_api, toggl_add_obm_action) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    ASSERT_FALSE(toggl_add_obm_action(app.ctx(), 0, "key", "value"));
    ASSERT_FALSE(toggl_add_obm_action(app.ctx(), 1, "key", " "));
    ASSERT_FALSE(toggl_add_obm_action(app.ctx(), 2, " ", ""));
    ASSERT_TRUE(toggl_add_obm_action(app.ctx(), 3, "key", "value"));
    ASSERT_TRUE(toggl_add_obm_action(app.ctx(), 3, "key", "value"));
}

TEST(toggl_api, toggl_add_project) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    std::string guid = "07fba193-91c4-0ec8-2894-820df0548a8f";
    uint64_t wid = 0;
    uint64_t cid = 0;
    std::string project_name("");
    bool_t is_private = false;

    testing::testresult::error = "";
    char_t *res = toggl_add_project(app.ctx(),
                                    guid.c_str(),
                                    wid,
                                    cid,
                                    "",
                                    project_name.c_str(),
                                    is_private,
                                    "");
    ASSERT_EQ("Please select a workspace",
              testing::testresult::error);
    ASSERT_FALSE(res);

    wid = 123456789;
    res = toggl_add_project(app.ctx(),
                            guid.c_str(),
                            wid,
                            cid,
                            "",
                            project_name.c_str(),
                            is_private,
                            "#ffffff");
    ASSERT_EQ("Project name must not be empty",
              testing::testresult::error);
    ASSERT_FALSE(res);
    free(res);

    project_name = "A new project";
    testing::testresult::error = "";
    res = toggl_add_project(app.ctx(),
                            guid.c_str(),
                            wid,
                            cid,
                            "",
                            project_name.c_str(),
                            is_private,
                            0);
    ASSERT_EQ("", testing::testresult::error);
    ASSERT_TRUE(res);
    free(res);

    bool found(false);
    for (std::size_t i = 0; i < testing::testresult::projects.size(); i++) {
        if (project_name == testing::testresult::projects[i]) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
}


TEST(toggl_api, toggl_create_client) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    uint64_t wid = 0;
    std::string client_name("        ");

    testing::testresult::error = "";
    char_t *res = toggl_create_client(app.ctx(),
                                      wid,
                                      client_name.c_str());
    ASSERT_EQ("Please select a workspace", testing::testresult::error);
    ASSERT_FALSE(res);
    free(res);

    wid = 123456789;
    res = toggl_create_client(app.ctx(),
                              wid,
                              client_name.c_str());
    ASSERT_EQ("Client name must not be empty",
              testing::testresult::error);
    ASSERT_FALSE(res);
    free(res);

    client_name = "A new client";
    testing::testresult::error = "";
    res = toggl_create_client(app.ctx(),
                              wid,
                              client_name.c_str());
    ASSERT_EQ("", testing::testresult::error);
    ASSERT_TRUE(res);
    free(res);

    bool found(false);
    for (std::size_t i = 0; i < testing::testresult::clients.size(); i++) {
        if (client_name == testing::testresult::clients[i]) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);

    // We should be able to add as many clients as we like!
    for (int i = 0; i < 10; i++) {
        std::stringstream ss;
        ss << "extra client " << i;
        testing::testresult::error = "";
        res = toggl_create_client(app.ctx(),
                                  wid,
                                  ss.str().c_str());
        ASSERT_EQ("", testing::testresult::error);
        ASSERT_TRUE(res);
        free(res);
    }

    // But none with an existing client name
    testing::testresult::error = "";
    res = toggl_create_client(app.ctx(),
                              wid,
                              client_name.c_str());
    ASSERT_FALSE(res);
    ASSERT_EQ("Client name already exists", testing::testresult::error);
}

TEST(toggl_api, toggl_continue) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    std::string guid("6c97dc31-582e-7662-1d6f-5e9d623b1685");

    testing::testresult::error = "";
    ASSERT_TRUE(toggl_continue(app.ctx(), guid.c_str()));
    ASSERT_NE(guid, testing::testresult::timer_state.GUID());
    ASSERT_EQ("More work", testing::testresult::timer_state.Description());
}

TEST(toggl_api, toggl_continue_in_manual_mode) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    ASSERT_TRUE(toggl_set_settings_manual_mode(app.ctx(), true));
    ASSERT_TRUE(testing::testresult::settings.manual_mode);

    std::string guid("6c97dc31-582e-7662-1d6f-5e9d623b1685");

    testing::testresult::error = "";
    testing::testresult::editor_state = TimeEntry();
    testing::testresult::timer_state = TimeEntry();

    ASSERT_TRUE(toggl_continue(app.ctx(), guid.c_str()));

    ASSERT_NE(guid, testing::testresult::timer_state.GUID());

    ASSERT_FALSE(testing::testresult::timer_state.Start());
    ASSERT_FALSE(testing::testresult::timer_state.DurationInSeconds());

    ASSERT_NE("", testing::testresult::editor_state.GUID());
}

TEST(toggl_api, toggl_check_view_struct_size) {
    char_t *err = toggl_check_view_struct_size(
        sizeof(TogglTimeEntryView),
        sizeof(TogglAutocompleteView),
        sizeof(TogglGenericView),
        sizeof(TogglSettingsView),
        sizeof(TogglAutotrackerRuleView));
    if (err) {
        ASSERT_EQ("", std::string(err));
    }
    ASSERT_FALSE(err);
    free(err);
}

TEST(toggl_api, toggl_view_time_entry_list) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    toggl_view_time_entry_list(app.ctx());
    ASSERT_EQ(std::size_t(5), testing::testresult::time_entries.size());
}

TEST(toggl_api, toggl_edit) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    std::string guid("6a958efd-0e9a-d777-7e19-001b2d7ced92");
    bool_t edit_running_time_entry = false;
    std::string focused_field("description");
    toggl_edit(app.ctx(), guid.c_str(), edit_running_time_entry,
               focused_field.c_str());
    ASSERT_EQ(guid, testing::testresult::editor_state.GUID());
    ASSERT_EQ("description", testing::testresult::editor_focused_field_name);
}

TEST(toggl_api, toggl_set_online) {
    // App can trigger online state before lib is initialized
    toggl_set_online(0);

    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    toggl_set_online(app.ctx());
}

TEST(toggl_api, toggl_set_sleep) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    toggl_set_sleep(app.ctx());
}

TEST(toggl_api, toggl_set_wake) {
    // App can trigger awake state from computer before lib is initialized
    toggl_set_wake(0);

    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    toggl_set_wake(app.ctx());
}

TEST(toggl_api, toggl_timeline_toggle_recording) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    ASSERT_TRUE(toggl_timeline_is_recording_enabled(app.ctx()));

    ASSERT_TRUE(toggl_timeline_toggle_recording(app.ctx(), true));
    ASSERT_TRUE(toggl_timeline_is_recording_enabled(app.ctx()));

    ASSERT_TRUE(toggl_timeline_toggle_recording(app.ctx(), false));
    ASSERT_FALSE(toggl_timeline_is_recording_enabled(app.ctx()));
}

TEST(toggl_api, toggl_edit_preferences) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::open_settings = false;
    toggl_edit_preferences(app.ctx());
    ASSERT_TRUE(testing::testresult::open_settings);
}

TEST(toggl_api, toggl_continue_latest) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::error = noError;
    bool res = toggl_continue_latest(app.ctx(), false);
    ASSERT_EQ(noError, testing::testresult::error);
    ASSERT_TRUE(res);
    ASSERT_EQ("arendus käib", testing::testresult::timer_state.Description());
}

TEST(toggl_api, toggl_continue_latest_with_manual_mode) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    ASSERT_TRUE(toggl_set_settings_manual_mode(app.ctx(), true));
    ASSERT_TRUE(testing::testresult::settings.manual_mode);

    testing::testresult::error = noError;
    testing::testresult::editor_state = TimeEntry();
    testing::testresult::timer_state = TimeEntry();

    ASSERT_TRUE(toggl_continue_latest(app.ctx(), false));

    ASSERT_EQ(noError, testing::testresult::error);

    ASSERT_FALSE(testing::testresult::timer_state.Start());
    ASSERT_FALSE(testing::testresult::timer_state.DurationInSeconds());

    ASSERT_NE("", testing::testresult::editor_state.GUID());
}

TEST(toggl_api, toggl_delete_time_entry) {
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

TEST(toggl_api, toggl_set_time_entry_duration) {
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

TEST(toggl_api, toggl_set_time_entry_description) {
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

TEST(toggl_api, toggl_stop) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::timer_state = TimeEntry();

    char_t *guid = toggl_start(app.ctx(), "test", "", 0, 0, 0, 0, false);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_FALSE(testing::testresult::timer_state.GUID().empty());

    ASSERT_TRUE(toggl_stop(app.ctx(), false));
    ASSERT_TRUE(testing::testresult::timer_state.GUID().empty());
}

TEST(toggl_api, toggl_with_default_project) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    // Clear default project ID
    // Check that project ID and task ID are cleared

    ASSERT_TRUE(toggl_set_default_project(app.ctx(), 0, 0));
    char_t *s = toggl_get_default_project_name(app.ctx());
    ASSERT_FALSE(s);
    free(s);

    // Start a time entry, no defaults should apply

    testing::testresult::timer_state = TimeEntry();

    char_t *guid = toggl_start(app.ctx(), "test", "", 0, 0, 0, 0, false);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_FALSE(testing::testresult::timer_state.GUID().empty());
    ASSERT_FALSE(testing::testresult::timer_state.PID());
    ASSERT_FALSE(testing::testresult::timer_state.TID());

    // Set default project ID.
    // Task ID should remain 0

    const uint64_t existing_project_id = 2598305;
    const std::string existing_project_name = "Testing stuff";
    ASSERT_TRUE(toggl_set_default_project(app.ctx(), existing_project_id, 0));
    s = toggl_get_default_project_name(app.ctx());
    ASSERT_TRUE(s);
    ASSERT_EQ(existing_project_name, std::string(s));
    free(s);

    // Start timer, the default project should apply

    testing::testresult::timer_state = TimeEntry();

    guid = toggl_start(app.ctx(), "test", "", 0, 0, 0, 0, false);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_FALSE(testing::testresult::timer_state.GUID().empty());
    ASSERT_EQ(existing_project_id, testing::testresult::timer_state.PID());
    ASSERT_EQ(0, testing::testresult::timer_state.TID());

    // Set default task

    const uint64_t existing_task_id = 1879027;
    ASSERT_TRUE(toggl_set_default_project(app.ctx(), 0, existing_task_id));
    s = toggl_get_default_project_name(app.ctx());
    ASSERT_TRUE(s);
    ASSERT_EQ("dadsad. Testing stuff", std::string(s));
    free(s);

    // Start timer, the default task should apply

    testing::testresult::timer_state = TimeEntry();

    guid = toggl_start(app.ctx(), "more testing", "", 0, 0, 0, 0, false);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_FALSE(testing::testresult::timer_state.GUID().empty());
    ASSERT_EQ(existing_project_id, testing::testresult::timer_state.PID());
    ASSERT_EQ(existing_task_id, testing::testresult::timer_state.TID());

    // Setting task ID to 0 should not affect project ID

    ASSERT_TRUE(toggl_set_default_project(app.ctx(), existing_project_id, 0));
    s = toggl_get_default_project_name(app.ctx());
    ASSERT_TRUE(s);
    ASSERT_EQ(existing_project_name, std::string(s));
    free(s);

    // Setting task ID to not 0 should attach a project ID, too

    const uint64_t another_task_id = 1894794;
    ASSERT_TRUE(toggl_set_default_project(app.ctx(), 0, another_task_id));
    s = toggl_get_default_project_name(app.ctx());
    ASSERT_TRUE(s);
    ASSERT_EQ("blog (writing). Testing stuff", std::string(s));
    free(s);

    // Setting project ID to 0 should not clear out task ID

    ASSERT_TRUE(toggl_set_default_project(app.ctx(), 0, existing_task_id));
    s = toggl_get_default_project_name(app.ctx());
    ASSERT_TRUE(s);
    ASSERT_EQ("dadsad. Testing stuff", std::string(s));
    free(s);

    ASSERT_TRUE(toggl_set_default_project(app.ctx(), 0, 0));
    s = toggl_get_default_project_name(app.ctx());
    ASSERT_FALSE(s);
    free(s);
}

TEST(toggl_api, toggl_start) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::timer_state = TimeEntry();

    char_t *guid = toggl_start(app.ctx(), "test", "", 0, 0, 0, 0, false);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_FALSE(testing::testresult::timer_state.GUID().empty());
}

TEST(toggl_api, concurrency) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    const int kThreadCount = 2;

    std::vector<Poco::Thread *> threads;
    for (int i = 0; i < kThreadCount; i++) {
        std::stringstream ss;
        ss << "thread " << i;
        Poco::Thread *thread = new Poco::Thread(ss.str());
        threads.push_back(thread);
    }

    std::vector<testing::ApiClient *> runnables;
    for (int i = 0; i < kThreadCount; i++) {
        std::stringstream ss;
        ss << "runnable " << i;
        testing::ApiClient *runnable = new testing::ApiClient(&app, ss.str());
        runnables.push_back(runnable);
    }

    for (int i = 0; i < kThreadCount; i++) {
        threads[i]->start(*runnables[i]);
    }

    Poco::Thread::sleep(2000);

    for (int i = 0; i < kThreadCount; i++) {
        Poco::Thread *thread = threads[i];
        testing::ApiClient *runnable = runnables[i];
        while (!runnable->finished()) {
            Poco::Thread::sleep(10);
        }
        thread->join();
        delete runnable;
        delete thread;
    }
}

TEST(toggl_api, toggl_set_compact_mode) {
    testing::App app;

    toggl_set_compact_mode(app.ctx(), true);
    ASSERT_TRUE(toggl_get_compact_mode(app.ctx()));

    toggl_set_compact_mode(app.ctx(), false);
    ASSERT_FALSE(toggl_get_compact_mode(app.ctx()));
}

TEST(toggl_api, toggl_set_keep_end_times_fixed) {
    testing::App app;

    toggl_set_keep_end_time_fixed(app.ctx(), true);
    ASSERT_TRUE(toggl_get_keep_end_time_fixed(app.ctx()));

    toggl_set_keep_end_time_fixed(app.ctx(), false);
    ASSERT_FALSE(toggl_get_keep_end_time_fixed(app.ctx()));
}

TEST(toggl_api, toggl_set_window_maximized) {
    testing::App app;

    toggl_set_window_maximized(app.ctx(), true);
    ASSERT_TRUE(toggl_get_window_maximized(app.ctx()));

    toggl_set_window_maximized(app.ctx(), false);
    ASSERT_FALSE(toggl_get_window_maximized(app.ctx()));
}

TEST(toggl_api, toggl_set_window_minimized) {
    testing::App app;

    toggl_set_window_minimized(app.ctx(), true);
    ASSERT_TRUE(toggl_get_window_minimized(app.ctx()));

    toggl_set_window_minimized(app.ctx(), false);
    ASSERT_FALSE(toggl_get_window_minimized(app.ctx()));
}

TEST(toggl_api, toggl_set_window_edit_size_height) {
    testing::App app;

    toggl_set_window_edit_size_height(app.ctx(), 0);
    ASSERT_EQ(0, toggl_get_window_edit_size_height(app.ctx()));

    toggl_set_window_edit_size_height(app.ctx(), 123);
    ASSERT_EQ(123, toggl_get_window_edit_size_height(app.ctx()));
}

TEST(toggl_api, toggl_set_window_edit_size_width) {
    testing::App app;

    toggl_set_window_edit_size_width(app.ctx(), -10);
    ASSERT_EQ(-10, toggl_get_window_edit_size_width(app.ctx()));

    toggl_set_window_edit_size_width(app.ctx(), 1234);
    ASSERT_EQ(1234, toggl_get_window_edit_size_width(app.ctx()));
}

TEST(toggl_api, toggl_set_key_start) {
    testing::App app;

    toggl_set_key_start(app.ctx(), "a");
    char_t *res = toggl_get_key_start(app.ctx());
    ASSERT_TRUE(res);
    ASSERT_EQ("a", std::string(res));
    free(res);

    toggl_set_key_start(app.ctx(), "");
    res = toggl_get_key_start(app.ctx());
    ASSERT_TRUE(res);
    ASSERT_EQ("", std::string(res));
    free(res);
}

TEST(toggl_api, toggl_set_key_show) {
    testing::App app;

    toggl_set_key_show(app.ctx(), "a");
    char_t *res = toggl_get_key_show(app.ctx());
    ASSERT_TRUE(res);
    ASSERT_EQ("a", std::string(res));
    free(res);

    toggl_set_key_show(app.ctx(), "");
    res = toggl_get_key_show(app.ctx());
    ASSERT_TRUE(res);
    ASSERT_EQ("", std::string(res));
    free(res);
}

TEST(toggl_api, toggl_set_key_modifier_start) {
    testing::App app;

    toggl_set_key_modifier_start(app.ctx(), "a");
    char_t *res = toggl_get_key_modifier_start(app.ctx());
    ASSERT_TRUE(res);
    ASSERT_EQ("a", std::string(res));
    free(res);

    toggl_set_key_modifier_start(app.ctx(), "");
    res = toggl_get_key_modifier_start(app.ctx());
    ASSERT_TRUE(res);
    ASSERT_EQ("", std::string(res));
    free(res);
}

TEST(toggl_api, toggl_start_with_tags) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::timer_state = TimeEntry();

    char_t *guid = toggl_start(app.ctx(), "test", "", 0, 0, 0, "a\tb\tc",
                               false);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_FALSE(testing::testresult::timer_state.GUID().empty());
    ASSERT_EQ(std::string("a\tb\tc"), testing::testresult::timer_state.Tags());
}

TEST(toggl_api, toggl_start_with_open_editor_on_shortcut_setting) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    ASSERT_TRUE(toggl_set_settings_open_editor_on_shortcut(app.ctx(), false));

    testing::testresult::editor_state = TimeEntry();

    char_t *guid = toggl_start(app.ctx(), "test", "", 0, 0, 0, 0, false);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_TRUE(testing::testresult::editor_state.GUID().empty());

    ASSERT_TRUE(toggl_set_settings_open_editor_on_shortcut(app.ctx(), true));

    testing::testresult::editor_state = TimeEntry();

    guid = toggl_start(app.ctx(), "test", "", 0, 0, 0, 0, false);
    ASSERT_TRUE(guid);
    // It should *not* open the editor, unless a shortcut was used
    // in the app, but this logic is driven from the UI instead of the lib.
    ASSERT_EQ(std::string(""), testing::testresult::editor_state.GUID());
    free(guid);
}

TEST(toggl_api, toggl_set_time_entry_billable) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::timer_state = TimeEntry();

    char_t *res = toggl_start(app.ctx(), "test", "", 0, 0, 0, 0, false);
    ASSERT_TRUE(res);
    free(res);

    std::string guid = testing::testresult::timer_state.GUID();
    ASSERT_FALSE(guid.empty());

    ASSERT_TRUE(toggl_set_time_entry_billable(app.ctx(), guid.c_str(), true));
    ASSERT_TRUE(testing::testresult::timer_state.Billable());

    ASSERT_TRUE(toggl_set_time_entry_billable(app.ctx(), guid.c_str(), false));
    ASSERT_FALSE(testing::testresult::timer_state.Billable());
}

TEST(toggl_api, toggl_set_time_entry_tags) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::timer_state = TimeEntry();

    char_t *res = toggl_start(app.ctx(), "test", "", 0, 0, 0, 0, false);
    ASSERT_TRUE(res);
    free(res);

    std::string guid = testing::testresult::timer_state.GUID();
    ASSERT_FALSE(guid.empty());

    ASSERT_TRUE(toggl_set_time_entry_tags(app.ctx(), guid.c_str(), "a|b|c"));
    ASSERT_EQ("a|b|c", testing::testresult::timer_state.Tags());

    ASSERT_TRUE(toggl_set_time_entry_tags(app.ctx(), guid.c_str(), "a"));
    ASSERT_EQ("a", testing::testresult::timer_state.Tags());

    ASSERT_TRUE(toggl_set_time_entry_tags(app.ctx(), guid.c_str(), ""));
    ASSERT_EQ("", testing::testresult::timer_state.Tags());
}

TEST(toggl_api, toggl_parse_duration_string_into_seconds) {
    int64_t seconds = toggl_parse_duration_string_into_seconds("15 seconds");
    ASSERT_EQ(15, seconds);
}

TEST(toggl_api,
     toggl_parse_duration_string_into_seconds_with_no_duration_string) {
    int64_t seconds = toggl_parse_duration_string_into_seconds(0);
    ASSERT_EQ(0, seconds);
}

TEST(toggl_api, toggl_discard_time_at_with_no_guid) {
    testing::App app;

    ASSERT_FALSE(toggl_discard_time_at(app.ctx(), 0, time(0), false));
}

TEST(toggl_api, toggl_discard_time_at_with_no_stop_time) {
    testing::App app;

    ASSERT_FALSE(toggl_discard_time_at(app.ctx(), "some fake guid", 0, false));
}

TEST(toggl_api, toggl_discard_time_at) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::timer_state = TimeEntry();

    // Start a time entry

    char_t *res = toggl_start(app.ctx(), "test", "", 0, 0, 0, 0, false);
    ASSERT_TRUE(res);
    free(res);

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

    res = toggl_start(app.ctx(), "test 2", "", 0, 0, 0, 0, false);
    ASSERT_TRUE(res);
    free(res);

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

TEST(toggl_api, toggl_search_help_articles) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::help_article_names.clear();
    toggl_search_help_articles(app.ctx(), "Tracking");
    ASSERT_TRUE(testing::testresult::help_article_names.size());
    ASSERT_TRUE(std::find(
        testing::testresult::help_article_names.begin(),
        testing::testresult::help_article_names.end(),
        "Tracking time") !=
                testing::testresult::help_article_names.end());

    testing::testresult::help_article_names.clear();
    toggl_search_help_articles(app.ctx(), "basic");
    ASSERT_TRUE(testing::testresult::help_article_names.size());
    ASSERT_TRUE(std::find(
        testing::testresult::help_article_names.begin(),
        testing::testresult::help_article_names.end(),
        "Basics") !=
                testing::testresult::help_article_names.end());
}

TEST(toggl_api, toggl_feedback_send) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    ASSERT_TRUE(toggl_feedback_send(app.ctx(),
                                    "Help", "I need help", ""));
}

TEST(toggl_api, toggl_set_time_entry_date) {
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

TEST(toggl_api, toggl_set_time_entry_start) {
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

TEST(toggl_api, toggl_set_time_entry_end) {
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

TEST(toggl_api, toggl_set_time_entry_end_prefers_same_day) {
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

TEST(toggl_api, toggl_autotracker_add_rule) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::error = noError;

    const uint64_t existing_project_id = 2598305;

    int64_t rule_id = toggl_autotracker_add_rule(
        app.ctx(), "delfi", existing_project_id, 0);
    ASSERT_EQ(noError, testing::testresult::error);
    ASSERT_TRUE(rule_id);

    testing::testresult::error = noError;
    rule_id = toggl_autotracker_add_rule(
        app.ctx(), "delfi", existing_project_id, 0);
    ASSERT_EQ("rule already exists", testing::testresult::error);
    ASSERT_FALSE(rule_id);

    const uint64_t existing_task_id = 1879027;

    testing::testresult::error = noError;
    rule_id = toggl_autotracker_add_rule(
        app.ctx(), "with task", 0, existing_task_id);
    ASSERT_EQ(noError, testing::testresult::error);
    ASSERT_TRUE(rule_id);

    testing::testresult::error = noError;
    rule_id = toggl_autotracker_add_rule(
        app.ctx(),
        "with task and project",
        existing_project_id,
        existing_task_id);
    ASSERT_EQ(noError, testing::testresult::error);
    ASSERT_TRUE(rule_id);
}

TEST(toggl_api, toggl_set_default_project) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    char_t *default_project_name = toggl_get_default_project_name(app.ctx());
    ASSERT_FALSE(default_project_name);
    free(default_project_name);

    testing::testresult::error = noError;
    bool_t res = toggl_set_default_project(app.ctx(), 123, 0);
    ASSERT_NE(noError, testing::testresult::error);
    ASSERT_FALSE(res);

    default_project_name = toggl_get_default_project_name(app.ctx());
    ASSERT_FALSE(default_project_name);
    free(default_project_name);

    const uint64_t existing_project_id = 2598305;
    const std::string existing_project_name = "Testing stuff";

    testing::testresult::error = noError;
    res = toggl_set_default_project(app.ctx(), existing_project_id, 0);
    ASSERT_TRUE(res);

    default_project_name = toggl_get_default_project_name(app.ctx());
    ASSERT_TRUE(default_project_name);
    ASSERT_EQ(existing_project_name, std::string(default_project_name));
    free(default_project_name);

    testing::testresult::error = noError;
    res = toggl_set_default_project(app.ctx(), 0, 0);
    ASSERT_TRUE(res);

    default_project_name = toggl_get_default_project_name(app.ctx());
    ASSERT_FALSE(default_project_name);
    free(default_project_name);
}

}  // namespace toggl
