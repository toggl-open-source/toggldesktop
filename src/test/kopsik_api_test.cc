// Copyright 2014 Toggl Desktop developers.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <iostream>  // NOLINT

#include "./../kopsik_api.h"
#include "./test_data.h"
#include "./../settings.h"
#include "./../proxy.h"

#include "Poco/FileStream.h"
#include "Poco/File.h"

namespace kopsik {

namespace testing {

namespace testresult {
std::string url("");
std::string reminder_title("");
std::string reminder_informative_text("");
std::string error("");
bool online_state(false);
kopsik::Settings settings;
bool login(false);
_Bool use_proxy;
kopsik::Proxy proxy;
_Bool is_tracking;
}  // namespace testresult

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
    const _Bool is_update_available,
    const char *url,
    const char *version) {
}

void on_online_state(const _Bool is_online) {
    testresult::online_state = is_online;
}

void on_url(const char *url) {
    testresult::url = std::string(url);
}

void on_login() {
    testresult::login = true;
}

void on_reminder(const char *title, const char *informative_text) {
    testresult::reminder_title = std::string(title);
    testresult::reminder_informative_text = std::string(informative_text);
}

void on_time_entry_list(KopsikTimeEntryViewItem *first) {
}

void on_autocomplete(KopsikAutocompleteItem *first) {
}

void on_client_select(KopsikViewItem *first) {
}

void on_workspace_select(KopsikViewItem *first) {
}

void on_tags(KopsikViewItem *first) {
}

void on_time_entry_editor(
    KopsikTimeEntryViewItem *te,
    const char *focused_field_name) {
}

void on_display_settings(
    const _Bool use_idle_detection,
    const _Bool menubar_timer,
    const _Bool dock_icon,
    const _Bool on_top,
    const _Bool reminder) {
    testresult::settings.use_idle_detection = use_idle_detection;
    testresult::settings.menubar_timer = menubar_timer;
    testresult::settings.dock_icon = dock_icon;
    testresult::settings.on_top = on_top;
    testresult::settings.reminder = reminder;
}

void on_display_proxy_settings(
    const _Bool use_proxy,
    const char *proxy_host,
    const uint64_t proxy_port,
    const char *proxy_username,
    const char *proxy_password) {
    testresult::use_proxy = use_proxy;
    testresult::proxy.host = std::string(proxy_host);
    testresult::proxy.port = proxy_port;
    testresult::proxy.username = std::string(proxy_username);
    testresult::proxy.password = std::string(proxy_password);
}

void on_display_timer_state(const _Bool is_tracking,
                            KopsikTimeEntryViewItem *te) {
    testresult::is_tracking = is_tracking;
}

class App {
 public:
    explicit App() {
        Poco::File f(TESTDB);
        if (f.exists()) {
            f.remove(false);
        }

        kopsik_set_log_path("test.log");

        ctx_ = kopsik_context_init("tests", "0.1");

        poco_assert(kopsik_set_db_path(ctx_, TESTDB));

        kopsik_on_error(ctx_, on_error);
        kopsik_on_update(ctx_, on_update);
        kopsik_on_online_state(ctx_, on_online_state);
        kopsik_on_login(ctx_, on_login);
        kopsik_on_url(ctx_, on_url);
        kopsik_on_reminder(ctx_, on_reminder);
        kopsik_on_time_entry_list(ctx_, on_time_entry_list);
        kopsik_on_autocomplete(ctx_, on_autocomplete);
        kopsik_on_workspace_select(ctx_, on_workspace_select);
        kopsik_on_client_select(ctx_, on_client_select);
        kopsik_on_tags(ctx_, on_tags);
        kopsik_on_time_entry_editor(ctx_, on_time_entry_editor);
        kopsik_on_settings(ctx_, on_display_settings);
        kopsik_on_proxy_settings(ctx_, on_display_proxy_settings);
        kopsik_on_timer_state(ctx_, on_display_timer_state);
    }
    ~App() {
        kopsik_context_clear(ctx_);
        ctx_ = 0;
    }

    void *ctx() {
        return ctx_;
    }

 private:
    void *ctx_;
};

};  // namespace testing

TEST(KopsikApiTest, kopsik_context_init) {
    testing::App app;
    ASSERT_TRUE(kopsik_context_start_events(app.ctx()));
}

TEST(KopsikApiTest, kopsik_set_settings) {
    testing::App app;

    ASSERT_TRUE(kopsik_set_settings(app.ctx(),
                                    false, false, false, false, false));

    ASSERT_FALSE(testing::testresult::settings.use_idle_detection);
    ASSERT_FALSE(testing::testresult::settings.menubar_timer);
    ASSERT_FALSE(testing::testresult::settings.dock_icon);
    ASSERT_FALSE(testing::testresult::settings.on_top);
    ASSERT_FALSE(testing::testresult::settings.reminder);

    ASSERT_TRUE(kopsik_set_settings(app.ctx(), true, true, true, true, true));

    ASSERT_TRUE(testing::testresult::settings.use_idle_detection);
    ASSERT_TRUE(testing::testresult::settings.menubar_timer);
    ASSERT_TRUE(testing::testresult::settings.dock_icon);
    ASSERT_TRUE(testing::testresult::settings.on_top);
    ASSERT_TRUE(testing::testresult::settings.reminder);
}

TEST(KopsikApiTest, kopsik_set_proxy_settings) {
    testing::App app;

    ASSERT_TRUE(kopsik_set_proxy_settings(
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

TEST(KopsikApiTest, kopsik_set_update_channel) {
    testing::App app;

    char update_channel[10];

    std::string default_channel("stable");

    ASSERT_TRUE(kopsik_get_update_channel(app.ctx(), update_channel, 10));
    ASSERT_EQ(default_channel, std::string(update_channel));

    ASSERT_FALSE(kopsik_set_update_channel(app.ctx(), "invalid"));

    ASSERT_TRUE(kopsik_set_update_channel(app.ctx(), "beta"));

    ASSERT_TRUE(kopsik_get_update_channel(app.ctx(), update_channel, 10));
    ASSERT_EQ(std::string("beta"), std::string(update_channel));

    ASSERT_TRUE(kopsik_set_update_channel(app.ctx(), "dev"));

    ASSERT_TRUE(kopsik_get_update_channel(app.ctx(), update_channel, 10));
    ASSERT_EQ(std::string("dev"), std::string(update_channel));

    ASSERT_TRUE(kopsik_set_update_channel(app.ctx(), "stable"));

    ASSERT_TRUE(kopsik_get_update_channel(app.ctx(), update_channel, 10));
    ASSERT_EQ(std::string("stable"), std::string(update_channel));
}

TEST(KopsikApiTest, kopsik_set_log_level) {
    kopsik_set_log_level("trace");
}

TEST(KopsikApiTest, kopsik_set_api_token) {
    testing::App app;
    ASSERT_TRUE(kopsik_set_api_token(app.ctx(), "token"));
    const int kMaxStrLen = 10;
    char str[kMaxStrLen];
    ASSERT_TRUE(kopsik_get_api_token(app.ctx(), str, kMaxStrLen));
    ASSERT_EQ("token", std::string(str));
}

unsigned int list_length(KopsikTimeEntryViewItem *first) {
    unsigned int n = 0;
    KopsikTimeEntryViewItem *it = first;
    while (it) {
        n++;
        it = reinterpret_cast<KopsikTimeEntryViewItem *>(it->Next);
    }
    return n;
}

TEST(KopsikApiTest, kopsik_set_logged_in_user) {
    testing::App app;
    ASSERT_TRUE(kopsik_set_logged_in_user(app.ctx(), loadTestData().c_str()));
}

TEST(KopsikApiTest, kopsik_lifecycle) {
    testing::App app;

    std::string json = loadTestData();

    testing::testresult::error = "";

    ASSERT_TRUE(kopsik_set_logged_in_user(app.ctx(), json.c_str()));

    ASSERT_EQ("", testing::testresult::error);

    // We should have the API token now
    const int kMaxStrLen = 100;
    char str[kMaxStrLen];
    ASSERT_TRUE(kopsik_get_api_token(app.ctx(), str, kMaxStrLen));
    ASSERT_EQ("30eb0ae954b536d2f6628f7fec47beb6", std::string(str));

    ASSERT_EQ("", testing::testresult::error);

    // Start tracking
    ASSERT_TRUE(kopsik_start(app.ctx(), "Test", 0, 0, 0));

    ASSERT_EQ("", testing::testresult::error);

    // Stop the time entry
    ASSERT_TRUE(kopsik_stop(app.ctx()));

    ASSERT_EQ("", testing::testresult::error);

    // Log out
    ASSERT_TRUE(kopsik_logout(app.ctx()));

    // Check that we have no API token after user logged out.
    ASSERT_TRUE(kopsik_get_api_token(app.ctx(), str, kMaxStrLen));
    ASSERT_EQ("", std::string(str));
}

TEST(KopsikApiTest, kopsik_parse_time) {
    int hours = 0;
    int minutes = 0;
    bool valid = true;

    valid = kopsik_parse_time("120a", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(1, hours);
    ASSERT_EQ(20, minutes);

    valid = kopsik_parse_time("1P", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(13, hours);
    ASSERT_EQ(0, minutes);

    valid = kopsik_parse_time("1230", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(12, hours);
    ASSERT_EQ(30, minutes);

    valid = kopsik_parse_time("11:20", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(11, hours);
    ASSERT_EQ(20, minutes);

    valid = kopsik_parse_time("5:30 PM", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(17, hours);
    ASSERT_EQ(30, minutes);

    valid = kopsik_parse_time("17:10", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(17, hours);
    ASSERT_EQ(10, minutes);

    valid = kopsik_parse_time("12:00 AM", &hours, &minutes);
    ASSERT_EQ(true, valid);
    ASSERT_EQ(0, hours);
    ASSERT_EQ(0, minutes);

    valid = kopsik_parse_time("NOT VALID", &hours, &minutes);
    ASSERT_EQ(false, valid);
}

TEST(KopsikApiTest, kopsik_format_duration_in_seconds_hhmmss) {
    const int kMaxStrLen = 100;
    char str[kMaxStrLen];
    kopsik_format_duration_in_seconds_hhmmss(10, str, kMaxStrLen);
    ASSERT_EQ("00:00:10", std::string(str));
    kopsik_format_duration_in_seconds_hhmmss(60, str, kMaxStrLen);
    ASSERT_EQ("00:01:00", std::string(str));
    kopsik_format_duration_in_seconds_hhmmss(65, str, kMaxStrLen);
    ASSERT_EQ("00:01:05", std::string(str));
    kopsik_format_duration_in_seconds_hhmmss(3600, str, kMaxStrLen);
    ASSERT_EQ("01:00:00", std::string(str));
    kopsik_format_duration_in_seconds_hhmmss(5400, str, kMaxStrLen);
    ASSERT_EQ("01:30:00", std::string(str));
    kopsik_format_duration_in_seconds_hhmmss(5410, str, kMaxStrLen);
    ASSERT_EQ("01:30:10", std::string(str));
}

TEST(KopsikApiTest, kopsik_format_duration_in_seconds_hhmm) {
    const int kMaxStrLen = 100;
    char str[kMaxStrLen];
    kopsik_format_duration_in_seconds_hhmm(10, str, kMaxStrLen);
    ASSERT_EQ("00:00", std::string(str));
    kopsik_format_duration_in_seconds_hhmm(60, str, kMaxStrLen);
    ASSERT_EQ("00:01", std::string(str));
    kopsik_format_duration_in_seconds_hhmm(65, str, kMaxStrLen);
    ASSERT_EQ("00:01", std::string(str));
    kopsik_format_duration_in_seconds_hhmm(3600, str, kMaxStrLen);
    ASSERT_EQ("01:00", std::string(str));
    kopsik_format_duration_in_seconds_hhmm(5400, str, kMaxStrLen);
    ASSERT_EQ("01:30", std::string(str));
    kopsik_format_duration_in_seconds_hhmm(5410, str, kMaxStrLen);
    ASSERT_EQ("01:30", std::string(str));
}

}  // namespace kopsik
