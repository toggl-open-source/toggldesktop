// Copyright 2014 Toggl Desktop developers.

#include "./test_data.h"

#include <sstream>

#include "Poco/FileStream.h"

std::string loadTestData() {
    Poco::FileStream fis("testdata/me.json", std::ios::binary);
    std::stringstream ss;
    ss << fis.rdbuf();
    fis.close();
    return ss.str();
}
