
// Copyright 2015 Toggl Desktop developers

// Collect Toggl Desktop usage using Google Analytics Measurement Protocol
// https://developers.google.com/analytics/devguides/collection/protocol/v1/

#include "../src/analytics.h"

#include <sstream>
#include <istream>

#include "./https_client.h"

#include "Poco/Logger.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/NumberFormatter.h"
#include "Poco/URI.h"

namespace toggl {

const std::string kTrackingID("UA-3215787-27");

void Analytics::TrackAutocompleteUsage(const std::string client_id,
                                       const bool was_using_autocomplete) {
    int value = 0;
    if (was_using_autocomplete) {
        value = 1;
    }
    start(new TrackEvent(client_id, "timer", "start", "was_using_autocomplete",
                         value));
}

void TrackEvent::runTask() {
    Poco::Logger& logger = Poco::Logger::get("TrackEvent");
    try {
        Poco::URI uri("http://www.google-analytics.com/collect");
        uri.addQueryParameter("v", "1");
        uri.addQueryParameter("tid", kTrackingID);
        uri.addQueryParameter("cid", client_id_);
        uri.addQueryParameter("t", "event");
        uri.addQueryParameter("ec", category_);
        uri.addQueryParameter("ea", action_);
        uri.addQueryParameter("el", opt_label_);
        uri.addQueryParameter("ev", Poco::NumberFormatter::format(opt_value_));

        std::string encoded_url("");
        Poco::URI::encode(uri.getPathAndQuery(), "", encoded_url);

        Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
        Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST,
                                   encoded_url,
                                   Poco::Net::HTTPMessage::HTTP_1_1);

        req.set("User-Agent", HTTPSClientConfig::UserAgent());

        // Log out request contents
        std::stringstream request_string;
        req.write(request_string);
        logger.debug(uri.toString());
        logger.debug(request_string.str());

        Poco::Net::HTTPResponse response;
        session.receiveResponse(response);

        std::stringstream ss;
        ss << "Analytics response status code " << response.getStatus()
           << ", content length " << response.getContentLength()
           << ", content type " << response.getContentType();
        if (response.has("Content-Encoding")) {
            ss << ", content encoding " << response.get("Content-Encoding");
        } else {
            ss << ", unknown content encoding";
        }
        logger.debug(ss.str());
    } catch(const Poco::Exception& exc) {
        logger.error(exc.displayText());
    } catch(const std::exception& ex) {
        logger.error(ex.what());
    } catch(const std::string& ex) {
        logger.error(ex);
    }
}

}  // namespace toggl
