// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_ANALYTICS_H_
#define SRC_ANALYTICS_H_

#include <string>

#include "Poco/Task.h"
#include "Poco/TaskManager.h"

#include "./types.h"

namespace toggl {

class User;
class Settings;

class Analytics : public Poco::TaskManager {
 public:
    void TrackAutocompleteUsage(
        const std::string client_id,
        const bool was_using_autocomplete);
    void TrackSettingsUsage(
        const std::string user_api_token,
        const Settings &settings);
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

class TogglAnalyticsEvent : public Poco::Task {
 public:
    TogglAnalyticsEvent(
        const std::string user_api_token,
        const std::string json) : Poco::Task("TogglAnalyticsEvent")
    , api_token_(user_api_token)
    , json_(json) {}

    void runTask();

 private:
    std::string api_token_;
    std::string json_;
};

}  // namespace toggl

#endif  // SRC_ANALYTICS_H_
