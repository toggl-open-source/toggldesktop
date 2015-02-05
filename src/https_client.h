// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_HTTPS_CLIENT_H_
#define SRC_HTTPS_CLIENT_H_

#include <string>
#include <vector>
#include <map>

#include "./proxy.h"
#include "./types.h"

#include "Poco/Activity.h"
#include "Poco/Timestamp.h"

namespace Poco {
class Logger;
namespace Util {
}
}

namespace toggl {

class ServerStatus {
 public:
    ServerStatus()
        : gone_(false)
    , checker_(this, &ServerStatus::runActivity)
    , fast_retry_(true) {}

    virtual ~ServerStatus() {
        stopStatusCheck("destructor");
    }

    error Status();
    void UpdateStatus(const Poco::Int64 status_code);

 protected:
    void runActivity();

 private:
    bool gone_;
    Poco::Activity<ServerStatus> checker_;
    bool fast_retry_;

    void setGone(const bool value);
    bool gone();

    void startStatusCheck();
    void stopStatusCheck(const std::string reason);
    bool checkingStatus();

    Poco::Logger &logger() const;
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

 protected:
    virtual error request(
        const std::string method,
        const std::string host,
        const std::string relative_url,
        const std::string payload,
        const std::string basic_auth_username,
        const std::string basic_auth_password,
        std::string *response_body,
        Poco::Int64 *response_status);

    virtual Poco::Logger &logger() const;

 private:
    // We only make requests if this timestamp lies in the past.
    static std::map<std::string, Poco::Timestamp> banned_until_;

    error statusCodeToError(const Poco::Int64 status_code) const;
};

class TogglClient : public HTTPSClient {
 public:
    static ServerStatus TogglStatus;

 protected:
    virtual error request(
        const std::string method,
        const std::string host,
        const std::string relative_url,
        const std::string json,
        const std::string basic_auth_username,
        const std::string basic_auth_password,
        std::string *response_body,
        Poco::Int64 *response_status);

    virtual Poco::Logger &logger() const;
};

}  // namespace toggl

#endif  // SRC_HTTPS_CLIENT_H_
