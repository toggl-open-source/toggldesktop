
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

void Analytics::Track(const std::string client_id,
                      const std::string category,
                      const std::string action) {
    start(new GoogleAnalyticsEvent(
        client_id, category, action, "", 1));
}

void Analytics::TrackAutocompleteUsage(const std::string client_id,
                                       const bool was_using_autocomplete) {
    std::stringstream ss;
    ss << "timer/autocomplete-";
    if (was_using_autocomplete) {
        ss << "true";
    } else {
        ss << "false";
    }

    Track(client_id, "timer", ss.str());
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
    HTTPSRequest req;
    req.host = "https://ssl.google-analytics.com";
    req.relative_url = relativeURL();

    HTTPSClient client;
    HTTPSResponse resp = client.Get(req);
    if (resp.err != noError) {
        Poco::Logger::get("Analytics").error(resp.err);
        return;
    }
}

}  // namespace toggl
