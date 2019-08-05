// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TEST_TEST_DATA_H_
#define SRC_TEST_TEST_DATA_H_

#include <string>

#if defined(WIN32) || defined(_WIN32)
#define TESTDB L"test.db"
#else
#define TESTDB "test.db"
#endif

std::string loadTestData();
std::string loadTestDataFile(const std::string filename);

#endif  // SRC_TEST_TEST_DATA_H_
