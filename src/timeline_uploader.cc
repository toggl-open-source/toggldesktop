// Copyright (c) 2013 Toggl

#include "./timeline_uploader.h"

#include "./timeline_constants.h"

#include <libjson.h>

#include <sstream>
#include <string>

#include "Poco/Foundation.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Util/Application.h"
#include "Poco/URI.h"
#include "Poco/Thread.h"
#include "Poco/StreamCopier.h"

void TimelineUploader::initialize(Poco::Util::Application&) {
    start_uploading();
}

void TimelineUploader::uninitialize() {
    stop_uploading();
}

void TimelineUploader::start_uploading() {
    if (!upload_token_.empty() && user_id_ > 0) {
        std::stringstream out;
        out << "start_uploading user_id = " << user_id_;
        Poco::Logger &logger = Poco::Logger::get("timeline_uploader");
        logger.debug(out.str());

        if (!uploading_.isRunning()) {
            uploading_.start();
        }
    }
}

void TimelineUploader::stop_uploading() {
    if (uploading_.isRunning()) {
        uploading_.stop();
        uploading_.wait();
    }
}

void TimelineUploader::handleConfigureNotification(
        ConfigureNotification* notification) {
    std::stringstream out;
    out << "handleConfigureNotification, user_id = " << notification->user_id;
    Poco::Logger &logger = Poco::Logger::get("timeline_uploader");
    logger.debug(out.str());
    if (user_id_ != notification->user_id) {
        stop_uploading();
    }
    user_id_ = notification->user_id;
    upload_token_ = notification->upload_token;
    start_uploading();
}

void TimelineUploader::handleTimelineBatchReadyNotification(
        TimelineBatchReadyNotification *notification) {
    Poco::Logger &logger = Poco::Logger::get("timeline_uploader");
    logger.debug("handleTimelineBatchReadyNotification");

    poco_assert(user_id_ == notification->user_id);
    poco_assert(!notification->desktop_id.empty());

    std::stringstream out;
    out << "Sync " << notification->batch.size() << " timeline event(s).";
    logger.debug(out.str());

    if (sync(user_id_, upload_token_, notification->batch,
            notification->desktop_id)) {
        if (!notification->batch.empty()) {
            std::stringstream out;
            out << "Sync of " << notification->batch.size()
                << " event(s) was successful.";
            logger.information(out.str());
        }

        if (!notification->batch.empty()) {
            Poco::NotificationCenter& nc =
                Poco::NotificationCenter::defaultCenter();
            DeleteTimelineBatchNotification response(notification->batch);
            Poco::AutoPtr<DeleteTimelineBatchNotification> ptr(&response);
            nc.postNotification(ptr);
        }
    } else {
        std::stringstream out;
        out << "Sync of " << notification->batch.size() << " event(s) failed.";
        logger.error(out.str());
    }
}

std::string TimelineUploader::convert_timeline_to_json(
        const std::vector<TimelineEvent> &timeline_events,
        const std::string &desktop_id) {
    // initialize a new JSON array
    JSONNODE *c = json_new(JSON_ARRAY);
    for (std::vector<TimelineEvent>::const_iterator i = timeline_events.begin();
            i != timeline_events.end();
            ++i) {
        const TimelineEvent &event = *i;
        // initialize new event node
        JSONNODE *n = json_new(JSON_NODE);
        // add fields to event node
        if (event.idle) {
            json_push_back(n, json_new_b("idle", true));
        } else {
            json_push_back(n, json_new_a("filename", event.filename.c_str()));
            json_push_back(n, json_new_a("title", event.title.c_str()));
        }
        json_push_back(n,
            json_new_i("start_time", (json_int_t)event.start_time));
        json_push_back(n,
            json_new_i("end_time", (json_int_t)event.end_time));
        json_push_back(n, json_new_a("desktop_id", desktop_id.c_str()));
        json_push_back(n, json_new_a("created_with", "timeline"));
        // Push event node to array
        json_push_back(c, n);
    }
    json_char *jc = json_write_formatted(c);
    std::string json(jc);
    json_free(jc);
    json_delete(c);
    return json;
}

void TimelineUploader::upload_loop_activity() {
    while (!uploading_.isStopped()) {
        std::stringstream out;
        out << "upload_loop_activity (current interval "
            << current_upload_interval_seconds_ << "s)";
        Poco::Logger &logger = Poco::Logger::get("timeline_uploader");
        logger.debug(out.str());

        {
            // Request data for upload.
            Poco::NotificationCenter& nc =
                Poco::NotificationCenter::defaultCenter();
            CreateTimelineBatchNotification notification(user_id_);
            Poco::AutoPtr<CreateTimelineBatchNotification> ptr(&notification);
            nc.postNotification(ptr);
        }

        // Sleep in increments for faster shutdown.
        for (unsigned int i = 0; i < current_upload_interval_seconds_; i++) {
            if (uploading_.isStopped()) {
                break;
            }
            Poco::Thread::sleep(1000);
        }
    }
}

void TimelineUploader::json_to_timeline_settings(const std::string &json,
        bool &record_timeline) {
    JSONNODE *n = json_parse(json.c_str());
    JSONNODE_ITERATOR i = json_begin(n);
    JSONNODE_ITERATOR e = json_end(n);
    while (i != e) {
        json_char *node_name = json_name(*i);
        if (strcmp(node_name, "record_timeline") == 0) {
            json_char *node_value = json_as_string(*i);
            record_timeline = (strcmp(node_value, "true") == 0);
        }
        ++i;
    }
    json_delete(n);
}

bool TimelineUploader::sync(const unsigned int user_id,
                            const std::string &upload_token,
        const std::vector<TimelineEvent> &timeline_events,
        const std::string &desktop_id) {
    Poco::Logger &logger = Poco::Logger::get("timeline_uploader");

    try {
        std::stringstream out;
        out << "Uploading " << timeline_events.size()
            << " event(s) of user " << user_id
            << " to " << upload_host_ << kTimelineUploadPath;
        logger.debug(out.str());

        poco_assert(user_id > 0);
        poco_assert(!upload_token.empty());
        std::string json =
            convert_timeline_to_json(timeline_events, desktop_id);

        logger.debug(json);

        const Poco::URI uri(upload_host_);

        // FIXME: must verify server certificate.
        const Poco::Net::Context::Ptr context(new Poco::Net::Context(
            Poco::Net::Context::CLIENT_USE, "", "", "",
            Poco::Net::Context::VERIFY_NONE, 9, false,
            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"));

        Poco::Net::HTTPSClientSession session(
            uri.getHost(), uri.getPort(), context);
        session.setKeepAlive(false);

        Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST,
            kTimelineUploadPath, Poco::Net::HTTPMessage::HTTP_1_1);
        req.setContentType("application/json");
        req.setKeepAlive(false);
        req.setContentLength(json.length());

        logger.debug("Sending request..");

        Poco::Net::HTTPBasicCredentials cred(upload_token, "upload_token");
        cred.authenticate(req);
        session.sendRequest(req) << json << std::flush;

        // Log out request contents
        std::stringstream request_string;
        req.write(request_string);
        logger.debug(request_string.str());

        logger.debug("Request sent. Receiving response..");

        // Receive response
        Poco::Net::HTTPResponse response;
        std::istream& is = session.receiveResponse(response);

        // Log out response contents
        std::stringstream response_string;
        response_string << "Response received: "
            << response.getStatus() << " " << response.getReason();
        logger.debug(response_string.str());

        if ((response.getStatus() != 202) && (response.getStatus() != 200)) {
            exponential_backoff();
            return false;
        }

        // Read request body
        std::ostringstream body;
        Poco::StreamCopier::copyStream(is, body);

        reset_backoff();

        // Parse JSON
        json = body.str();
        logger.debug(json);

        bool record_timeline = false;
        if (json.empty()) {
            logger.warning("Empty response from server: timeline is disabled?");
        } else {
            json_to_timeline_settings(json, record_timeline);

            Poco::NotificationCenter& nc =
                Poco::NotificationCenter::defaultCenter();
            UserTimelineSettingsNotification notification(
                user_id_, record_timeline);
            Poco::AutoPtr<UserTimelineSettingsNotification> ptr(&notification);
            nc.postNotification(ptr);
        }
    } catch(const Poco::Exception& exc) {
        logger.error(exc.displayText());
        exponential_backoff();
        return false;
    } catch(const std::exception& ex) {
        logger.error(ex.what());
        exponential_backoff();
        return false;
    } catch(const std::string& ex) {
        logger.error(ex);
        exponential_backoff();
        return false;
    }

    return true;
}

void TimelineUploader::exponential_backoff() {
    Poco::Logger &logger = Poco::Logger::get("timeline_uploader");
    logger.warning("exponential_backoff");
    current_upload_interval_seconds_ *= 2;
    if (current_upload_interval_seconds_ > max_upload_interval_seconds_) {
        logger.warning("Max upload interval reached.");
        current_upload_interval_seconds_ = max_upload_interval_seconds_;
    }
    std::stringstream out;
    out << "Upload interval set to " << current_upload_interval_seconds_ << "s";
    logger.debug(out.str());
}

void TimelineUploader::reset_backoff() {
    Poco::Logger &logger = Poco::Logger::get("timeline_uploader");
    logger.debug("reset_backoff");
    current_upload_interval_seconds_ = upload_interval_seconds_;
}

void TimelineUploader::defineOptions(Poco::Util::OptionSet& options) { // NOLINT
    options.addOption(
        Poco::Util::Option("upload_interval", "",
                "upload and timeline settings check interval")
            .required(false)
            .repeatable(false)
            .argument("seconds")
            .callback(Poco::Util::OptionCallback<TimelineUploader>(this,
                &TimelineUploader::handleConfigUploadInterval)));

    options.addOption(
        Poco::Util::Option("upload_host", "",
                "API host, must be HTTPS, for example: https://localhost:8080")
            .required(false)
            .repeatable(false)
            .argument("URL")
            .callback(Poco::Util::OptionCallback<TimelineUploader>(this,
                &TimelineUploader::handleConfigUploadHost)));
}

void TimelineUploader::handleConfigUploadInterval(const std::string& name,
        const std::string& value) {
    Poco::Logger &logger = Poco::Logger::get("timeline_uploader");
    logger.debug("handleConfigUploadInterval");
    stop_uploading();
    upload_interval_seconds_ = atoi(value.c_str());
    start_uploading();
}

void TimelineUploader::handleConfigUploadHost(const std::string& name,
        const std::string& value) {
    Poco::Logger &logger = Poco::Logger::get("timeline_uploader");
    logger.debug("handleConfigUploadHost");
    stop_uploading();
    upload_host_ = value;
    start_uploading();
}
