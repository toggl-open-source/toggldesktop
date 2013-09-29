// Copyright 2013 Tanel Lebedev

#include "gtest/gtest.h"
#include "./kopsik_api.h"

#include "Poco/File.h"

#define TESTDB "test.db"

namespace kopsik {

    TEST(KopsikApiTest, kopsik_context_init) {
        KopsikContext *ctx = kopsik_context_init();
        ASSERT_TRUE(ctx);
        kopsik_context_clear(ctx);
        ASSERT_FALSE(ctx->db);
        ASSERT_FALSE(ctx->user);
    }

    TEST(KopsikApiTest, kopsik_version) {
        int major(0), minor(0), patch(0);
        kopsik_version(&major, &minor, &patch);
        ASSERT_TRUE(major || minor || patch);
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
        ASSERT_FALSE(user->ID);
        ASSERT_FALSE(user->Fullname);
    }

}  // namespace kopsik
