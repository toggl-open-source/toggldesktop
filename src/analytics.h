// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_ANALYTICS_H_
#define SRC_ANALYTICS_H_

#include <string>

#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/LocalDateTime.h"
#include "./proxy.h"
#include "./settings.h"

namespace toggl {

class Analytics : public Poco::TaskManager {
 public:
    Analytics()
        : Poco::TaskManager()
    , settings_sync_date(
        Poco::LocalDateTime() -
        Poco::Timespan(24 * Poco::Timespan::HOURS)) {}

    void Track(
        const std::string client_id,
        const std::string category,
        const std::string action);

    void TrackChannel(
        const std::string client_id,
        const std::string channel);

    void TrackSettings(
        const std::string client_id,
        const bool record_timeline,
        const Settings settings,
        const bool use_proxy,
        const Proxy proxy);

    void TrackIdleDetectionClick(
        const std::string client_id,
        const std::string button);

    void TrackAutocompleteUsage(
        const std::string client_id,
        const bool was_using_autocomplete);

 private:
    Poco::LocalDateTime settings_sync_date;
};

class GoogleAnalyticsEvent : public Poco::Task {
 public:
    GoogleAnalyticsEvent(
        const std::string client_id,
        const std::string category,
        const std::string action,
        const std::string opt_label,
        const int opt_value)
        : Poco::Task("GoogleAnalyticsEvent")
    , client_id_(client_id)
    , category_(category)
    , action_(action)
    , opt_label_(opt_label)
    , opt_value_(opt_value) {}
    void runTask();

 private:
    std::string client_id_;
    std::string category_;
    std::string action_;
    std::string opt_label_;
    int opt_value_;

    const std::string relativeURL();
};

class GoogleAnalyticsSettingsEvent : public Poco::Task {
 public:
    GoogleAnalyticsSettingsEvent(
        const std::string client_id,
        const std::string category,
        const bool record_timeline,
        Settings settings,
        const bool uses_proxy,
        Proxy proxy)
        : Poco::Task("GoogleAnalyticsSettingsEvent")
    , client_id_(client_id)
    , category_(category)
    , record_timeline(record_timeline)
    , settings(settings)
    , uses_proxy(uses_proxy)
    , proxy(proxy) {}
    void runTask();

 private:
    std::string client_id_;
    std::string category_;
    std::string action_;
    bool uses_proxy;
    bool record_timeline;
    Settings settings;
    Proxy proxy;

    const std::string relativeURL();
    void makeReq();
    void setActionBool(std::string type, bool value);
    void setActionInt(std::string type, Poco::Int64 value);
    void setActionString(std::string type, std::string value);
};


}  // namespace toggl

#endif  // SRC_ANALYTICS_H_
