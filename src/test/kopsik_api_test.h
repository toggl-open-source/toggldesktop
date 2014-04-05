// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TEST_KOPSIK_API_TEST_H_
#define SRC_TEST_KOPSIK_API_TEST_H_

#include "gtest/gtest.h"

namespace kopsik {

class KopsikApiTest : public ::testing::Test {};

void wipe_test_db();

}  // namespace kopsik

#endif  // SRC_TEST_KOPSIK_API_TEST_H_
