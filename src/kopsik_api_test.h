// Copyright 2013 Tanel Lebedev

#ifndef SRC_KOPSIK_API_TEST_H_
#define SRC_KOPSIK_API_TEST_H_

#include "gtest/gtest.h"

namespace kopsik {

    class KopsikApiTest : public ::testing::Test {};

    void wipe_test_db();

}  // namespace kopsik

#endif  // SRC_KOPSIK_API_TEST_H_
