// Copyright 2014 Toggl Desktop developers.

#include "../src/proxy.h"

#include <sstream>

namespace toggl {

bool Proxy::IsConfigured() const {
    return !host_.empty() && port_;
}

bool Proxy::HasCredentials() const {
    return !username_.empty() && !password_.empty();
}

std::string Proxy::String() const {
    std::stringstream ss;
    ss << "Proxy host=" << host_
       << " port=" << port_
       << " username=" << username_
       << " password=" << password_;
    return ss.str();
}

}  // namespace toggl
