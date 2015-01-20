// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_HTTPS_CLIENT_H_
#define SRC_HTTPS_CLIENT_H_

#include <string>
#include <vector>

#include "./types.h"
#include "./proxy.h"

#include "Poco/Timestamp.h"

namespace toggl {

class ServerStatus {
 public:
    ServerStatus() {}
    virtual ~ServerStatus() {}

 private:
    bool gone_;
    Poco::Timestamp next_try_at_;
};

class HTTPSClientConfig {
 public:
    HTTPSClientConfig()
        : AppName("")
    , AppVersion("")
    , UseProxy(false)
    , ProxySettings(Proxy())
    , IgnoreCert(false)
    , CACertPath("") {}
    ~HTTPSClientConfig() {}

    std::string AppName;
    std::string AppVersion;
    bool UseProxy;
    toggl::Proxy ProxySettings;
    bool IgnoreCert;
    std::string CACertPath;

    std::string UserAgent() {
        return AppName + "/" + AppVersion;
    }
};

class HTTPSClient {
 public:
    HTTPSClient() {}
    virtual ~HTTPSClient() {}

    virtual error PostJSON(
        const std::string host,
        const std::string relative_url,
        const std::string json,
        const std::string basic_auth_username,
        const std::string basic_auth_password,
        std::string *response_body);

    virtual error GetJSON(
        const std::string host,
        const std::string relative_url,
        const std::string basic_auth_username,
        const std::string basic_auth_password,
        std::string *response_body);

    static HTTPSClientConfig Config;
    static ServerStatus BackendStatus;;

 private:
    error request(
        const std::string method,
        const std::string host,
        const std::string relative_url,
        const std::string payload,
        const std::string basic_auth_username,
        const std::string basic_auth_password,
        std::string *response_body);

    error requestJSON(
        const std::string method,
        const std::string host,
        const std::string relative_url,
        const std::string json,
        const std::string basic_auth_username,
        const std::string basic_auth_password,
        std::string *response_body);
};

}  // namespace toggl

#endif  // SRC_HTTPS_CLIENT_H_
