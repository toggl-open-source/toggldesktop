// Copyright 2013 Tanel Lebedev

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "./kopsik_api.h"

#include "Poco/File.h"

#define TESTDB "test.db"
#define ERRLEN 1024

namespace kopsik {

    TEST(KopsikApiTest, kopsik_context_init) {
        KopsikContext *ctx = kopsik_context_init();
        ASSERT_TRUE(ctx);
        kopsik_context_clear(ctx);
        ASSERT_FALSE(ctx->db);
        ASSERT_FALSE(ctx->user);
    }

    TEST(KopsikApiTest, kopsik_version) {
        int major = 0;
        int minor = 0;
        int patch = 0;
        kopsik_version(&major, &minor, &patch);
        ASSERT_TRUE(major || minor || patch);
    }

    TEST(KopsikApiTest, kopsik_set_proxy) {
        KopsikContext *ctx = kopsik_context_init();
        kopsik_set_proxy(ctx, "localhost", 8000, "johnsmith", "secret");
        ASSERT_TRUE(true);
        kopsik_context_clear(ctx);
    }

    TEST(KopsikApiTest, kopsik_set_db_path) {
        KopsikContext *ctx = kopsik_context_init();
        {
            Poco::File f(TESTDB);
            if (f.exists()) f.remove(false);
        }
        kopsik_set_db_path(ctx, TESTDB);
        kopsik_context_clear(ctx);
        Poco::File f(TESTDB);
        ASSERT_TRUE(f.exists());
    }

    TEST(KopsikApiTest, kopsik_set_log_path) {
        KopsikContext *ctx = kopsik_context_init();
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
        KopsikContext *ctx = kopsik_context_init();
        Poco::File f(TESTDB);
        if (f.exists()) f.remove(false);
        kopsik_set_db_path(ctx, TESTDB);
        char err[ERRLEN];
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_set_api_token(ctx, err, ERRLEN, "token"));
        const int kMaxStrLen = 10;
        char str[kMaxStrLen];
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_get_api_token(ctx, err, ERRLEN, str, kMaxStrLen));
        ASSERT_EQ("token", std::string(str));
        kopsik_context_clear(ctx);
    }

    TEST(KopsikApiTest, kopsik_current_user) {
        ASSERT_TRUE(false);
    }

    TEST(KopsikApiTest, kopsik_login) {
        ASSERT_TRUE(false);
    }

    TEST(KopsikApiTest, kopsik_logout) {
        KopsikContext *ctx = kopsik_context_init();
        Poco::File f(TESTDB);
        if (f.exists()) f.remove(false);
        kopsik_set_db_path(ctx, TESTDB);
        char err[ERRLEN];
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_set_api_token(ctx, err, ERRLEN, "secret"));
        const int kMaxStrLen = 10;
        char str[kMaxStrLen];
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_get_api_token(ctx, err, ERRLEN, str, kMaxStrLen));
        ASSERT_EQ("secret", std::string(str));
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_logout(ctx, err, ERRLEN));
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_get_api_token(ctx, err, ERRLEN, str, kMaxStrLen));
        ASSERT_EQ("", std::string(str));
        kopsik_context_clear(ctx);
    }

    TEST(KopsikApiTest, kopsik_sync) {
        ASSERT_TRUE(false);
    }

    TEST(KopsikApiTest, kopsik_push) {
        ASSERT_TRUE(false);
    }

    TEST(KopsikApiTest, kopsik_dirty_models) {
        ASSERT_TRUE(false);
    }

    TEST(KopsikApiTest, kopsik_time_entry_view_item_init) {
        KopsikTimeEntryViewItem *te = kopsik_time_entry_view_item_init();
        ASSERT_TRUE(te);
        kopsik_time_entry_view_item_clear(te);
    }

    TEST(KopsikApiTest, kopsik_running_time_entry_view_item) {
        ASSERT_TRUE(false);
    }

    TEST(KopsikApiTest, kopsik_format_duration_in_seconds) {
        const int kMaxStrLen = 100;
        char str[kMaxStrLen];
        kopsik_format_duration_in_seconds(10, str, kMaxStrLen);
        ASSERT_EQ("10 sec", std::string(str));
        kopsik_format_duration_in_seconds(60, str, kMaxStrLen);
        ASSERT_EQ("01:00 min", std::string(str));
        kopsik_format_duration_in_seconds(65, str, kMaxStrLen);
        ASSERT_EQ("01:05 min", std::string(str));
        kopsik_format_duration_in_seconds(3600, str, kMaxStrLen);
        ASSERT_EQ("01:00:00", std::string(str));
        kopsik_format_duration_in_seconds(5400, str, kMaxStrLen);
        ASSERT_EQ("01:30:00", std::string(str));
        kopsik_format_duration_in_seconds(5410, str, kMaxStrLen);
        ASSERT_EQ("01:30:10", std::string(str));
    }

    TEST(KopsikApiTest, kopsik_start) {
        ASSERT_TRUE(false);
    }

    TEST(KopsikApiTest, kopsik_continue) {
        ASSERT_TRUE(false);
    }

    TEST(KopsikApiTest, kopsik_stop) {
        ASSERT_TRUE(false);
    }

    TEST(KopsikApiTest, kopsik_time_entry_view_item_list_init) {
        KopsikTimeEntryViewItemList *list =
            kopsik_time_entry_view_item_list_init();
        ASSERT_TRUE(list);
        kopsik_time_entry_view_item_list_clear(list);
    }

    TEST(KopsikApiTest, kopsik_time_entry_view_items) {
        KopsikContext *ctx = kopsik_context_init();
        KopsikTimeEntryViewItemList *list =
            kopsik_time_entry_view_item_list_init();
        ASSERT_TRUE(list);
        char err[ERRLEN];
        ASSERT_EQ(KOPSIK_API_SUCCESS,
            kopsik_time_entry_view_items(ctx, err, ERRLEN, list));
        kopsik_time_entry_view_item_list_clear(list);
        kopsik_context_clear(ctx);
    }

    TEST(KopsikApiTest, kopsik_listen) {
        ASSERT_TRUE(false);
    }

}  // namespace kopsik
