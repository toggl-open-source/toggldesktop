// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_PROXY_H_
#define SRC_PROXY_H_

#include <string>

#include "./types.h"

#include "Poco/Types.h"

namespace toggl {

class Proxy {
 public:
    Proxy() :
    host(""),
    port(0),
    username(""),
    password("") {}

    bool IsConfigured() const;
    bool HasCredentials() const;
    std::string String() const;

    std::string host;
    Poco::UInt64 port;
    std::string username;
    std::string password;
};

}  // namespace toggl

#endif  // SRC_PROXY_H_
