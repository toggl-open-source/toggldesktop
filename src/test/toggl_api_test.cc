// Copyright 2014 Toggl Desktop developers.

#include <vector>

#include "gtest/gtest.h"

#include "toggl_api_test.h"
#include "./../https_client.h"
#include "./../proxy.h"
#include "./../toggl_api.h"
#include "./../toggl_api_private.h"
#include "./../model/obm_action.h"
#include "./../model/settings.h"
#include "./../model/time_entry.h"
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

static class TestRelatedData {
public:
    // on_display_settings
    ProtectedModel<Settings> settings { nullptr, true };

    // on_time_entry_editor
    ProtectedModel<TimeEntry> editor_state { nullptr, true };

    // on_display_timer_state
    ProtectedModel<TimeEntry> timer_state { nullptr , true };

    // on_project_autocomplete
    std::vector<std::string> projects;

    // on_client_select
    std::vector<std::string> clients;

    // on_time_entry_list
    ProtectedContainer<TimeEntry> time_entries { nullptr };

    // on_project_colors
    std::vector<std::string> project_colors;

    // on_obm_experiment
    ProtectedContainer<ObmExperiment> obm_experiments { nullptr };
} related {};

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
bool use_proxy(false);
Proxy proxy;

// on_display_idle_notification
std::string idle_guid("");
std::string idle_since("");
std::string idle_duration("");
uint64_t idle_started(0);
std::string idle_description("");

locked<TimeEntry> time_entry_by_id(uint64_t id) {
    locked<TimeEntry> te;
    std::cerr << "Looking for " << id << std::endl;
    for (std::size_t i = 0; i < related.time_entries.size();
            i++) {
        std::cerr << "\t Checking " << related.time_entries[i]->ID() << std::endl;
        if (related.time_entries[i]->ID() == id) {
            te = related.time_entries[i];
            break;
        }
    }
    return te;
}

// on_time_entry_editor
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
    testresult::update_url = to_string(url);
}

void on_unsynced_items(const int64_t count) {
    testresult::unsynced_item_count = count;
}

void on_error(
    const char_t *errmsg,
    const bool_t user_error) {
    if (errmsg) {
        testresult::error = to_string(errmsg);
        return;
    }
    testresult::error = std::string("");
}

void on_online_state(const int64_t state) {
    testresult::online_state = state;
}

void on_url(const char_t *url) {
    testresult::url = to_string(url);
}

void on_login(const bool_t open, const uint64_t user_id) {
    testresult::user_id = user_id;
}

void on_reminder(const char_t *title, const char_t *informative_text) {
    testresult::reminder_title = to_string(title);
    testresult::reminder_informative_text = to_string(informative_text);
}

void on_help_articles(TogglHelpArticleView *first) {
    testing::testresult::help_article_names.clear();
    TogglHelpArticleView *it = first;
    while (it) {
        std::string name(to_string(it->Name));
        testing::testresult::help_article_names.push_back(name);
        it = reinterpret_cast<TogglHelpArticleView *>(it->Next);
    }
}

void on_time_entry_list(
    const bool_t open,
    TogglTimeEntryView *first,
    const bool_t show_load_more) {
    related.time_entries.clear();
    TogglTimeEntryView *it = first;
    while (it) {
        locked<TimeEntry> te = related.time_entries.create();
        te->SetGUID(to_string(it->GUID));
        te->SetID(it->ID);
        te->SetDurationInSeconds(it->DurationInSeconds);
        te->SetDescription(to_string(it->Description));
        te->SetStart(it->Started);
        te->SetStop(it->Ended);
        it = reinterpret_cast<TogglTimeEntryView *>(it->Next);
    }
}

void on_time_entry_autocomplete(TogglAutocompleteView *first) {
}

void on_mini_timer_autocomplete(TogglAutocompleteView *first) {
}

void on_project_autocomplete(TogglAutocompleteView *first) {
    related.projects.clear();
    TogglAutocompleteView *it = first;
    while (it) {
        related.projects.push_back(
            to_string(it->ProjectLabel));
        it = reinterpret_cast<TogglAutocompleteView *>(it->Next);
    }
}

void on_client_select(TogglGenericView *first) {
    related.clients.clear();
    TogglGenericView *it = first;
    while (it) {
        related.clients.push_back(to_string(it->Name));
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
    const char_t *focused_field_name) {
    related.editor_state.create();
    related.editor_state->SetGUID(to_string(te->GUID));
    testing::testresult::editor_open = open;
    testing::testresult::editor_focused_field_name =
        to_string(focused_field_name);
}

void on_display_settings(
    const bool_t open,
    TogglSettingsView *settings) {

    testing::testresult::open_settings = open;

    related.settings->use_idle_detection =settings->UseIdleDetection;
    related.settings->menubar_project = settings->MenubarProject;
    related.settings->autodetect_proxy = settings->AutodetectProxy;
    related.settings->menubar_timer = settings->MenubarTimer;
    related.settings->reminder = settings->Reminder;
    related.settings->dock_icon = settings->DockIcon;
    related.settings->on_top = settings->OnTop;
    related.settings->idle_minutes = settings->IdleMinutes;
    related.settings->reminder_minutes = settings->ReminderMinutes;
    related.settings->focus_on_shortcut = settings->FocusOnShortcut;
    related.settings->manual_mode = settings->ManualMode;
    related.settings->autotrack = settings->Autotrack;

    testing::testresult::use_proxy = settings->UseProxy;

    testing::testresult::proxy.SetHost(to_string(settings->ProxyHost));
    testing::testresult::proxy.SetPort(settings->ProxyPort);
    testing::testresult::proxy.SetUsername(
        to_string(settings->ProxyUsername));
    testing::testresult::proxy.SetPassword(
        to_string(settings->ProxyPassword));

    related.settings->remind_starts = to_string(settings->RemindStarts);
    related.settings->remind_ends = to_string(settings->RemindEnds);
    related.settings->remind_mon = settings->RemindMon;
    related.settings->remind_tue = settings->RemindTue;
    related.settings->remind_wed = settings->RemindWed;
    related.settings->remind_thu = settings->RemindThu;
    related.settings->remind_fri = settings->RemindFri;
    related.settings->remind_sat = settings->RemindSat;
    related.settings->remind_sun = settings->RemindSun;
}

void on_project_colors(
    string_list_t color_list,
    const uint64_t color_count) {
    related.project_colors.clear();
    for (uint64_t i = 0; i < color_count; i++) {
        related.project_colors.push_back(to_string(color_list[i]));
    }
}

void on_obm_experiment(
    const uint64_t nr,
    const bool_t included,
    const bool_t seen) {
    locked<ObmExperiment> experiment = related.obm_experiments.create();
    experiment->SetNr(nr);
    experiment->SetIncluded(included);
    experiment->SetHasSeen(seen);
}

void on_display_timer_state(TogglTimeEntryView *te) {
    related.timer_state.create();
    if (te) {
        related.timer_state->SetID(te->ID);
        related.timer_state->SetStart(te->Started);
        related.timer_state->SetGUID(to_string(te->GUID));
        related.timer_state->SetDurationInSeconds(
            te->DurationInSeconds);
        related.timer_state->SetDescription(to_string(te->Description));
        if (te->Tags) {
            related.timer_state->SetTags(to_string(te->Tags));
        }
        related.timer_state->SetBillable(te->Billable);
        related.timer_state->SetPID(te->PID);
        related.timer_state->SetTID(te->TID);
    }
}

void on_display_idle_notification(
    const char_t *guid,
    const char_t *since,
    const char_t *duration,
    const int64_t started,
    const char_t *description) {
    testing::testresult::idle_since = to_string(since);
    testing::testresult::idle_started = started;
    testing::testresult::idle_duration = to_string(duration);
    testing::testresult::idle_guid = to_string(guid);
    testing::testresult::idle_description = to_string(description);
}

class App {
 public:
    App() {
        Poco::File f("test.db");
        std::cout << f.path();
        if (f.exists()) {
            f.remove(false);
        }

        toggl_set_log_path(STR("test.log"));

        ctx_ = toggl_context_init(STR("tests"), STR("0.1"));

        poco_assert(toggl_set_db_path(ctx_, STR("test.db")));

        Poco::Path path(std::string(SRCDIR) + "ssl/cacert.pem");
        toggl_set_cacert_path(ctx_, to_char_t(path.toString()));

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
            char_t *guid = toggl_start(app_->ctx(), STR("test"), STR(""), 0, 0, 0, 0,
                                       false, 0, 0);
            ASSERT_TRUE(guid);

            ASSERT_TRUE(toggl_stop(app_->ctx(), false));

            toggl_edit(app_->ctx(), guid, true, STR(""));

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
    auto s = toggl_run_script(app.ctx(), STR("print 'test'"), &err);
    std::string res(to_string(s));
    free(s);
    ASSERT_EQ(0, err);
    ASSERT_EQ("0 value(s) returned\n\n\n", res);
}

TEST(toggl_api, toggl_run_script_with_invalid_script) {
    testing::App app;
    int64_t err(0);
    auto s = toggl_run_script(app.ctx(), STR("foo bar"), &err);
    std::string res(to_string(s));
    free(s);
    ASSERT_NE(0, err);
    ASSERT_EQ("[string \"foo bar\"]:1: syntax error near 'bar'", res);
}

TEST(toggl_api, toggl_add_obm_experiment_nr) {
    testing::App app;

    toggl_add_obm_experiment_nr(123);
    ASSERT_EQ("tests/0.1-obm-123", toggl::HTTPSClient::Config.UserAgent());

    toggl_add_obm_experiment_nr(456);
    ASSERT_EQ("tests/0.1-obm-123-obm-456", toggl::HTTPSClient::Config.UserAgent());
}

TEST(toggl_api, toggl_set_settings) {
    testing::App app;

    // set to false/null

    ASSERT_TRUE(toggl_set_settings_menubar_project(app.ctx(), false));
    ASSERT_FALSE(related.settings->menubar_project);

    ASSERT_TRUE(toggl_set_settings_autodetect_proxy(app.ctx(), false));
    ASSERT_FALSE(related.settings->autodetect_proxy);

    ASSERT_TRUE(toggl_set_settings_use_idle_detection(app.ctx(), false));
    ASSERT_FALSE(related.settings->use_idle_detection);

    ASSERT_TRUE(toggl_set_settings_menubar_timer(app.ctx(), false));
    ASSERT_FALSE(related.settings->menubar_timer);

    ASSERT_TRUE(toggl_set_settings_dock_icon(app.ctx(), false));
    ASSERT_FALSE(related.settings->dock_icon);

    ASSERT_TRUE(toggl_set_settings_on_top(app.ctx(), false));
    ASSERT_FALSE(related.settings->on_top);

    ASSERT_TRUE(toggl_set_settings_reminder(app.ctx(), false));
    ASSERT_FALSE(related.settings->reminder);

    ASSERT_TRUE(toggl_set_settings_idle_minutes(app.ctx(), 0));
    ASSERT_EQ(Poco::UInt64(1), related.settings->idle_minutes);

    ASSERT_TRUE(toggl_set_settings_reminder_minutes(app.ctx(), 0));
    ASSERT_EQ(Poco::UInt64(1), related.settings->reminder_minutes);

    ASSERT_TRUE(toggl_set_settings_focus_on_shortcut(app.ctx(), false));
    ASSERT_FALSE(related.settings->focus_on_shortcut);

    ASSERT_TRUE(toggl_set_settings_manual_mode(app.ctx(), false));
    ASSERT_FALSE(related.settings->manual_mode);

    // set to true / not null

    ASSERT_TRUE(toggl_set_settings_menubar_project(app.ctx(), true));
    ASSERT_TRUE(related.settings->menubar_project);

    ASSERT_TRUE(toggl_set_settings_autodetect_proxy(app.ctx(), true));
    ASSERT_TRUE(related.settings->autodetect_proxy);

    ASSERT_TRUE(toggl_set_settings_use_idle_detection(app.ctx(), true));
    ASSERT_TRUE(related.settings->use_idle_detection);

    ASSERT_TRUE(toggl_set_settings_menubar_timer(app.ctx(), true));
    ASSERT_TRUE(related.settings->menubar_timer);

    ASSERT_TRUE(toggl_set_settings_dock_icon(app.ctx(), true));
    ASSERT_TRUE(related.settings->dock_icon);

    ASSERT_TRUE(toggl_set_settings_on_top(app.ctx(), true));
    ASSERT_TRUE(related.settings->on_top);

    ASSERT_TRUE(toggl_set_settings_reminder(app.ctx(), true));
    ASSERT_TRUE(related.settings->reminder);

    ASSERT_TRUE(toggl_set_settings_idle_minutes(app.ctx(), 123));
    ASSERT_EQ(Poco::UInt64(123), related.settings->idle_minutes);

    ASSERT_TRUE(toggl_set_settings_reminder_minutes(app.ctx(), 222));
    ASSERT_EQ(Poco::UInt64(222),
              related.settings->reminder_minutes);

    ASSERT_TRUE(toggl_set_settings_focus_on_shortcut(app.ctx(), true));
    ASSERT_TRUE(related.settings->focus_on_shortcut);

    ASSERT_TRUE(toggl_set_settings_manual_mode(app.ctx(), true));
    ASSERT_TRUE(related.settings->manual_mode);

    testing::testresult::error = noError;
    ASSERT_TRUE(toggl_set_settings_autotrack(app.ctx(), true));
    ASSERT_EQ(noError, testing::testresult::error);
    ASSERT_TRUE(related.settings->autotrack);
}

TEST(toggl_api, toggl_set_proxy_settings) {
    testing::App app;

    ASSERT_TRUE(toggl_set_proxy_settings(
        app.ctx(), 1, STR("localhost"), 8000, STR("johnsmith"), STR("secret")));

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

    ASSERT_TRUE(related.settings->remind_mon);
    ASSERT_TRUE(related.settings->remind_tue);
    ASSERT_TRUE(related.settings->remind_wed);
    ASSERT_TRUE(related.settings->remind_thu);
    ASSERT_TRUE(related.settings->remind_fri);
    ASSERT_TRUE(related.settings->remind_sat);
    ASSERT_TRUE(related.settings->remind_sun);

    ASSERT_TRUE(toggl_set_settings_remind_days(
        app.ctx(), false, false, false, false, false, false, false));

    ASSERT_FALSE(related.settings->remind_mon);
    ASSERT_FALSE(related.settings->remind_tue);
    ASSERT_FALSE(related.settings->remind_wed);
    ASSERT_FALSE(related.settings->remind_thu);
    ASSERT_FALSE(related.settings->remind_fri);
    ASSERT_FALSE(related.settings->remind_sat);
    ASSERT_FALSE(related.settings->remind_sun);
}

TEST(toggl_api, toggl_set_settings_remind_times) {
    testing::App app;

    ASSERT_TRUE(toggl_set_settings_remind_times(app.ctx(), STR(""), STR("")));

    ASSERT_EQ(std::string(""), related.settings->remind_starts);
    ASSERT_EQ(std::string(""), related.settings->remind_ends);

    ASSERT_TRUE(toggl_set_settings_remind_times(app.ctx(), STR("09:30"), STR("17:30")));

    ASSERT_EQ(std::string("09:30"),
              related.settings->remind_starts);
    ASSERT_EQ(std::string("17:30"),
              related.settings->remind_ends);
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

    auto str = toggl_get_user_fullname(app.ctx());
    ASSERT_EQ("", to_string(str));
    free(str);

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    str = toggl_get_user_fullname(app.ctx());
    ASSERT_EQ("John Smith", to_string(str));
    free(str);
}

TEST(toggl_api, toggl_get_user_email) {
    testing::App app;

    auto str = toggl_get_user_email(app.ctx());
    ASSERT_EQ("", to_string(str));
    free(str);

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    str = toggl_get_user_email(app.ctx());
    ASSERT_EQ("johnsmith@toggl.com", to_string(str));
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
    auto str = toggl_get_update_channel(app.ctx());
    ASSERT_TRUE(default_channel == to_string(str)
                || std::string("beta") == to_string(str));
    free(str);

    ASSERT_FALSE(toggl_set_update_channel(app.ctx(), STR("invalid")));

    // The channel should be the same in the API
    str = toggl_get_update_channel(app.ctx());
    ASSERT_TRUE(default_channel == to_string(str) ||
                "beta" == to_string(str));
    free(str);

    ASSERT_TRUE(toggl_set_update_channel(app.ctx(), STR("beta")));

    // The channel should have been changed in the API
    str = toggl_get_update_channel(app.ctx());
    ASSERT_EQ(std::string("beta"), to_string(str));
    free(str);

    ASSERT_TRUE(toggl_set_update_channel(app.ctx(), STR("dev")));

    // The channel should have been changed in the API
    str = toggl_get_update_channel(app.ctx());
    ASSERT_EQ(std::string("dev"), to_string(str));
    free(str);

    ASSERT_TRUE(toggl_set_update_channel(app.ctx(), STR("stable")));

    // The channel should have been changed in the API
    str = toggl_get_update_channel(app.ctx());
    ASSERT_EQ(std::string("stable"), to_string(str));
    free(str);
}

TEST(toggl_api, toggl_set_log_level) {
    toggl_set_log_level(STR("trace"));
}

TEST(toggl_api, toggl_format_tracking_time_duration) {
    auto str = toggl_format_tracking_time_duration(10);
    ASSERT_EQ("10 sec", to_string(str));
    free(str);

    str = toggl_format_tracking_time_duration(60);
    ASSERT_EQ("01:00 min", to_string(str));
    free(str);

    str = toggl_format_tracking_time_duration(65);
    ASSERT_EQ("01:05 min", to_string(str));
    free(str);

    str = toggl_format_tracking_time_duration(3600);
    ASSERT_EQ("01:00:00", to_string(str));
    free(str);

    str = toggl_format_tracking_time_duration(5400);
    ASSERT_EQ("01:30:00", to_string(str));
    free(str);

    str = toggl_format_tracking_time_duration(5410);
    ASSERT_EQ("01:30:10", to_string(str));
    free(str);
}

TEST(toggl_api, toggl_format_tracked_time_duration) {
    auto str  = toggl_format_tracked_time_duration(10);
    ASSERT_EQ("0:00", to_string(str));
    free(str);

    str = toggl_format_tracked_time_duration(60);
    ASSERT_EQ("0:01", to_string(str));
    free(str);

    str = toggl_format_tracked_time_duration(65);
    ASSERT_EQ("0:01", to_string(str));
    free(str);

    str = toggl_format_tracked_time_duration(3600);
    ASSERT_EQ("1:00", to_string(str));
    free(str);

    str = toggl_format_tracked_time_duration(5400);
    ASSERT_EQ("1:30", to_string(str));
    free(str);

    str = toggl_format_tracked_time_duration(5410);
    ASSERT_EQ("1:30", to_string(str));
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

    toggl_set_environment(app.ctx(), STR("test"));

    auto env = toggl_environment(app.ctx());
    std::string res(to_string(env));
    free(env);
    ASSERT_EQ("test", res);
}

TEST(toggl_api, toggl_set_update_path) {
    testing::App app;

    toggl_set_update_path(app.ctx(), STR("/tmp/"));

    auto s = toggl_update_path(app.ctx());
    std::string path(to_string(s));
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
    toggl_set_log_path(STR("test.log"));
    toggl_debug(STR("Test 123"));
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

    char_t *guid = toggl_start(app.ctx(), STR("test"), STR(""), 0, 0, 0, 0, false, 0, 0);
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
                to_char_t(related.timer_state->GUID()),
                STR("301 seconds")));

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
    ASSERT_EQ("https://support.toggl.com/toggl-on-my-desktop/",
              testing::testresult::url);
}

TEST(toggl_api, toggl_login) {
    testing::App app;
    toggl_login(app.ctx(), STR("username"), STR("password"));
}

TEST(toggl_api, toggl_google_login) {
    testing::App app;
    toggl_google_login(app.ctx(), STR("token"));
}

TEST(toggl_api, toggl_sync) {
    testing::App app;
    toggl_sync(app.ctx());
}

TEST(toggl_api, toggl_add_obm_action) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    ASSERT_FALSE(toggl_add_obm_action(app.ctx(), 0, STR("key"), STR("value")));
    ASSERT_FALSE(toggl_add_obm_action(app.ctx(), 1, STR("key"), STR(" ")));
    ASSERT_FALSE(toggl_add_obm_action(app.ctx(), 2, STR(" "), STR("")));
    ASSERT_TRUE(toggl_add_obm_action(app.ctx(), 3, STR("key"), STR("value")));
    ASSERT_TRUE(toggl_add_obm_action(app.ctx(), 3, STR("key"), STR("value")));
}

TEST(toggl_api, toggl_add_project) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    auto guid = STR("07fba193-91c4-0ec8-2894-820df0548a8f");
    uint64_t wid = 0;
    uint64_t cid = 0;
    auto project_name = STR("");
    bool_t is_private = false;

    testing::testresult::error = "";
    char_t *res = toggl_add_project(app.ctx(),
                                    guid,
                                    wid,
                                    cid,
                                    STR(""),
                                    project_name,
                                    is_private,
                                    STR(""));
    ASSERT_EQ("Please select a workspace",
              testing::testresult::error);
    ASSERT_FALSE(res);

    wid = 123456789;
    res = toggl_add_project(app.ctx(),
                            guid,
                            wid,
                            cid,
                            STR(""),
                            project_name,
                            is_private,
                            STR("#ffffff"));
    ASSERT_EQ("Project name must not be empty",
              testing::testresult::error);
    ASSERT_FALSE(res);
    free(res);

    project_name = STR("A new project");
    testing::testresult::error = "";
    res = toggl_add_project(app.ctx(),
                            guid,
                            wid,
                            cid,
                            STR(""),
                            project_name,
                            is_private,
                            0);
    ASSERT_EQ("", testing::testresult::error);
    ASSERT_TRUE(res);
    free(res);

    // TODO: remove this when we address async issues in tests
    testing_sleep(1);

    bool found(false);
    for (std::size_t i = 0; i < related.projects.size(); i++) {
        if (to_string(project_name) == related.projects[i]) {
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
    auto client_name = STR("        ");

    testing::testresult::error = "";
    char_t *res = toggl_create_client(app.ctx(),
                                      wid,
                                      client_name);
    ASSERT_EQ("Please select a workspace", testing::testresult::error);
    ASSERT_FALSE(res);
    free(res);

    wid = 123456789;
    res = toggl_create_client(app.ctx(),
                              wid,
                              client_name);
    ASSERT_EQ("Client name must not be empty",
              testing::testresult::error);
    ASSERT_FALSE(res);
    free(res);

    client_name = STR("A new client");
    testing::testresult::error = "";
    res = toggl_create_client(app.ctx(),
                              wid,
                              client_name);
    ASSERT_EQ("", testing::testresult::error);
    ASSERT_TRUE(res);
    free(res);

    bool found(false);
    for (std::size_t i = 0; i < related.clients.size(); i++) {
        if (to_string(client_name) == related.clients[i]) {
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
                                  to_char_t(ss.str()));
        ASSERT_EQ("", testing::testresult::error);
        ASSERT_TRUE(res);
        free(res);
    }

    // But none with an existing client name
    testing::testresult::error = "";
    res = toggl_create_client(app.ctx(),
                              wid,
                              client_name);
    ASSERT_FALSE(res);
    ASSERT_EQ("Client name already exists", testing::testresult::error);
}

TEST(toggl_api, toggl_continue) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    auto te = testing::testresult::time_entry_by_id(89833438);
    std::string guid = te->GUID();

    testing::testresult::error = "";
    ASSERT_TRUE(toggl_continue(app.ctx(), to_char_t(guid)));
    ASSERT_NE(guid, related.timer_state->GUID());
    ASSERT_EQ("More work", related.timer_state->Description());
}

TEST(toggl_api, toggl_continue_in_manual_mode) {
    testing::App app;

    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    ASSERT_TRUE(toggl_set_settings_manual_mode(app.ctx(), true));
    ASSERT_TRUE(related.settings->manual_mode);

    auto te = testing::testresult::time_entry_by_id(89833438);
    std::string guid = te->GUID();

    testing::testresult::error = "";
    related.editor_state.create();
    related.timer_state.create();

    ASSERT_TRUE(toggl_continue(app.ctx(), to_char_t(guid)));

    ASSERT_NE(guid, related.timer_state->GUID());

    ASSERT_FALSE(related.timer_state->Start());
    ASSERT_FALSE(related.timer_state->DurationInSeconds());

    ASSERT_NE("", related.editor_state->GUID());
}

TEST(toggl_api, toggl_check_view_struct_size) {
    char_t *err = toggl_check_view_struct_size(
        sizeof(TogglTimeEntryView),
        sizeof(TogglAutocompleteView),
        sizeof(TogglGenericView),
        sizeof(TogglSettingsView),
        sizeof(TogglAutotrackerRuleView));
    if (err) {
        ASSERT_EQ("", to_string(err));
    }
    ASSERT_FALSE(err);
    free(err);
}

TEST(toggl_api, toggl_view_time_entry_list) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    toggl_view_time_entry_list(app.ctx());
    ASSERT_EQ(std::size_t(5), related.time_entries.size());
}

TEST(toggl_api, toggl_edit) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));
    auto te = testing::testresult::time_entry_by_id(89837259);
    std::string guid = te->GUID();
    bool_t edit_running_time_entry = false;
    auto focused_field = STR("description");
    toggl_edit(app.ctx(), to_char_t(guid), edit_running_time_entry,
               focused_field);
    ASSERT_EQ(guid, related.editor_state->GUID());
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
    ASSERT_EQ("arendus käib", related.timer_state->Description());
}

TEST(toggl_api, toggl_continue_latest_with_manual_mode) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    ASSERT_TRUE(toggl_set_settings_manual_mode(app.ctx(), true));
    ASSERT_TRUE(related.settings->manual_mode);

    testing::testresult::error = noError;
    related.editor_state.create();
    related.timer_state.create();

    ASSERT_TRUE(toggl_continue_latest(app.ctx(), false));

    ASSERT_EQ(noError, testing::testresult::error);

    ASSERT_FALSE(related.timer_state->Start());
    ASSERT_FALSE(related.timer_state->DurationInSeconds());

    ASSERT_NE("", related.editor_state->GUID());
}

TEST(toggl_api, toggl_delete_time_entry) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    toggl_view_time_entry_list(app.ctx());
    ASSERT_EQ(std::size_t(5), related.time_entries.size());

    auto te = testing::testresult::time_entry_by_id(89837259);
    std::string guid = te->GUID();
    ASSERT_TRUE(toggl_delete_time_entry(app.ctx(), to_char_t(guid)));

    toggl_view_time_entry_list(app.ctx());
    ASSERT_EQ(std::size_t(4), related.time_entries.size());
}

TEST(toggl_api, toggl_set_time_entry_duration) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    auto te = testing::testresult::time_entry_by_id(89818605);
    std::string guid = te->GUID();

    ASSERT_TRUE(toggl_set_time_entry_duration(app.ctx(),
                to_char_t(guid), STR("2 hours")));

    toggl_view_time_entry_list(app.ctx());

    for (std::size_t i = 0; i < related.time_entries.size();
            i++) {
        if (related.time_entries[i]->GUID() == guid) {
            te = related.time_entries[i];
            break;
        }
    }
    ASSERT_EQ(guid, te->GUID());
    ASSERT_EQ(7200, te->DurationInSeconds());
}

TEST(toggl_api, toggl_set_time_entry_description) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    auto te = testing::testresult::time_entry_by_id(89818605);
    std::string guid = te->GUID();

    ASSERT_TRUE(toggl_set_time_entry_description(app.ctx(),
                to_char_t(guid), STR("this is a nuclear test")));

    toggl_view_time_entry_list(app.ctx());

    for (std::size_t i = 0; i < related.time_entries.size();
            i++) {
        if (related.time_entries[i]->GUID() == guid) {
            te = related.time_entries[i];
            break;
        }
    }
    ASSERT_EQ(guid, te->GUID());
    ASSERT_EQ("this is a nuclear test", te->Description());
}

TEST(toggl_api, toggl_stop) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    related.timer_state.create();

    char_t *guid = toggl_start(app.ctx(), STR("test"), STR(""), 0, 0, 0, 0, false, 0, 0);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_FALSE(related.timer_state->GUID().empty());

    ASSERT_TRUE(toggl_stop(app.ctx(), false));
    ASSERT_TRUE(related.timer_state->GUID().empty());
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

    related.timer_state.create();

    char_t *guid = toggl_start(app.ctx(), STR("test"), STR(""), 0, 0, 0, 0, false, 0, 0);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_FALSE(related.timer_state->GUID().empty());
    ASSERT_FALSE(related.timer_state->PID());
    ASSERT_FALSE(related.timer_state->TID());

    // Set default project ID.
    // Task ID should remain 0

    const uint64_t existing_project_id = 2598305;
    const std::string existing_project_name = "Testing stuff";
    ASSERT_TRUE(toggl_set_default_project(app.ctx(), existing_project_id, 0));
    s = toggl_get_default_project_name(app.ctx());
    ASSERT_TRUE(s);
    ASSERT_EQ(existing_project_name, to_string(s));
    free(s);

    // Start timer, the default project should apply

    related.timer_state.create();

    guid = toggl_start(app.ctx(), STR("test"), STR(""), 0, 0, 0, 0, false, 0, 0);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_FALSE(related.timer_state->GUID().empty());
    ASSERT_EQ(existing_project_id, related.timer_state->PID());
    ASSERT_EQ(0, related.timer_state->TID());

    // Set default task

    const uint64_t existing_task_id = 1879027;
    ASSERT_TRUE(toggl_set_default_project(app.ctx(), 0, existing_task_id));
    s = toggl_get_default_project_name(app.ctx());
    ASSERT_TRUE(s);
    ASSERT_EQ("dadsad. Testing stuff", to_string(s));
    free(s);

    // Start timer, the default task should apply

    related.timer_state.create();

    guid = toggl_start(app.ctx(), STR("more testing"), STR(""), 0, 0, 0, 0, false, 0, 0);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_FALSE(related.timer_state->GUID().empty());
    ASSERT_EQ(existing_project_id, related.timer_state->PID());
    ASSERT_EQ(existing_task_id, related.timer_state->TID());

    // Setting task ID to 0 should not affect project ID

    ASSERT_TRUE(toggl_set_default_project(app.ctx(), existing_project_id, 0));
    s = toggl_get_default_project_name(app.ctx());
    ASSERT_TRUE(s);
    ASSERT_EQ(existing_project_name, to_string(s));
    free(s);

    // Setting task ID to not 0 should attach a project ID, too

    const uint64_t another_task_id = 1894794;
    ASSERT_TRUE(toggl_set_default_project(app.ctx(), 0, another_task_id));
    s = toggl_get_default_project_name(app.ctx());
    ASSERT_TRUE(s);
    ASSERT_EQ("blog (writing). Testing stuff", to_string(s));
    free(s);

    // Setting project ID to 0 should not clear out task ID

    ASSERT_TRUE(toggl_set_default_project(app.ctx(), 0, existing_task_id));
    s = toggl_get_default_project_name(app.ctx());
    ASSERT_TRUE(s);
    ASSERT_EQ("dadsad. Testing stuff", to_string(s));
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

    related.timer_state.create();

    char_t *guid = toggl_start(app.ctx(), STR("test"), STR(""), 0, 0, 0, 0, false, 0, 0);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_FALSE(related.timer_state->GUID().empty());
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

    toggl_set_key_start(app.ctx(), STR("a"));
    char_t *res = toggl_get_key_start(app.ctx());
    ASSERT_TRUE(res);
    ASSERT_EQ("a", to_string(res));
    free(res);

    toggl_set_key_start(app.ctx(), STR(""));
    res = toggl_get_key_start(app.ctx());
    ASSERT_TRUE(res);
    ASSERT_EQ("", to_string(res));
    free(res);
}

TEST(toggl_api, toggl_set_key_show) {
    testing::App app;

    toggl_set_key_show(app.ctx(), STR("a"));
    char_t *res = toggl_get_key_show(app.ctx());
    ASSERT_TRUE(res);
    ASSERT_EQ("a", to_string(res));
    free(res);

    toggl_set_key_show(app.ctx(), STR(""));
    res = toggl_get_key_show(app.ctx());
    ASSERT_TRUE(res);
    ASSERT_EQ("", to_string(res));
    free(res);
}

TEST(toggl_api, toggl_set_key_modifier_start) {
    testing::App app;

    toggl_set_key_modifier_start(app.ctx(), STR("a"));
    char_t *res = toggl_get_key_modifier_start(app.ctx());
    ASSERT_TRUE(res);
    ASSERT_EQ("a", to_string(res));
    free(res);

    toggl_set_key_modifier_start(app.ctx(), STR(""));
    res = toggl_get_key_modifier_start(app.ctx());
    ASSERT_TRUE(res);
    ASSERT_EQ("", to_string(res));
    free(res);
}

TEST(toggl_api, toggl_start_with_tags) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    related.timer_state.create();

    char_t *guid = toggl_start(app.ctx(), STR("test"), STR(""), 0, 0, 0, STR("a\tb\tc"),
                               false, 0, 0);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_FALSE(related.timer_state->GUID().empty());
    ASSERT_EQ(std::string("a\tb\tc"), related.timer_state->Tags());
}

TEST(toggl_api, toggl_start_with_open_editor_on_shortcut_setting) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    ASSERT_TRUE(toggl_set_settings_open_editor_on_shortcut(app.ctx(), false));

    related.editor_state.create();

    char_t *guid = toggl_start(app.ctx(), STR("test"), STR(""), 0, 0, 0, 0, false, 0, 0);
    ASSERT_TRUE(guid);
    free(guid);

    ASSERT_TRUE(related.editor_state->GUID().empty());

    ASSERT_TRUE(toggl_set_settings_open_editor_on_shortcut(app.ctx(), true));

    related.editor_state.create();

    guid = toggl_start(app.ctx(), STR("test"), STR(""), 0, 0, 0, 0, false, 0, 0);
    ASSERT_TRUE(guid);

    ASSERT_EQ(to_string(guid), related.editor_state->GUID());
    free(guid);
}

TEST(toggl_api, toggl_set_time_entry_billable) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    related.timer_state.create();

    char_t *res = toggl_start(app.ctx(), STR("test"), STR(""), 0, 0, 0, 0, false, 0, 0);
    ASSERT_TRUE(res);
    free(res);

    std::string guid = related.timer_state->GUID();
    ASSERT_FALSE(guid.empty());

    ASSERT_TRUE(toggl_set_time_entry_billable(app.ctx(), to_char_t(guid), true));
    ASSERT_TRUE(related.timer_state->Billable());

    ASSERT_TRUE(toggl_set_time_entry_billable(app.ctx(), to_char_t(guid), false));
    ASSERT_FALSE(related.timer_state->Billable());
}

TEST(toggl_api, toggl_set_time_entry_tags) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    related.timer_state.create();

    char_t *res = toggl_start(app.ctx(), STR("test"), STR(""), 0, 0, 0, 0, false, 0, 0);
    ASSERT_TRUE(res);
    free(res);

    std::string guid = related.timer_state->GUID();
    ASSERT_FALSE(guid.empty());

    ASSERT_TRUE(toggl_set_time_entry_tags(app.ctx(), to_char_t(guid), STR("a|b|c")));
    ASSERT_EQ("a|b|c", related.timer_state->Tags());

    ASSERT_TRUE(toggl_set_time_entry_tags(app.ctx(), to_char_t(guid), STR("a")));
    ASSERT_EQ("a", related.timer_state->Tags());

    ASSERT_TRUE(toggl_set_time_entry_tags(app.ctx(), to_char_t(guid), STR("")));
    ASSERT_EQ("", related.timer_state->Tags());
}

TEST(toggl_api, toggl_parse_duration_string_into_seconds) {
    int64_t seconds = toggl_parse_duration_string_into_seconds(STR("15 seconds"));
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

    ASSERT_FALSE(toggl_discard_time_at(app.ctx(), STR("some fake guid"), 0, false));
}

TEST(toggl_api, toggl_discard_time_at) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    related.timer_state.create();

    // Start a time entry

    char_t *res = toggl_start(app.ctx(), STR("test"), STR(""), 0, 0, 0, 0, false, 0, 0);
    ASSERT_TRUE(res);
    free(res);

    Poco::UInt64 started = time(0);
    std::string guid = related.timer_state->GUID();
    ASSERT_FALSE(guid.empty());

    // Discard the time entry at some point

    Poco::UInt64 stopped = time(0);
    ASSERT_TRUE(toggl_discard_time_at(app.ctx(), to_char_t(guid), stopped, false));
    ASSERT_NE(guid, related.timer_state->GUID());
    ASSERT_TRUE(related.timer_state->GUID().empty());

    locked<TimeEntry> te;
    for (std::size_t i = 0; i < related.time_entries.size();
            i++) {
        if (related.time_entries[i]->GUID() == guid) {
            te = related.time_entries[i];
            break;
        }
    }
    ASSERT_EQ(guid, te->GUID());
    ASSERT_TRUE(started == te->Start() || started + 1 == te->Start());
    ASSERT_TRUE(stopped == te->Stop() || stopped + 1 == te->Stop());

    // Start another time entry

    res = toggl_start(app.ctx(), STR("test 2"), STR(""), 0, 0, 0, 0, false, 0, 0);
    ASSERT_TRUE(res);
    free(res);

    started = time(0);
    guid = related.timer_state->GUID();
    ASSERT_FALSE(guid.empty());

    // Discard the time entry, by creating a new one

    stopped = time(0);
    ASSERT_TRUE(toggl_discard_time_at(app.ctx(), to_char_t(guid), stopped, true));
    ASSERT_NE(guid, related.timer_state->GUID());

    te = locked<TimeEntry>();
    for (std::size_t i = 0; i < related.time_entries.size();
            i++) {
        if (related.time_entries[i]->GUID() == guid) {
            te = related.time_entries[i];
            break;
        }
    }
    ASSERT_EQ(guid, te->GUID());
    ASSERT_TRUE(started == te->Start() || started + 1 == te->Start());
    ASSERT_TRUE(stopped == te->Stop() || stopped + 1 == te->Stop());

    // Check that a new time entry was created

    ASSERT_TRUE(!related.timer_state->GUID().empty());
    ASSERT_EQ(stopped, related.timer_state->Start());
    ASSERT_TRUE(related.timer_state->IsTracking());
    ASSERT_EQ("", related.timer_state->Description());
}

TEST(toggl_api, toggl_search_help_articles) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    testing::testresult::help_article_names.clear();
    toggl_search_help_articles(app.ctx(), STR("Tracking"));
    ASSERT_TRUE(testing::testresult::help_article_names.size());
    ASSERT_TRUE(std::find(
        testing::testresult::help_article_names.begin(),
        testing::testresult::help_article_names.end(),
        "Tracking time") !=
                testing::testresult::help_article_names.end());

    testing::testresult::help_article_names.clear();
    toggl_search_help_articles(app.ctx(), STR("basic"));
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
                                    STR("Help"), STR("I need help"), STR("")));
}

TEST(toggl_api, toggl_set_time_entry_date) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    auto te = testing::testresult::time_entry_by_id(89818605);
    std::string guid = te->GUID();

    Poco::DateTime datetime(Poco::Timestamp::fromEpochTime(te->Start()));
    ASSERT_EQ(2013, datetime.year());
    ASSERT_EQ(9, datetime.month());
    ASSERT_EQ(5, datetime.day());
    ASSERT_EQ(6, datetime.hour());
    ASSERT_EQ(33, datetime.minute());
    ASSERT_EQ(50, datetime.second());

    // 10/27/2014 @ 12:51pm in UTC.
    int unix_timestamp(1414414311);
    ASSERT_TRUE(toggl_set_time_entry_date(app.ctx(),
                                          to_char_t(guid),
                                          unix_timestamp));

    te = testing::testresult::time_entry_by_id(89818605);
    datetime = Poco::DateTime(Poco::Timestamp::fromEpochTime(te->Start()));
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

    auto te = testing::testresult::time_entry_by_id(89818605);
    std::string guid = te->GUID();

    Poco::DateTime datetime(Poco::Timestamp::fromEpochTime(te->Start()));
    ASSERT_EQ(2013, datetime.year());
    ASSERT_EQ(9, datetime.month());
    ASSERT_EQ(5, datetime.day());
    ASSERT_EQ(6, datetime.hour());
    ASSERT_EQ(33, datetime.minute());
    ASSERT_EQ(50, datetime.second());

    ASSERT_TRUE(toggl_set_time_entry_start(app.ctx(), to_char_t(guid), STR("12:34")));

    te = testing::testresult::time_entry_by_id(89818605);
    Poco::LocalDateTime local =
        Poco::DateTime(Poco::Timestamp::fromEpochTime(te->Start()));
    ASSERT_EQ(2013, local.year());
    ASSERT_EQ(9, local.month());
    ASSERT_EQ(5, local.day());
    ASSERT_EQ(12, local.hour());
    ASSERT_EQ(34, local.minute());
    ASSERT_EQ(50, local.second());

    // Setting an invalid value should not crash the app
    ASSERT_FALSE(toggl_set_time_entry_start(app.ctx(), to_char_t(guid), STR("12:558")));
}

TEST(toggl_api, toggl_set_time_entry_end) {
    testing::App app;
    std::string json = loadTestData();
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    auto te = testing::testresult::time_entry_by_id(89818605);
    std::string guid = te->GUID();

    Poco::DateTime datetime(Poco::Timestamp::fromEpochTime(te->Stop()));
    ASSERT_EQ(2013, datetime.year());
    ASSERT_EQ(9, datetime.month());
    ASSERT_EQ(5, datetime.day());
    ASSERT_EQ(8, datetime.hour());
    ASSERT_EQ(19, datetime.minute());
    ASSERT_EQ(46, datetime.second());

    ASSERT_TRUE(toggl_set_time_entry_end(app.ctx(), to_char_t(guid), STR("18:29")));

    te = testing::testresult::time_entry_by_id(89818605);
    Poco::LocalDateTime local =
        Poco::DateTime(Poco::Timestamp::fromEpochTime(te->Stop()));
    ASSERT_EQ(2013, local.year());
    ASSERT_EQ(9, local.month());
    ASSERT_EQ(5, local.day());
    ASSERT_EQ(18, local.hour());
    ASSERT_EQ(29, local.minute());
    ASSERT_EQ(46, local.second());

    // Setting an invalid value should not crash the app
    ASSERT_FALSE(toggl_set_time_entry_end(app.ctx(), to_char_t(guid), STR("12:558")));
}

TEST(toggl_api, toggl_set_time_entry_end_prefers_same_day) {
    testing::App app;
    std::string json =
        loadFromTestDataDir(std::string("time_entry_ending_tomorrow.json"));
    ASSERT_TRUE(testing_set_logged_in_user(app.ctx(), json.c_str()));

    auto te = testing::testresult::time_entry_by_id(89818605);
    std::string guid = te->GUID();

    // Set start time so it will be local time
    ASSERT_TRUE(toggl_set_time_entry_date(app.ctx(), to_char_t(guid), time(0)));
    ASSERT_TRUE(toggl_set_time_entry_start(app.ctx(), to_char_t(guid), STR("06:33")));

    ASSERT_TRUE(toggl_set_time_entry_end(app.ctx(), to_char_t(guid), STR("06:34")));

    // take the updated time entry
    te = testing::testresult::time_entry_by_id(89818605);

    Poco::DateTime start(Poco::Timestamp::fromEpochTime(te->Start()));
    Poco::DateTime end(Poco::Timestamp::fromEpochTime(te->Stop()));
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
        app.ctx(), STR("delfi"), existing_project_id, 0);
    ASSERT_EQ(noError, testing::testresult::error);
    ASSERT_TRUE(rule_id);

    testing::testresult::error = noError;
    rule_id = toggl_autotracker_add_rule(
        app.ctx(), STR("delfi"), existing_project_id, 0);
    ASSERT_EQ("rule already exists", testing::testresult::error);
    ASSERT_FALSE(rule_id);

    const uint64_t existing_task_id = 1879027;

    testing::testresult::error = noError;
    rule_id = toggl_autotracker_add_rule(
        app.ctx(), STR("with task"), 0, existing_task_id);
    ASSERT_EQ(noError, testing::testresult::error);
    ASSERT_TRUE(rule_id);

    testing::testresult::error = noError;
    rule_id = toggl_autotracker_add_rule(
        app.ctx(),
        STR("with task and project"),
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
    ASSERT_EQ(existing_project_name, to_string(default_project_name));
    free(default_project_name);

    testing::testresult::error = noError;
    res = toggl_set_default_project(app.ctx(), 0, 0);
    ASSERT_TRUE(res);

    default_project_name = toggl_get_default_project_name(app.ctx());
    ASSERT_FALSE(default_project_name);
    free(default_project_name);
}

}  // namespace toggl

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
