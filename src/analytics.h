// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_ANALYTICS_H_
#define SRC_ANALYTICS_H_

#include <string>

#include <Poco/Task.h>
#include <Poco/TaskManager.h>
#include <Poco/LocalDateTime.h>

#include "proxy.h"
#include "util/memory.h"
#include "util/rectangle.h"
#include "model/settings.h"

namespace toggl {

class Analytics : public Poco::TaskManager {
 public:
    Analytics()
        : Poco::TaskManager()
    , settings_sync_date(
        Poco::LocalDateTime() -
        Poco::Timespan(24 * Poco::Timespan::HOURS)) {}

    void Track(
        const std::string &client_id,
        const std::string &category,
        const std::string &action);

    void TrackChannel(
        const std::string &client_id,
        const std::string &channel);

    void TrackOs(
        const std::string &client_id,
        const std::string &os);

    void TrackOSDetails(
        const std::string &client_id);

    void TrackInAppMessage(
        const std::string &client_id,
        const std::string &id,
        const Poco::Int64 type);

    void TrackSettings(const std::string &client_id,
        const bool record_timeline,
        locked<const Settings> &settings,
        const bool use_proxy,
        const Proxy &proxy);

    void TrackIdleDetectionClick(
        const std::string &client_id,
        const std::string &button);

    void TrackAutocompleteUsage(
        const std::string &client_id,
        const bool was_using_autocomplete);

    void TrackWindowSize(const std::string &client_id,
                         const std::string &os,
                         const toggl::Rectangle rect);

    void TrackEditSize(const std::string &client_id,
                       const std::string &os,
                       const toggl::Rectangle rect);

    void TrackStartTimeEntry(const std::string &client_id,
                             const std::string& os,
                             const uint8_t tab_index);
    void TrackEditTimeEntry(const std::string &client_id,
                            const std::string& os,
                            const uint8_t tab_index);
    void TrackDeleteTimeEntry(const std::string &client_id,
                              const std::string& os,
                              const uint8_t tab_index);

    void TrackLoginWithUsernamePassword(const std::string &client_id);
    void TrackLoginWithGoogle(const std::string &client_id);
    void TrackSignupWithUsernamePassword(const std::string &client_id);
    void TrackSignupWithGoogle(const std::string &client_id);

 private:
    Poco::LocalDateTime settings_sync_date;

    void TrackSize(const std::string &client_id,
                   const std::string &os,
                   const std::string &name,
                   const toggl::Rectangle rect);

    void TrackTimeEntryActivity(const std::string &client_id,
                                const std::string& os,
                                const std::string &action,
                                const uint8_t tab_index);

    void TrackUserAuthentication(const std::string &client_id,
                                 const std::string &action,
                                 const std::string &from);
};

class GoogleAnalyticsEvent : public Poco::Task {
 public:
    GoogleAnalyticsEvent(
        const std::string &client_id,
        const std::string &category,
        const std::string &action,
        const std::string &opt_label,
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
        const std::string &client_id,
        const std::string &category,
        const bool record_timeline,
        Settings settings,
        const bool uses_proxy,
        const Proxy &proxy)
        : Poco::Task("GoogleAnalyticsSettingsEvent")
    , client_id_(client_id)
    , category_(category)
    , uses_proxy(uses_proxy)
    , record_timeline(record_timeline)
    , settings(settings)
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
    void setActionBool(const std::string &type, bool value);
    void setActionInt(const std::string &type, Poco::Int64 value);
    void setActionString(const std::string &type, const std::string &value);
};


}  // namespace toggl

#endif  // SRC_ANALYTICS_H_
