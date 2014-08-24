// Copyright 2014 Toggl Desktop developers.

#include "./proxy.h"

#include <sstream>

namespace toggl {

bool Proxy::IsConfigured() const {
    return !host.empty() && port;
}

bool Proxy::HasCredentials() const {
    return !username.empty() && !password.empty();
}

std::string Proxy::String() const {
    std::stringstream ss;
    ss << "Proxy host=" << host
       << " port=" << port
       << " username=" << username
       << " password=" << password;
    return ss.str();
}

}  // namespace toggl
