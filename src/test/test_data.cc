// Copyright 2014 Toggl Desktop developers.

#include "../../src/test/test_data.h"

#include <sstream>

#include "Poco/FileStream.h"

std::string loadTestData() {
    return loadTestDataFile("../testdata/me.json");
}

std::string loadTestDataFile(const std::string &filename) {
    Poco::FileStream fis(filename, std::ios::binary);
    std::stringstream ss;
    ss << fis.rdbuf();
    fis.close();
    return ss.str();
}
