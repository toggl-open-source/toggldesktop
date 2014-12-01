// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_HTTPS_CLIENT_H_
#define SRC_HTTPS_CLIENT_H_

#include <string>
#include <vector>

#include "Poco/Activity.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"

#include "./types.h"
#include "./proxy.h"

namespace toggl {

class HTTPSClientConfig {
 public:
    static std::string AppName;
    static std::string AppVersion;
    static std::string APIURL;
    static bool UseProxy;
    static toggl::Proxy ProxySettings;
    static bool IgnoreCert;
    static std::string CACertPath;

    static std::string UserAgent() {
        return AppName + "/" + AppVersion;
    }
};

class HTTPSClient {
 public:
    HTTPSClient() {}
    virtual ~HTTPSClient() {}

    virtual error PostJSON(
        const std::string relative_url,
        const std::string json,
        const std::string basic_auth_username,
        const std::string basic_auth_password,
        std::string *response_body);

    virtual error GetJSON(
        const std::string relative_url,
        const std::string basic_auth_username,
        const std::string basic_auth_password,
        std::string *response_body);

 private:
    error request(
        const std::string method,
        const std::string relative_url,
        const std::string payload,
        const std::string basic_auth_username,
        const std::string basic_auth_password,
        std::string *response_body);
    error requestJSON(
        const std::string method,
        const std::string relative_url,
        const std::string json,
        const std::string basic_auth_username,
        const std::string basic_auth_password,
        std::string *response_body);

    // If we get 410, we'll refuse to connect to the endpoint again
    static bool endpoint_gone;
};

}  // namespace toggl

#endif  // SRC_HTTPS_CLIENT_H_
