// Copyright 2014 Toggl Desktop developers.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "./kopsik_api.h"
#include "./https_client.h"
#include "./test_data.h"

#include "Poco/FileStream.h"
#include "Poco/File.h"

namespace kopsik {

void in_test_change_callback(
    KopsikModelChange *change) {
}

void in_test_on_error_callback(
    const char *errmsg) {
}

void in_test_check_updates_callback(
    const _Bool is_update_available,
    const char *url,
    const char *version) {
}

void in_test_online_callback() {}

unsigned int g_user_id = 0;
std::string g_user_fullname = "";
std::string g_user_timeofdayformat = "";

void in_test_user_login_callback(
    const uint64_t id,
    const char *fullname,
    const char *timeofdayformat) {

    g_user_id = id;
    g_user_fullname = std::string(fullname);
    g_user_timeofdayformat = std::string(timeofdayformat);
}

void *create_test_context() {
    return kopsik_context_init("tests", "0.1",
                               in_test_change_callback,
                               in_test_on_error_callback,
                               in_test_check_updates_callback,
                               in_test_online_callback,
                               in_test_user_login_callback);
}

void wipe_test_db() {
    Poco::File f(TESTDB);
    if (f.exists()) {
        f.remove(false);
    }
}

TEST(KopsikApiTest, kopsik_context_init) {
    void *ctx = create_test_context();
    ASSERT_TRUE(ctx);
    kopsik_context_clear(ctx);
}

TEST(KopsikApiTest, kopsik_set_settings) {
    void *ctx = create_test_context();
    wipe_test_db();

    ASSERT_TRUE(kopsik_set_db_path(ctx, TESTDB));

    ASSERT_TRUE(kopsik_set_settings(ctx,
                                    false, false, false));

    _Bool idle_detection(false), menubar_timer(false), dock_icon(false);

    ASSERT_TRUE(kopsik_get_settings(
        ctx, &idle_detection, &menubar_timer, &dock_icon));

    ASSERT_FALSE(idle_detection);
    ASSERT_FALSE(menubar_timer);
    ASSERT_FALSE(dock_icon);

    ASSERT_TRUE(kopsik_set_settings(ctx, true, true, true));

    ASSERT_TRUE(kopsik_get_settings(
        ctx, &idle_detection, &menubar_timer, &dock_icon));

    ASSERT_TRUE(idle_detection);
    ASSERT_TRUE(menubar_timer);
    ASSERT_TRUE(dock_icon);
}

TEST(KopsikApiTest, kopsik_set_proxy_settings) {
    void *ctx = create_test_context();
    wipe_test_db();

    ASSERT_TRUE(kopsik_set_db_path(ctx, TESTDB));

    ASSERT_TRUE(kopsik_set_proxy_settings(
        ctx, 1, "localhost", 8000, "johnsmith", "secret"));

    _Bool use_proxy = false;
    char *host = 0;
    uint64_t port = 0;
    char *username = 0;
    char *password = 0;
    ASSERT_TRUE(kopsik_get_proxy_settings(
        ctx, &use_proxy, &host, &port, &username, &password));

    ASSERT_TRUE(use_proxy);
    ASSERT_EQ(std::string("localhost"), std::string(host));
    ASSERT_EQ(8000, static_cast<int>(port));
    ASSERT_EQ(std::string("johnsmith"), std::string(username));
    ASSERT_EQ(std::string("secret"), std::string(password));

    if (host) {
        free(host);
    }
    if (username) {
        free(username);
    }
    if (password) {
        free(password);
    }

    kopsik_context_clear(ctx);
}

TEST(KopsikApiTest, kopsik_set_update_channel) {
    void *ctx = create_test_context();
    wipe_test_db();

    ASSERT_TRUE(kopsik_set_db_path(ctx, TESTDB));

    char update_channel[10];

    std::string default_channel("stable");

    ASSERT_TRUE(kopsik_get_update_channel(ctx, update_channel, 10));
    ASSERT_EQ(default_channel, std::string(update_channel));

    ASSERT_FALSE(kopsik_set_update_channel(ctx, "invalid"));

    ASSERT_TRUE(kopsik_set_update_channel(ctx, "beta"));

    ASSERT_TRUE(kopsik_get_update_channel(ctx, update_channel, 10));
    ASSERT_EQ(std::string("beta"), std::string(update_channel));

    ASSERT_TRUE(kopsik_set_update_channel(ctx, "dev"));

    ASSERT_TRUE(kopsik_get_update_channel(ctx, update_channel, 10));
    ASSERT_EQ(std::string("dev"), std::string(update_channel));

    ASSERT_TRUE(kopsik_set_update_channel(ctx, "stable"));

    ASSERT_TRUE(kopsik_get_update_channel(ctx, update_channel, 10));
    ASSERT_EQ(std::string("stable"), std::string(update_channel));

    kopsik_context_clear(ctx);
}

TEST(KopsikApiTest, kopsik_set_db_path) {
    void *ctx = create_test_context();
    wipe_test_db();

    ASSERT_TRUE(kopsik_set_db_path(ctx, TESTDB));

    kopsik_context_clear(ctx);
    Poco::File f(TESTDB);
    ASSERT_TRUE(f.exists());
}

TEST(KopsikApiTest, kopsik_set_log_path) {
    kopsik_set_log_path("test.log");
}

TEST(KopsikApiTest, kopsik_set_log_level) {
    kopsik_set_log_level("trace");
}

TEST(KopsikApiTest, kopsik_set_api_token) {
    void *ctx = create_test_context();
    wipe_test_db();

    ASSERT_TRUE(kopsik_set_db_path(ctx, TESTDB));

    ASSERT_TRUE(kopsik_set_api_token(ctx, "token"));
    const int kMaxStrLen = 10;
    char str[kMaxStrLen];
    ASSERT_TRUE(kopsik_get_api_token(ctx, str, kMaxStrLen));
    ASSERT_EQ("token", std::string(str));
    kopsik_context_clear(ctx);
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

TEST(KopsikApiTest, kopsik_lifecycle) {
    void *ctx = create_test_context();

    wipe_test_db();

    ASSERT_TRUE(kopsik_set_db_path(ctx, TESTDB));

    std::string json = loadTestData();

    ASSERT_TRUE(kopsik_set_logged_in_user(ctx, json.c_str()));

    // We should have the API token now
    const int kMaxStrLen = 100;
    char str[kMaxStrLen];
    ASSERT_TRUE(kopsik_get_api_token(ctx, str, kMaxStrLen));
    ASSERT_EQ("30eb0ae954b536d2f6628f7fec47beb6", std::string(str));

    // We should have current user now
    ASSERT_EQ((unsigned int)10471231, g_user_id);
    ASSERT_EQ(std::string("John Smith"), g_user_fullname);
    ASSERT_EQ(std::string("H:mm"), g_user_timeofdayformat);

    // Count time entry items before start. It should be 3, since
    // there are 3 time entries in the me.json file we're using:
    KopsikTimeEntryViewItem *first = 0;
    ASSERT_TRUE(kopsik_time_entry_view_items(ctx, &first));
    int number_of_items = list_length(first);
    ASSERT_EQ(5, number_of_items);
    kopsik_time_entry_view_item_clear(first);

    // Start tracking
    KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
    ASSERT_TRUE(kopsik_start(ctx, "Test", 0, 0, 0, item));
    ASSERT_EQ(std::string("Test"), std::string(item->Description));
    kopsik_time_entry_view_item_clear(item);

    // We should now have a running time entry
    _Bool is_tracking = false;
    KopsikTimeEntryViewItem *running = kopsik_time_entry_view_item_init();
    ASSERT_TRUE(kopsik_running_time_entry_view_item(
        ctx, running, &is_tracking));
    ASSERT_TRUE(is_tracking);
    ASSERT_GT(0, running->DurationInSeconds);
    ASSERT_EQ(std::string("Test"), std::string(running->Description));
    ASSERT_EQ(std::string(""), std::string(running->ProjectAndTaskLabel));
    ASSERT_TRUE(running->Duration);
    ASSERT_TRUE(running->GUID);
    std::string GUID(running->GUID);
    kopsik_time_entry_view_item_clear(running);

    // The running time entry should *not* be listed
    // among time entry view items.
    first = 0;
    ASSERT_TRUE(kopsik_time_entry_view_items(
        ctx, &first));
    ASSERT_TRUE(first);
    ASSERT_EQ((unsigned int)number_of_items + 0, list_length(first));
    kopsik_time_entry_view_item_clear(first);

    // Set a new duration for the time entry.
    // It should keep on tracking and also the duration should change.
    ASSERT_TRUE(kopsik_set_time_entry_duration(
        ctx, GUID.c_str(), "1 hour"));
    is_tracking = false;
    running = kopsik_time_entry_view_item_init();
    ASSERT_TRUE(kopsik_running_time_entry_view_item(
        ctx, running, &is_tracking));
    ASSERT_TRUE(is_tracking);
    ASSERT_EQ("01:00:00", std::string(running->Duration));
    kopsik_time_entry_view_item_clear(running);

    // Set a new start time for the time entry.
    // Set it to a certain point in the past.
    // The duration should change accordingly to be now - start.
    ASSERT_TRUE(kopsik_set_time_entry_start_iso_8601(ctx,
                GUID.c_str(), "2013-11-28T13:15:30Z"));
    running = kopsik_time_entry_view_item_init();
    ASSERT_TRUE(kopsik_running_time_entry_view_item(
        ctx, running, &is_tracking));
    ASSERT_TRUE(is_tracking);
    ASSERT_EQ((unsigned int)1385644530, running->Started);
    ASSERT_NE("01:00:00", std::string(running->Duration));
    ASSERT_EQ(-1385644530, running->DurationInSeconds);
    kopsik_time_entry_view_item_clear(running);

    // Stop the time entry
    KopsikTimeEntryViewItem *stopped = kopsik_time_entry_view_item_init();
    _Bool was_stopped = false;
    ASSERT_TRUE(kopsik_stop(ctx, stopped, &was_stopped));
    ASSERT_TRUE(was_stopped);
    ASSERT_EQ(std::string("Test"), std::string(stopped->Description));
    std::string dirty_guid(stopped->GUID);
    kopsik_time_entry_view_item_clear(stopped);

    // Change duration of the stopped time entry to 1 hour.
    // Check it was really applied.
    ASSERT_TRUE(kopsik_set_time_entry_duration(
        ctx, dirty_guid.c_str(), "2,5 hours"));
    stopped = kopsik_time_entry_view_item_init();
    _Bool was_found = false;
    ASSERT_TRUE(kopsik_time_entry_view_item_by_guid(
        ctx, dirty_guid.c_str(), stopped, &was_found));
    ASSERT_TRUE(was_found);
    ASSERT_EQ("02:30:00", std::string(stopped->Duration));
    kopsik_time_entry_view_item_clear(stopped);

    // Set a new start time for the stopped entry.
    ASSERT_TRUE(kopsik_set_time_entry_start_iso_8601(ctx,
                dirty_guid.c_str(), "2013-11-27T12:30:00Z"));
    stopped = kopsik_time_entry_view_item_init();
    was_found = false;
    ASSERT_TRUE(kopsik_time_entry_view_item_by_guid(
        ctx, dirty_guid.c_str(), stopped, &was_found));
    ASSERT_TRUE(was_found);
    ASSERT_EQ((unsigned int)1385555400, stopped->Started);
    ASSERT_EQ("02:30:00", std::string(stopped->Duration));
    ASSERT_EQ(stopped->Ended, stopped->Started + 9000);
    kopsik_time_entry_view_item_clear(stopped);

    // Set a new end time for the stopped entry.
    // Check that the duration changes.
    ASSERT_TRUE(kopsik_set_time_entry_end_iso_8601(ctx,
                dirty_guid.c_str(), "2013-11-27T13:30:00Z"));
    stopped = kopsik_time_entry_view_item_init();
    was_found = false;
    ASSERT_TRUE(kopsik_time_entry_view_item_by_guid(
        ctx, dirty_guid.c_str(), stopped, &was_found));
    ASSERT_TRUE(was_found);
    ASSERT_EQ((unsigned int)1385555400, stopped->Started);
    ASSERT_EQ((unsigned int)1385559000, stopped->Ended);
    ASSERT_EQ("01:00:00", std::string(stopped->Duration));
    unsigned int started = stopped->Started;
    unsigned int ended = stopped->Ended;
    kopsik_time_entry_view_item_clear(stopped);

    // Change duration of the stopped time entry.
    // Start time should be the same, but end time should change.
    ASSERT_TRUE(kopsik_set_time_entry_duration(
        ctx, dirty_guid.c_str(), "2 hours"));
    stopped = kopsik_time_entry_view_item_init();
    was_found = false;
    ASSERT_TRUE(kopsik_time_entry_view_item_by_guid(
        ctx, dirty_guid.c_str(), stopped, &was_found));
    ASSERT_TRUE(was_found);
    ASSERT_EQ("02:00:00", std::string(stopped->Duration));
    ASSERT_EQ(started, stopped->Started);
    ASSERT_NE(ended, stopped->Ended);
    ASSERT_EQ(stopped->Started + 3600*2, stopped->Ended);
    kopsik_time_entry_view_item_clear(stopped);

    // Now the stopped time entry should be listed
    // among time entry view items.
    first = 0;
    ASSERT_TRUE(kopsik_time_entry_view_items(
        ctx, &first));
    ASSERT_TRUE(first);
    ASSERT_EQ((unsigned int)number_of_items + 1, list_length(first));
    kopsik_time_entry_view_item_clear(first);

    // We should no longer get a running time entry from API.
    is_tracking = false;
    running = kopsik_time_entry_view_item_init();
    ASSERT_TRUE(kopsik_running_time_entry_view_item(
        ctx, running, &is_tracking));
    ASSERT_FALSE(is_tracking);
    kopsik_time_entry_view_item_clear(running);

    // Continue the time entry we created in the start.
    KopsikTimeEntryViewItem *continued =
        kopsik_time_entry_view_item_init();
    ASSERT_TRUE(kopsik_continue(
        ctx, GUID.c_str(), continued));
    ASSERT_NE(std::string(GUID), std::string(continued->GUID));
    ASSERT_FALSE(std::string(continued->Duration).empty());
    ASSERT_GT(0, continued->DurationInSeconds);
    kopsik_time_entry_view_item_clear(continued);

    // Get time entry view using GUID
    was_found = false;
    KopsikTimeEntryViewItem *found = kopsik_time_entry_view_item_init();
    ASSERT_TRUE(kopsik_time_entry_view_item_by_guid(
        ctx, dirty_guid.c_str(), found, &was_found));
    ASSERT_EQ(dirty_guid, std::string(found->GUID));
    ASSERT_TRUE(was_found);
    kopsik_time_entry_view_item_clear(found);

    // Ask for a non-existant time entry
    KopsikTimeEntryViewItem *nonexistant =
        kopsik_time_entry_view_item_init();
    ASSERT_TRUE(kopsik_time_entry_view_item_by_guid(
        ctx, "bad guid", nonexistant, &was_found));
    ASSERT_FALSE(nonexistant->GUID);
    ASSERT_FALSE(was_found);
    kopsik_time_entry_view_item_clear(nonexistant);

    // Delete the time entry we created in the start.
    ASSERT_TRUE(kopsik_delete_time_entry(
        ctx, GUID.c_str()));

    // We shouldnt be able to retrieve this time entry now in list.
    KopsikTimeEntryViewItem *visible = 0;
    ASSERT_TRUE(kopsik_time_entry_view_items(
        ctx, &visible));
    KopsikTimeEntryViewItem *it = visible;
    while (it) {
        ASSERT_FALSE(std::string(it->GUID) == GUID);
        it = reinterpret_cast<KopsikTimeEntryViewItem *>(it->Next);
    }
    kopsik_time_entry_view_item_clear(visible);

    // Log out
    ASSERT_TRUE(kopsik_logout(ctx));

    // Check that we have no API token after user logged out.
    ASSERT_TRUE(kopsik_get_api_token(ctx, str, kMaxStrLen));
    ASSERT_EQ("", std::string(str));

    kopsik_context_clear(ctx);
}

TEST(KopsikApiTest, kopsik_time_entry_view_item_init) {
    KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
    ASSERT_TRUE(te);
    kopsik_time_entry_view_item_clear(te);
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
