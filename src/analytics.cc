
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
#include "./urls.h"
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
    HTTPSRequest req;
    req.host = "https://ssl.google-analytics.com";
    req.relative_url = relativeURL();
    error err = client.Get(req, &response_body);
    if (err != noError) {
        Poco::Logger::get("Analytics").error(err);
        return;
    }
}

void Analytics::TrackSettingsUsage(
    const std::string user_api_token,
    const Settings &settings,
    const std::string update_channel,
    const std::string desktop_id) {

    Json::Value json;
    json["toggl_desktop_settings"] = settings.SaveToJSON();
    if (!update_channel.empty()) {
        json["toggl_desktop_settings"]["update_channel"] = update_channel;
    }
    if (!desktop_id.empty()) {
        json["toggl_desktop_settings"]["desktop_id"] = desktop_id;
    }

    start(new TogglAnalyticsEvent(
        user_api_token,
        Json::StyledWriter().write(json)));
}

void TogglAnalyticsEvent::runTask() {
    Poco::Logger &logger = Poco::Logger::get("Analytics");

    logger.debug(json_);

    TogglClient toggl_client;
    std::string response_body("");
    HTTPSRequest req;
    req.host = urls::API();
    req.relative_url = "/api/v9/analytics";
    req.payload = json_;
    req.basic_auth_username = api_token_;
    req.basic_auth_password = "api_token";

    error err = toggl_client.Post(req, &response_body);
    if (err != noError) {
        logger.error(err);
        return;
    }

    logger.debug(response_body);
}

}  // namespace toggl
