// Copyright 2014 Toggl Desktop developers.

#include "gtest/gtest.h"

#include <iostream>  // NOLINT

#include "./../lib/include/toggl_api.h"
#include "./../toggl_api_private.h"
#include "./test_data.h"
#include "./../settings.h"
#include "./../proxy.h"

#include "Poco/FileStream.h"
#include "Poco/File.h"
#include "Poco/Path.h"

namespace toggl {

namespace testing {

namespace testresult {
std::string url("");
std::string reminder_title("");
std::string reminder_informative_text("");
std::string error("");
bool online_state(false);
uint64_t user_id(0);
Settings settings;
bool use_proxy(false);
Proxy proxy;
std::string update_channel("");
}  // namespace testresult

void on_app(const _Bool open) {
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

void on_update(
    const _Bool open,
    TogglUpdateView *view) {
    testresult::update_channel = std::string(view->UpdateChannel);
}

void on_online_state(const _Bool is_online, const char *reason) {
    testresult::online_state = is_online;
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
}

void on_time_entry_autocomplete(TogglAutocompleteView *first) {
}

void on_project_autocomplete(TogglAutocompleteView *first) {
}

void on_client_select(TogglGenericView *first) {
}

void on_workspace_select(TogglGenericView *first) {
}

void on_tags(TogglGenericView *first) {
}

void on_time_entry_editor(
    const _Bool open,
    TogglTimeEntryView *te,
    const char *focused_field_name) {
}

void on_display_settings(
    const _Bool open,
    TogglSettingsView *settings) {

    testing::testresult::settings.use_idle_detection =
        settings->UseIdleDetection;
    testing::testresult::settings.menubar_timer = settings->MenubarTimer;
    testing::testresult::settings.reminder = settings->Reminder;
    testing::testresult::settings.dock_icon = settings->DockIcon;
    testing::testresult::settings.on_top = settings->OnTop;

    testing::testresult::use_proxy = settings->UseProxy;

    testing::testresult::proxy.host = std::string(settings->ProxyHost);
    testing::testresult::proxy.port = settings->ProxyPort;
    testing::testresult::proxy.username = std::string(settings->ProxyUsername);
    testing::testresult::proxy.password = std::string(settings->ProxyPassword);
}

void on_display_timer_state(TogglTimeEntryView *te) {
}

void on_display_idle_notification(
    const char *guid,
    const char *since,
    const char *duration,
    const uint64_t started) {
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
        toggl_on_error(ctx_, on_error);
        toggl_on_update(ctx_, on_update);
        toggl_on_online_state(ctx_, on_online_state);
        toggl_on_login(ctx_, on_login);
        toggl_on_url(ctx_, on_url);
        toggl_on_reminder(ctx_, on_reminder);
        toggl_on_time_entry_list(ctx_, on_time_entry_list);
        toggl_on_time_entry_autocomplete(ctx_, on_time_entry_autocomplete);
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

TEST(TogglApiTest, toggl_set_settings) {
    testing::App app;

    ASSERT_TRUE(toggl_set_settings(app.ctx(),
                                   false, false, false, false, false));

    ASSERT_FALSE(testing::testresult::settings.use_idle_detection);
    ASSERT_FALSE(testing::testresult::settings.menubar_timer);
    ASSERT_FALSE(testing::testresult::settings.dock_icon);
    ASSERT_FALSE(testing::testresult::settings.on_top);
    ASSERT_FALSE(testing::testresult::settings.reminder);

    ASSERT_TRUE(toggl_set_settings(app.ctx(),
                                   true, true, true, true, true));

    ASSERT_TRUE(testing::testresult::settings.use_idle_detection);
    ASSERT_TRUE(testing::testresult::settings.menubar_timer);
    ASSERT_TRUE(testing::testresult::settings.dock_icon);
    ASSERT_TRUE(testing::testresult::settings.on_top);
    ASSERT_TRUE(testing::testresult::settings.reminder);
}

TEST(TogglApiTest, toggl_set_proxy_settings) {
    testing::App app;

    ASSERT_TRUE(toggl_set_proxy_settings(
        app.ctx(), 1, "localhost", 8000, "johnsmith", "secret"));

    ASSERT_TRUE(testing::testresult::use_proxy);
    ASSERT_EQ(std::string("localhost"),
              std::string(testing::testresult::proxy.host));
    ASSERT_EQ(8000,
              static_cast<int>(testing::testresult::proxy.port));
    ASSERT_EQ(std::string("johnsmith"),
              std::string(testing::testresult::proxy.username));
    ASSERT_EQ(std::string("secret"),
              std::string(testing::testresult::proxy.password));
}

TEST(TogglApiTest, toggl_set_update_channel) {
    testing::App app;

    std::string default_channel("stable");

    toggl_about(app.ctx());

    ASSERT_EQ(default_channel, testing::testresult::update_channel);

    // Also check that the API itself thinks the default channel is
    char *str = toggl_get_update_channel(app.ctx());
    ASSERT_EQ(default_channel, std::string(str));
    free(str);

    ASSERT_FALSE(toggl_set_update_channel(app.ctx(), "invalid"));

    // The channel should be the same in the API
    str = toggl_get_update_channel(app.ctx());
    ASSERT_EQ(default_channel, std::string(str));
    free(str);

    ASSERT_TRUE(toggl_set_update_channel(app.ctx(), "beta"));

    ASSERT_EQ(std::string("beta"), testing::testresult::update_channel);

    // The channel should have been changed in the API
    str = toggl_get_update_channel(app.ctx());
    ASSERT_EQ(std::string("beta"), std::string(str));
    free(str);

    ASSERT_TRUE(toggl_set_update_channel(app.ctx(), "dev"));

    ASSERT_EQ(std::string("dev"), testing::testresult::update_channel);

    // The channel should have been changed in the API
    str = toggl_get_update_channel(app.ctx());
    ASSERT_EQ(std::string("dev"), std::string(str));
    free(str);

    ASSERT_TRUE(toggl_set_update_channel(app.ctx(), "stable"));

    ASSERT_EQ(std::string("stable"), testing::testresult::update_channel);

    // The channel should have been changed in the API
    str = toggl_get_update_channel(app.ctx());
    ASSERT_EQ(std::string("stable"), std::string(str));
    free(str);
}

TEST(TogglApiTest, toggl_set_log_level) {
    toggl_set_log_level("trace");
}

TEST(TogglApiTest, toggl_parse_time) {
    int hours = 0;
    int minutes = 0;
    bool valid = true;

    valid = toggl_parse_time("120a", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(1, hours);
    ASSERT_EQ(20, minutes);

    valid = toggl_parse_time("1P", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(13, hours);
    ASSERT_EQ(0, minutes);

    ASSERT_FALSE(toggl_parse_time("x", &hours, &minutes));

    valid = toggl_parse_time("2", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(2, hours);
    ASSERT_EQ(0, minutes);

    valid = toggl_parse_time("12", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(12, hours);
    ASSERT_EQ(0, minutes);

    valid = toggl_parse_time("1230", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(12, hours);
    ASSERT_EQ(30, minutes);

    ASSERT_FALSE(toggl_parse_time("12x", &hours, &minutes));

    ASSERT_FALSE(toggl_parse_time("12xx", &hours, &minutes));

    ASSERT_FALSE(toggl_parse_time(":", &hours, &minutes));

    ASSERT_FALSE(toggl_parse_time("11:", &hours, &minutes));

    ASSERT_FALSE(toggl_parse_time(":20", &hours, &minutes));

    ASSERT_FALSE(toggl_parse_time("11:xx", &hours, &minutes));

    valid = toggl_parse_time("11:20", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(11, hours);
    ASSERT_EQ(20, minutes);

    valid = toggl_parse_time("5:30", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(5, hours);
    ASSERT_EQ(30, minutes);

    valid = toggl_parse_time("5:30 PM", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(17, hours);
    ASSERT_EQ(30, minutes);

    valid = toggl_parse_time("5:30 odp.", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(17, hours);
    ASSERT_EQ(30, minutes);

    valid = toggl_parse_time("17:10", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(17, hours);
    ASSERT_EQ(10, minutes);

    valid = toggl_parse_time("12:00 AM", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(0, hours);
    ASSERT_EQ(0, minutes);

    valid = toggl_parse_time("12:00 dop.", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(0, hours);
    ASSERT_EQ(0, minutes);

    ASSERT_FALSE(toggl_parse_time("NOT VALID", &hours, &minutes));
}

TEST(TogglApiTest, toggl_format_duration_in_seconds_hhmmss) {
    char *str = toggl_format_duration_in_seconds_hhmmss(10);
    ASSERT_EQ("00:00:10", std::string(str));
    free(str);

    str = toggl_format_duration_in_seconds_hhmmss(60);
    ASSERT_EQ("00:01:00", std::string(str));
    free(str);

    str = toggl_format_duration_in_seconds_hhmmss(65);
    ASSERT_EQ("00:01:05", std::string(str));
    free(str);

    str = toggl_format_duration_in_seconds_hhmmss(3600);
    ASSERT_EQ("01:00:00", std::string(str));
    free(str);

    str = toggl_format_duration_in_seconds_hhmmss(5400);
    ASSERT_EQ("01:30:00", std::string(str));
    free(str);

    str = toggl_format_duration_in_seconds_hhmmss(5410);
    ASSERT_EQ("01:30:10", std::string(str));
    free(str);
}

TEST(TogglApiTest, toggl_format_duration_in_seconds_hhmm) {
    char *str  = toggl_format_duration_in_seconds_hhmm(10);
    ASSERT_EQ("00:00", std::string(str));
    free(str);

    str = toggl_format_duration_in_seconds_hhmm(60);
    ASSERT_EQ("00:01", std::string(str));
    free(str);

    str = toggl_format_duration_in_seconds_hhmm(65);
    ASSERT_EQ("00:01", std::string(str));
    free(str);

    str = toggl_format_duration_in_seconds_hhmm(3600);
    ASSERT_EQ("01:00", std::string(str));
    free(str);

    str = toggl_format_duration_in_seconds_hhmm(5400);
    ASSERT_EQ("01:30", std::string(str));
    free(str);

    str = toggl_format_duration_in_seconds_hhmm(5410);
    ASSERT_EQ("01:30", std::string(str));
    free(str);
}

TEST(TogglApiTest, toggl_parse_duration_string_into_seconds) {
    time_t res = toggl_parse_duration_string_into_seconds("");
    ASSERT_EQ(0, res);
}

}  // namespace toggl
