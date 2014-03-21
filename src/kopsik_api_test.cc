// Copyright 2014 Toggl Desktop developers.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "./kopsik_api.h"
#include "./https_client.h"
#include "./test_data.h"

#include "Poco/FileStream.h"
#include "Poco/File.h"

const int ERRLEN = 1024;

namespace kopsik {

    void in_test_change_callback(
        kopsik_api_result result,
        const char *errmsg,
        KopsikModelChange *change) {
    }

    void in_test_on_error_callback(
        const char *errmsg) {
    }

    void in_test_check_updates_callback(
        kopsik_api_result result,
        const char *errmsg,
        const int is_update_available,
        const char *url,
        const char *version) {
    }

    void in_test_online_callback() {}

    void *create_test_context() {
        return kopsik_context_init("tests", "0.1",
            in_test_change_callback,
            in_test_on_error_callback,
            in_test_check_updates_callback,
            in_test_online_callback);
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
      char err[ERRLEN];

      ASSERT_EQ(KOPSIK_API_SUCCESS,
        kopsik_set_db_path(ctx, err, ERRLEN, TESTDB));

      ASSERT_EQ(KOPSIK_API_SUCCESS,
        kopsik_set_settings(ctx, err, ERRLEN, 0, 0, 0));

      unsigned int idle_detection(0), menubar_timer(0), dock_icon(0);

      ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_get_settings(
        ctx, err, ERRLEN, &idle_detection, &menubar_timer, &dock_icon));

      ASSERT_FALSE(idle_detection);
      ASSERT_FALSE(menubar_timer);
      ASSERT_FALSE(dock_icon);

      ASSERT_EQ(KOPSIK_API_SUCCESS,
        kopsik_set_settings(ctx, err, ERRLEN, 1, 1, 1));

      ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_get_settings(
        ctx, err, ERRLEN, &idle_detection, &menubar_timer, &dock_icon));

      ASSERT_TRUE(idle_detection);
      ASSERT_TRUE(menubar_timer);
      ASSERT_TRUE(dock_icon);
    }

    TEST(KopsikApiTest, kopsik_set_proxy_settings) {
      void *ctx = create_test_context();
      wipe_test_db();
      char err[ERRLEN];

      ASSERT_EQ(KOPSIK_API_SUCCESS,
        kopsik_set_db_path(ctx, err, ERRLEN, TESTDB));

      ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_set_proxy_settings(
        ctx, err, ERRLEN, 1, "localhost", 8000, "johnsmith", "secret"));

      unsigned int use_proxy = 0;
      char *host = 0;
      unsigned int port = 0;
      char *username = 0;
      char *password = 0;
      ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_get_proxy_settings(
        ctx, err, ERRLEN, &use_proxy, &host, &port, &username, &password));

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

        char err[ERRLEN];
        kopsik_api_result res = kopsik_set_db_path(ctx, err, ERRLEN, TESTDB);
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);

        char update_channel[10];

        std::string default_channel("stable");

        res = kopsik_get_update_channel(ctx, err, ERRLEN, update_channel, 10);
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);
        ASSERT_EQ(default_channel, std::string(update_channel));

        res = kopsik_set_update_channel(ctx, err, ERRLEN, "invalid");
        ASSERT_NE(KOPSIK_API_SUCCESS, res);

        res = kopsik_set_update_channel(ctx, err, ERRLEN, "beta");
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);

        res = kopsik_get_update_channel(ctx, err, ERRLEN, update_channel, 10);
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);
        ASSERT_EQ(std::string("beta"), std::string(update_channel));

        res = kopsik_set_update_channel(ctx, err, ERRLEN, "dev");
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);

        res = kopsik_get_update_channel(ctx, err, ERRLEN, update_channel, 10);
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);
        ASSERT_EQ(std::string("dev"), std::string(update_channel));

        res = kopsik_set_update_channel(ctx, err, ERRLEN, "stable");
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);

        res = kopsik_get_update_channel(ctx, err, ERRLEN, update_channel, 10);
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);
        ASSERT_EQ(std::string("stable"), std::string(update_channel));

        kopsik_context_clear(ctx);
    }

    TEST(KopsikApiTest, kopsik_set_db_path) {
        void *ctx = create_test_context();
        wipe_test_db();

        char err[ERRLEN];
        kopsik_api_result res = kopsik_set_db_path(ctx, err, ERRLEN, TESTDB);
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);

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

    TEST(KopsikApiTest, kopsik_user_init) {
        KopsikUser *user = kopsik_user_init();
        ASSERT_TRUE(user);
        kopsik_user_clear(user);
    }

    TEST(KopsikApiTest, kopsik_set_api_token) {
        void *ctx = create_test_context();
        wipe_test_db();

        char err[ERRLEN];
        kopsik_api_result res = kopsik_set_db_path(ctx, err, ERRLEN, TESTDB);
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);

        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_set_api_token(ctx, err, ERRLEN, "token"));
        const int kMaxStrLen = 10;
        char str[kMaxStrLen];
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_get_api_token(ctx, err, ERRLEN, str, kMaxStrLen));
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

        char err[ERRLEN];
        kopsik_api_result res = kopsik_set_db_path(ctx, err, ERRLEN, TESTDB);
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);

        std::string json = loadTestData();

        res = kopsik_set_logged_in_user(ctx, err, ERRLEN, json.c_str());
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);

        // We should have the API token now
        const int kMaxStrLen = 100;
        char str[kMaxStrLen];
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_get_api_token(ctx, err, ERRLEN, str, kMaxStrLen));
        ASSERT_EQ("30eb0ae954b536d2f6628f7fec47beb6", std::string(str));

        // We should have current user now
        KopsikUser *user = kopsik_user_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_current_user(ctx, err, ERRLEN, user));
        ASSERT_EQ((unsigned int)10471231, user->ID);
        ASSERT_EQ(std::string("John Smith"), std::string(user->Fullname));
        kopsik_user_clear(user);

        // Count time entry items before start. It should be 3, since
        // there are 3 time entries in the me.json file we're using:
        KopsikTimeEntryViewItem *first = 0;
        if (KOPSIK_API_SUCCESS != kopsik_time_entry_view_items(
                ctx, err, ERRLEN, &first)) {
            ASSERT_EQ(std::string(""), std::string(err));
            FAIL();
        }
        int number_of_items = list_length(first);
        ASSERT_EQ(5, number_of_items);
        kopsik_time_entry_view_item_clear(first);

        // Start tracking
        KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_start(ctx, err, ERRLEN, "Test", 0, 0, 0, item));
        ASSERT_EQ(std::string("Test"), std::string(item->Description));
        kopsik_time_entry_view_item_clear(item);

        // We should now have a running time entry
        int is_tracking = 0;
        KopsikTimeEntryViewItem *running = kopsik_time_entry_view_item_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_running_time_entry_view_item(
            ctx, err, ERRLEN, running, &is_tracking));
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
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_items(
            ctx, err, ERRLEN, &first));
        ASSERT_TRUE(first);
        ASSERT_EQ((unsigned int)number_of_items + 0, list_length(first));
        kopsik_time_entry_view_item_clear(first);

        // Set a new duration for the time entry.
        // It should keep on tracking and also the duration should change.
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_set_time_entry_duration(
            ctx, err, ERRLEN, GUID.c_str(), "1 hour"));
        is_tracking = 0;
        running = kopsik_time_entry_view_item_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_running_time_entry_view_item(
            ctx, err, ERRLEN, running, &is_tracking));
        ASSERT_TRUE(is_tracking);
        ASSERT_EQ("01:00:00", std::string(running->Duration));
        kopsik_time_entry_view_item_clear(running);

        // Set a new start time for the time entry.
        // Set it to a certain point in the past.
        // The duration should change accordingly to be now - start.
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_set_time_entry_start_iso_8601(ctx,
            err, ERRLEN, GUID.c_str(), "2013-11-28T13:15:30Z"));
        running = kopsik_time_entry_view_item_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_running_time_entry_view_item(
            ctx, err, ERRLEN, running, &is_tracking));
        ASSERT_TRUE(is_tracking);
        ASSERT_EQ((unsigned int)1385644530, running->Started);
        ASSERT_NE("01:00:00", std::string(running->Duration));
        ASSERT_EQ(-1385644530, running->DurationInSeconds);
        kopsik_time_entry_view_item_clear(running);

        // Stop the time entry
        KopsikTimeEntryViewItem *stopped = kopsik_time_entry_view_item_init();
        int was_stopped = 0;
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_stop(ctx, err, ERRLEN, stopped, &was_stopped));
        ASSERT_EQ(1, was_stopped);
        ASSERT_EQ(std::string("Test"), std::string(stopped->Description));
        std::string dirty_guid(stopped->GUID);
        kopsik_time_entry_view_item_clear(stopped);

        // Change duration of the stopped time entry to 1 hour.
        // Check it was really applied.
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_set_time_entry_duration(
            ctx, err, ERRLEN, dirty_guid.c_str(), "2,5 hours"));
        stopped = kopsik_time_entry_view_item_init();
        int was_found = 0;
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_item_by_guid(
            ctx, err, ERRLEN, dirty_guid.c_str(), stopped, &was_found));
        ASSERT_TRUE(was_found);
        ASSERT_EQ("02:30:00", std::string(stopped->Duration));
        kopsik_time_entry_view_item_clear(stopped);

        // Set a new start time for the stopped entry.
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_set_time_entry_start_iso_8601(ctx,
            err, ERRLEN, dirty_guid.c_str(), "2013-11-27T12:30:00Z"));
        stopped = kopsik_time_entry_view_item_init();
        was_found = 0;
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_item_by_guid(
            ctx, err, ERRLEN, dirty_guid.c_str(), stopped, &was_found));
        ASSERT_TRUE(was_found);
        ASSERT_EQ((unsigned int)1385555400, stopped->Started);
        ASSERT_EQ("02:30:00", std::string(stopped->Duration));
        ASSERT_EQ(stopped->Ended, stopped->Started + 9000);
        kopsik_time_entry_view_item_clear(stopped);

        // Set a new end time for the stopped entry.
        // Check that the duration changes.
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_set_time_entry_end_iso_8601(ctx,
            err, ERRLEN, dirty_guid.c_str(), "2013-11-27T13:30:00Z"));
        stopped = kopsik_time_entry_view_item_init();
        was_found = 0;
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_item_by_guid(
            ctx, err, ERRLEN, dirty_guid.c_str(), stopped, &was_found));
        ASSERT_TRUE(was_found);
        ASSERT_EQ((unsigned int)1385555400, stopped->Started);
        ASSERT_EQ((unsigned int)1385559000, stopped->Ended);
        ASSERT_EQ("01:00:00", std::string(stopped->Duration));
        unsigned int started = stopped->Started;
        unsigned int ended = stopped->Ended;
        kopsik_time_entry_view_item_clear(stopped);

        // Change duration of the stopped time entry.
        // Start time should be the same, but end time should change.
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_set_time_entry_duration(
            ctx, err, ERRLEN, dirty_guid.c_str(), "2 hours"));
        stopped = kopsik_time_entry_view_item_init();
        was_found = 0;
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_item_by_guid(
            ctx, err, ERRLEN, dirty_guid.c_str(), stopped, &was_found));
        ASSERT_TRUE(was_found);
        ASSERT_EQ("02:00:00", std::string(stopped->Duration));
        ASSERT_EQ(started, stopped->Started);
        ASSERT_NE(ended, stopped->Ended);
        ASSERT_EQ(stopped->Started + 3600*2, stopped->Ended);
        kopsik_time_entry_view_item_clear(stopped);

        // Now the stopped time entry should be listed
        // among time entry view items.
        first = 0;
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_items(
            ctx, err, ERRLEN, &first));
        ASSERT_TRUE(first);
        ASSERT_EQ((unsigned int)number_of_items + 1, list_length(first));
        kopsik_time_entry_view_item_clear(first);

        // We should no longer get a running time entry from API.
        is_tracking = 0;
        running = kopsik_time_entry_view_item_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_running_time_entry_view_item(
            ctx, err, ERRLEN, running, &is_tracking));
        ASSERT_FALSE(is_tracking);
        kopsik_time_entry_view_item_clear(running);

        // We started and stopped one time entry.
        // This means we should have one dirty model now.
        KopsikPushableModelStats stats;
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_pushable_models(
            ctx, err, ERRLEN, &stats));
        ASSERT_EQ((unsigned int)1, stats.TimeEntries);

        // Continue the time entry we created in the start.
        KopsikTimeEntryViewItem *continued =
            kopsik_time_entry_view_item_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_continue(
            ctx, err, ERRLEN, GUID.c_str(), continued));
        ASSERT_NE(std::string(GUID), std::string(continued->GUID));
        ASSERT_FALSE(std::string(continued->Duration).empty());
        ASSERT_GT(0, continued->DurationInSeconds);
        kopsik_time_entry_view_item_clear(continued);

        // We should now once again have a dirty model.
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_pushable_models(
            ctx, err, ERRLEN, &stats));
        ASSERT_EQ((unsigned int)2, stats.TimeEntries);

        // Get time entry view using GUID
        was_found = 0;
        KopsikTimeEntryViewItem *found = kopsik_time_entry_view_item_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_item_by_guid(
            ctx, err, ERRLEN, dirty_guid.c_str(), found, &was_found));
        ASSERT_EQ(dirty_guid, std::string(found->GUID));
        ASSERT_TRUE(was_found);
        kopsik_time_entry_view_item_clear(found);

        // Ask for a non-existant time entry
        KopsikTimeEntryViewItem *nonexistant =
            kopsik_time_entry_view_item_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_item_by_guid(
            ctx, err, ERRLEN, "bad guid", nonexistant, &was_found));
        ASSERT_FALSE(nonexistant->GUID);
        ASSERT_FALSE(was_found);
        kopsik_time_entry_view_item_clear(nonexistant);

        // Delete the time entry we created in the start.
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_delete_time_entry(
            ctx, err, ERRLEN, GUID.c_str()));

        // We shouldnt be able to retrieve this time entry now in list.
        KopsikTimeEntryViewItem *visible = 0;
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_items(
            ctx, err, ERRLEN, &visible));
        KopsikTimeEntryViewItem *it = visible;
        while (it) {
            ASSERT_FALSE(std::string(it->GUID) == GUID);
            it = reinterpret_cast<KopsikTimeEntryViewItem *>(it->Next);
        }
        kopsik_time_entry_view_item_clear(visible);

        // Log out
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_logout(ctx, err, ERRLEN));

        // Check that we have no API token after user logged out.
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_get_api_token(ctx, err, ERRLEN, str, kMaxStrLen));
        ASSERT_EQ("", std::string(str));

        kopsik_context_clear(ctx);
    }

    TEST(KopsikApiTest, kopsik_time_entry_view_item_init) {
        KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
        ASSERT_TRUE(te);
        kopsik_time_entry_view_item_clear(te);
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
