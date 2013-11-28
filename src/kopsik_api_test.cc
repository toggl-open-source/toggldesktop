// Copyright 2013 Tanel Lebedev

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "./kopsik_api.h"
#include "./https_client.h"

#include "Poco/FileStream.h"
#include "Poco/File.h"

const char *TESTDB = "test.db";
const int ERRLEN = 1024;

namespace kopsik {

    class MockHTTPSClient : public HTTPSClient {
    public:
        MockHTTPSClient() :
            HTTPSClient("https://localhost:8080", "mock", "0.1") {}
        MOCK_METHOD0(ListenToWebsocket, kopsik::error());
        MOCK_METHOD5(PostJSON, kopsik::error(
            std::string relative_url,
            std::string json,
            std::string basic_auth_username,
            std::string basic_auth_password,
            std::string *response_body));
        MOCK_METHOD4(GetJSON, kopsik::error(
            std::string relative_url,
            std::string basic_auth_username,
            std::string basic_auth_password,
            std::string *response_body));
    };

    std::string meJSON() {
        Poco::FileStream fis("testdata/me.json", std::ios::binary);
        std::stringstream ss;
        ss << fis.rdbuf();
        fis.close();
        return ss.str();
    }

    void *create_test_context() {
        return kopsik_context_init("tests", "0.1");
    }

    TEST(KopsikApiTest, kopsik_context_init) {
        void *ctx = create_test_context();
        ASSERT_TRUE(ctx);
        kopsik_context_clear(ctx);
    }

    TEST(KopsikApiTest, kopsik_set_proxy) {
        void *ctx = create_test_context();
        {
            Poco::File f(TESTDB);
            if (f.exists()) f.remove(false);
        }
        char err[ERRLEN];
        kopsik_api_result res = kopsik_set_db_path(ctx, err, ERRLEN, TESTDB);
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);

        res = kopsik_set_proxy(
            ctx,
            err, ERRLEN,
            1, "localhost", 8000, "johnsmith", "secret");
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);

        KopsikProxySettings *settings =
            kopsik_proxy_settings_init();
        res = kopsik_get_proxy(ctx, err, ERRLEN, settings);
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);
        ASSERT_TRUE(settings->UseProxy);
        ASSERT_EQ(std::string("localhost"), std::string(settings->Host));
        ASSERT_EQ(8000, static_cast<int>(settings->Port));
        ASSERT_EQ(std::string("johnsmith"), std::string(settings->Username));
        ASSERT_EQ(std::string("secret"), std::string(settings->Password));
        kopsik_proxy_settings_clear(settings);

        kopsik_context_clear(ctx);
    }

    TEST(KopsikApiTest, kopsik_set_db_path) {
        void *ctx = create_test_context();
        {
            Poco::File f(TESTDB);
            if (f.exists()) f.remove(false);
        }
        char err[ERRLEN];
        kopsik_api_result res = kopsik_set_db_path(ctx, err, ERRLEN, TESTDB);
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);

        kopsik_context_clear(ctx);
        Poco::File f(TESTDB);
        ASSERT_TRUE(f.exists());
    }

    TEST(KopsikApiTest, kopsik_set_log_path) {
        void *ctx = create_test_context();
        kopsik_set_log_path(ctx, "test.log");
        ASSERT_TRUE(true);
        kopsik_context_clear(ctx);
    }

    TEST(KopsikApiTest, kopsik_user_init) {
        KopsikUser *user = kopsik_user_init();
        ASSERT_TRUE(user);
        kopsik_user_clear(user);
    }

    TEST(KopsikApiTest, kopsik_set_api_token) {
        void *ctx = create_test_context();
        Poco::File f(TESTDB);
        if (f.exists()) f.remove(false);

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

    TEST(KopsikApiTest, kopsik_lifecycle) {
        void *ctx = create_test_context();

        Poco::File f(TESTDB);
        if (f.exists()) f.remove(false);
        char err[ERRLEN];
        kopsik_api_result res = kopsik_set_db_path(ctx, err, ERRLEN, TESTDB);
        ASSERT_EQ(KOPSIK_API_SUCCESS, res);

        MockHTTPSClient *mock_client = new MockHTTPSClient();
        kopsik_test_set_https_client(ctx,
            reinterpret_cast<void *>(mock_client));

        std::string json = meJSON();

        // Login
        EXPECT_CALL(*mock_client, GetJSON(
            std::string("/api/v8/me?app_name=kopsik&with_related_data=true"),
            std::string("30eb0ae954b536d2f6628f7fec47beb6"),
            std::string("api_token"),
            testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<3>(json),
            testing::Return("")));

        EXPECT_CALL(*mock_client, GetJSON(
            std::string("/api/v8/me?app_name=kopsik&with_related_data=false"),
            std::string("foo@bar.com"),
            std::string("secret"),
            testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<3>(json),
            testing::Return("")));

        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_login(
            ctx,
            err, ERRLEN,
            "foo@bar.com", "secret"));

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
        KopsikTimeEntryViewItemList *list =
            kopsik_time_entry_view_item_list_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_items(
            ctx, err, ERRLEN, list));
        ASSERT_EQ((unsigned int)3, list->Length);
        int number_of_items = list->Length;
        kopsik_time_entry_view_item_list_clear(list);

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
        ASSERT_FALSE(running->Project);
        ASSERT_TRUE(running->Duration);
        ASSERT_FALSE(running->Color);
        ASSERT_TRUE(running->GUID);
        std::string GUID(running->GUID);
        kopsik_time_entry_view_item_clear(running);

        // The running time entry should *not* be listed
        // among time entry view items.
        list = kopsik_time_entry_view_item_list_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_items(
            ctx, err, ERRLEN, list));
        ASSERT_TRUE(list);
        ASSERT_EQ((unsigned int)number_of_items + 0, list->Length);
        kopsik_time_entry_view_item_list_clear(list);

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
        // Check that the duration does not change,
        // but end time changes instead.
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

        // Now the stopped time entry should be listed
        // among time entry view items.
        list = kopsik_time_entry_view_item_list_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_items(
            ctx, err, ERRLEN, list));
        ASSERT_TRUE(list);
        ASSERT_EQ((unsigned int)number_of_items + 1, list->Length);
        kopsik_time_entry_view_item_list_clear(list);

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

        // Push changes
        std::stringstream response_body;
        response_body
            << "["
            << "{"
            << "\"status\": 200,"
            << "\"guid\": \"" << dirty_guid << "\","
            << "\"content_type\": \"application/json\","
            << "\"body\": \""
                << "{\"data\": {\"id\": 123456789, \"ui_modified_at\": "
                << time(0) << "}}\""
            << "}"
            << "]";
        std::string response_json = response_body.str();
        EXPECT_CALL(*mock_client, PostJSON(
            std::string("/api/v8/batch_updates"),
            testing::An<std::string>(),
            std::string("30eb0ae954b536d2f6628f7fec47beb6"),
            std::string("api_token"),
            testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<4>(response_json),
            testing::Return("")));

        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_push(
            ctx, err, ERRLEN));

        // Check that no dirty models are left.
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_pushable_models(
            ctx, err, ERRLEN, &stats));
        ASSERT_EQ((unsigned int)0, stats.TimeEntries);

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
        ASSERT_EQ((unsigned int)1, stats.TimeEntries);

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
        KopsikTimeEntryViewItemList *visible =
            kopsik_time_entry_view_item_list_init();
        ASSERT_EQ(KOPSIK_API_SUCCESS, kopsik_time_entry_view_items(
            ctx, err, ERRLEN, visible));
        for (unsigned int i = 0; i < visible->Length; i++) {
            KopsikTimeEntryViewItem *n = visible->ViewItems[i];
            ASSERT_FALSE(std::string(n->GUID) == GUID);
        }
        kopsik_time_entry_view_item_list_clear(visible);

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
        kopsik_format_duration_in_seconds_hhmm(10, 0, str, kMaxStrLen);
        ASSERT_EQ("00:00", std::string(str));
        kopsik_format_duration_in_seconds_hhmm(60, 0, str, kMaxStrLen);
        ASSERT_EQ("00:01", std::string(str));
        kopsik_format_duration_in_seconds_hhmm(65, 0, str, kMaxStrLen);
        ASSERT_EQ("00:01", std::string(str));
        kopsik_format_duration_in_seconds_hhmm(3600, 0, str, kMaxStrLen);
        ASSERT_EQ("01:00", std::string(str));
        kopsik_format_duration_in_seconds_hhmm(5400, 0, str, kMaxStrLen);
        ASSERT_EQ("01:30", std::string(str));
        kopsik_format_duration_in_seconds_hhmm(5410, 0, str, kMaxStrLen);
        ASSERT_EQ("01:30", std::string(str));
    }

    TEST(KopsikApiTest, kopsik_time_entry_view_item_list_init) {
        KopsikTimeEntryViewItemList *list =
            kopsik_time_entry_view_item_list_init();
        ASSERT_TRUE(list);
        kopsik_time_entry_view_item_list_clear(list);
    }

}  // namespace kopsik
