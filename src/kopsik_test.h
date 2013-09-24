// Copyright 2013 Tanel Lebedev

#ifndef SRC_KOPSIK_TEST_H_
#define SRC_KOPSIK_TEST_H_

#include "gtest/gtest.h"
#include "./toggl_api_client.h"

namespace kopsik {

    class KopsikTest : public ::testing::Test {};

}  // namespace kopsik

int main(int argc, char **argv);

#endif  // SRC_KOPSIK_TEST_H_
