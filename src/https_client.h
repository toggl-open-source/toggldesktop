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

namespace kopsik {

class HTTPSClient {
 public:
    explicit HTTPSClient(
        const std::string api_url,
        const std::string app_name,
        const std::string app_version)
        : api_url_(api_url)
    , app_name_(app_name)
    , app_version_(app_version) {}
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

    void SetApiURL(const std::string value) {
        api_url_ = value;
    }
    void SetProxy(const Proxy value) {
        proxy_ = value;
    }

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
    Poco::Net::Context *get_context() const;

    std::string api_url_;
    std::string app_name_;
    std::string app_version_;

    Proxy proxy_;
};

}  // namespace kopsik

#endif  // SRC_HTTPS_CLIENT_H_
