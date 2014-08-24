// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TEST_TOGGL_API_TEST_H_
#define SRC_TEST_TOGGL_API_TEST_H_

#include "gtest/gtest.h"

namespace toggl {

class TogglApiTest : public ::testing::Test {};

void wipe_test_db();

}  // namespace toggl

#endif  // SRC_TEST_TOGGL_API_TEST_H_
