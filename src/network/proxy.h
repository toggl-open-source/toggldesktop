// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_PROXY_H_
#define SRC_PROXY_H_

#include <string>

#include "Poco/Types.h"

namespace toggl {

class Proxy {
 public:
    Proxy() :
    host_(""),
    port_(0),
    username_(""),
    password_("") {}

    bool IsConfigured() const;
    bool HasCredentials() const;

    std::string String() const;

    const std::string &Host() const {
        return host_;
    }
    void SetHost(const std::string &value) {
        host_ = value;
    }

    const Poco::UInt64 &Port() const {
        return port_;
    }
    void SetPort(const Poco::UInt64 value) {
        port_ = value;
    }

    const std::string &Username() const {
        return username_;
    }
    void SetUsername(const std::string &value) {
        username_ = value;
    }

    const std::string &Password() const {
        return password_;
    }
    void SetPassword(const std::string &value) {
        password_ = value;
    }

 private:
    std::string host_;
    Poco::UInt64 port_;
    std::string username_;
    std::string password_;
};

}  // namespace toggl

#endif  // SRC_PROXY_H_
