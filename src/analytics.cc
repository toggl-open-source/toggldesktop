
// Copyright 2015 Toggl Desktop developers

// Collect Toggl Desktop usage using Google Analytics Measurement Protocol
// https://developers.google.com/analytics/devguides/collection/protocol/v1/

#include "../src/analytics.h"

#include <sstream>

#include "./https_client.h"

#include "Poco/Logger.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"

namespace toggl {

void Analytics::TrackAutocompleteUsage(const std::string client_id,
                                       const bool was_using_autocomplete) {
    int value = 0;
    if (was_using_autocomplete) {
        value = 1;
    }
    start(new TrackEvent(
        client_id, "timer", "start", "was_using_autocomplete", value));
}

const std::string TrackEvent::relativeURL() {
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

void TrackEvent::runTask() {
    std::string response_body("");
    HTTPSClient client;
    error err = client.GetJSON(
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

}  // namespace toggl
