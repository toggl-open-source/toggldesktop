
// Copyright 2015 Toggl Desktop developers

// Collect Toggl Desktop usage using Google Analytics Measurement Protocol
// https://developers.google.com/analytics/devguides/collection/protocol/v1/
// or using Toggl's own backend when appropriate.

#include "../src/analytics.h"

#include <sstream>

#include <json/json.h>  // NOLINT

#include "./const.h"
#include "./https_client.h"
#include "./settings.h"
#include "./user.h"

#include "Poco/Logger.h"

namespace toggl {

void Analytics::TrackAutocompleteUsage(const std::string client_id,
                                       const bool was_using_autocomplete) {
    int value = 0;
    if (was_using_autocomplete) {
        value = 1;
    }
    start(new GoogleAnalyticsEvent(
        client_id, "timer", "start", "was_using_autocomplete", value));
}

const std::string GoogleAnalyticsEvent::relativeURL() {
    std::stringstream ss;
    ss << "/collect"
       << "?v=" << "1"
       << "&tid=" << "UA-3215787-27"
       << "&cid=" << client_id_
       << "&t=" << "event"
       << "&ec=" << category_
       << "&ea=" << action_
       << "&el=" << opt_label_
       << "&ev=" << opt_value_;
    return ss.str();
}

void GoogleAnalyticsEvent::runTask() {
    std::string response_body("");
    HTTPSClient client;
    error err = client.Get(
        "https://ssl.google-analytics.com",
        relativeURL(),
        std::string(""),
        std::string(""),
        &response_body);

    if (err != noError) {
        Poco::Logger::get("Analytics").error(err);
        return;
    }
}

void Analytics::TrackSettingsUsage(
    const std::string user_api_token,
    const Settings &settings) {

    Json::Value json;
    json["toggl_desktop_settings"] = settings.SaveToJSON();

    start(new TogglAnalyticsEvent(
        user_api_token,
        Json::StyledWriter().write(json)));
}

void TogglAnalyticsEvent::runTask() {
    Poco::Logger &logger = Poco::Logger::get("Analytics");

    logger.debug(json_);

    TogglClient toggl_client;
    std::string response_body("");
    error err = toggl_client.Post(kAPIURL,
                                  "/api/v9/analytics",
                                  json_,
                                  api_token_,
                                  "api_token",
                                  &response_body);
    if (err != noError) {
        logger.error(err);
        return;
    }

    logger.debug(response_body);
}

}  // namespace toggl
