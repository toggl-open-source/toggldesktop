// Copyright 2014 Toggl Desktop developers.

#include "test_data.h"

#include <sstream>

#include "Poco/FileStream.h"

#if defined(WIN32) || defined(_WIN32)
#define TESTDATADIR "../../../../testdata/"
#else
#define TESTDATADIR "../testdata/"
#endif

std::string loadTestData() {
    return loadFromTestDataDir("me.json");
}

std::string loadFromTestDataDir(const std::string &filename) {
    return loadTestDataFile(std::string(TESTDATADIR) + filename);
}

std::string loadTestDataFile(const std::string &filename) {
    Poco::FileStream fis(filename, std::ios::binary);
    std::stringstream ss;
    ss << fis.rdbuf();
    fis.close();
    return ss.str();
}
