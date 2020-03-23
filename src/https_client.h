// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_HTTPS_CLIENT_H_
#define SRC_HTTPS_CLIENT_H_

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "const.h"
#include "proxy.h"
#include "types.h"
#include "util/logger.h"

#include <Poco/Activity.h>
#include <Poco/Timestamp.h>

namespace Poco {
    namespace Net {
        class HTMLForm;
    } // namespace Poco::Net
} // namespace Poco

namespace toggl {

class TOGGL_INTERNAL_EXPORT ServerStatus {
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
    void DisableStatusCheck() {
        stopStatusCheck("DisableStatusCheck");
    }

 protected:
    void runActivity();

 private:
    bool gone_;
    Poco::Activity<ServerStatus> checker_;
    bool fast_retry_;

    void setGone(const bool value);
    bool gone();

    void startStatusCheck();
    void stopStatusCheck(const std::string &reason);
    bool checkingStatus();

    Logger logger() const;
};

class TOGGL_INTERNAL_EXPORT HTTPSClientConfig {
 public:
    HTTPSClientConfig()
        : AppName("")
    , AppVersion("")
    , UseProxy(false)
    , ProxySettings(Proxy())
    , IgnoreCert(false)
    , CACertPath("")
    , AutodetectProxy(true) {}
    ~HTTPSClientConfig() {}

    std::string AppName;
    std::string AppVersion;
    bool UseProxy;
    toggl::Proxy ProxySettings;
    bool IgnoreCert;
    std::string CACertPath;
    bool AutodetectProxy;

    std::vector<Poco::UInt64> OBMExperimentNrs;

    std::string UserAgent() const {
        std::stringstream ss;
        ss << AppName + "/" + AppVersion;
        for (auto it = OBMExperimentNrs.begin();
                it != OBMExperimentNrs.end();
                ++it) {
            ss << "-obm-" << *it;
        }
        return ss.str();
    }
};

class TOGGL_INTERNAL_EXPORT HTTPSRequest {
 public:
    HTTPSRequest() {}
    ~HTTPSRequest() {}

    bool IsNull() const {
        return method.empty() && host.empty() && relative_url.empty() && payload.empty() && basic_auth_password.empty() && basic_auth_password.empty()
            && !form && timeout_seconds == kHTTPClientTimeoutSeconds;
    }

    std::string method {};
    std::string host {};
    std::string relative_url {};
    std::string payload {};
    std::string basic_auth_username {};
    std::string basic_auth_password {};
    Poco::Net::HTMLForm *form {};
    Poco::Int64 timeout_seconds { kHTTPClientTimeoutSeconds };
};

class TOGGL_INTERNAL_EXPORT HTTPSResponse {
 public:
    HTTPSResponse()
        : body("")
    , err(noError)
    , status_code(0) {}
    virtual ~HTTPSResponse() {}

    std::string body;
    error err;
    Poco::Int64 status_code;
};

class TOGGL_INTERNAL_EXPORT HTTPSClient {
 public:
    HTTPSClient() {}
    virtual ~HTTPSClient() {}

    HTTPSResponse Post(
        HTTPSRequest req) const;

    HTTPSResponse Get(
        HTTPSRequest req) const;

    HTTPSResponse GetFile(
        HTTPSRequest req) const;

    HTTPSResponse Delete(
        HTTPSRequest req) const;

    HTTPSResponse Put(
        HTTPSRequest req) const;

    virtual HTTPSResponse Request(
        HTTPSRequest req) const;

    static HTTPSClientConfig Config;

 protected:

    virtual Logger logger() const;

 private:
    // We only make requests if this timestamp lies in the past.
    static std::map<std::string, Poco::Timestamp> banned_until_;

    error statusCodeToError(const Poco::Int64 status_code) const;

    bool isRedirect(const Poco::Int64 status_code) const;

    virtual HTTPSResponse makeHttpRequest(
        HTTPSRequest req) const;
};

class TOGGL_INTERNAL_EXPORT SyncStateMonitor {
 public:
    virtual ~SyncStateMonitor() {}

    virtual void DisplaySyncState(const Poco::Int64 state) = 0;
};

class TOGGL_INTERNAL_EXPORT TogglClient : public HTTPSClient {
 public:
    explicit TogglClient(SyncStateMonitor *monitor = nullptr)
        : monitor_(monitor) {}

    virtual HTTPSResponse Request(
        HTTPSRequest req) const override;

    static ServerStatus TogglStatus;

 protected:

    virtual Logger logger() const override;

 private:
    SyncStateMonitor *monitor_;
};

}  // namespace toggl

#endif  // SRC_HTTPS_CLIENT_H_
