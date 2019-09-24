#ifndef SRC_ANALYTICS_H_
#define SRC_ANALYTICS_H_

#include "model/settings.h"
#include "proxy.h"

#include <string>
#include <Poco/LocalDateTime.h>

namespace toggl {

class Rectangle;

class Analytics {
 public:
    Analytics();

    void Track(const std::string &client_id, const std::string &category, const std::string &action);
    void TrackChannel(const std::string &client_id, const std::string &channel);
    void TrackOs(const std::string &client_id, const std::string &os);
    void TrackOSDetails(const std::string &client_id);
    void TrackSettings(const std::string &client_id, bool record_timeline, const Settings &settings, bool use_proxy, const Proxy &proxy);
    void TrackIdleDetectionClick(const std::string &client_id, const std::string &button);
    void TrackAutocompleteUsage(const std::string &client_id, const bool was_using_autocomplete);
    void TrackWindowSize(const std::string &client_id, const std::string &os, const toggl::Rectangle &rect);
    void TrackEditSize(const std::string &client_id, const std::string &os, const toggl::Rectangle &rect);

 private:
    void TrackSize(const std::string &client_id, const std::string &os, const std::string &name, const toggl::Rectangle &rect);

    Poco::LocalDateTime settings_sync_date;
};

class GoogleAnalyticsEvent  {
 public:
    GoogleAnalyticsEvent(
        const std::string &client_id,
        const std::string &category,
        const std::string &action,
        const std::string &opt_label,
        const int opt_value)
    : client_id_(client_id)
    , category_(category)
    , action_(action)
    , opt_label_(opt_label)
    , opt_value_(opt_value) {}
    void runTask();

 private:
    const std::string relativeURL();

    std::string client_id_;
    std::string category_;
    std::string action_;
    std::string opt_label_;
    int opt_value_;
};

class GoogleAnalyticsSettingsEvent {
 public:
    GoogleAnalyticsSettingsEvent(
        const std::string &client_id,
        const std::string &category,
        const bool record_timeline,
        Settings &settings,
        const bool uses_proxy,
        Proxy &proxy)
    : client_id_(client_id)
    , category_(category)
    , uses_proxy(uses_proxy)
    , record_timeline(record_timeline)
    , settings(settings)
    , proxy(proxy) {}
    void runTask();

 private:
    const std::string relativeURL();
    void makeReq();
    void setActionBool(std::string type, bool value);
    void setActionInt(std::string type, Poco::Int64 value);
    void setActionString(std::string type, std::string value);

    std::string client_id_;
    std::string category_;
    std::string action_;
    bool uses_proxy;
    bool record_timeline;
    Settings settings;
    Proxy proxy;
};

}  // namespace toggl

#endif // SRC_ANALYTICS_H_
