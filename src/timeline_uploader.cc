// Copyright 2014 Toggl Desktop developers.

#include "./timeline_uploader.h"

#include <libjson.h>

#include <sstream>
#include <string>

#include "./timeline_constants.h"
#include "./https_client.h"
#include "./formatter.h"

#include "Poco/Foundation.h"
#include "Poco/Util/Application.h"
#include "Poco/Thread.h"

namespace toggl {
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
            json_push_back(n, json_new_a(
                "filename",
                Formatter::EscapeJSONString(event.filename).c_str()));
            json_push_back(n, json_new_a(
                "title",
                Formatter::EscapeJSONString(event.title).c_str()));
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

void TimelineUploader::sleep() {
    // Sleep in increments for faster shutdown.
    for (unsigned int i = 0; i < current_upload_interval_seconds_; i++) {
        if (uploading_.isStopped()) {
            return;
        }
        Poco::Thread::sleep(1000);
    }
}

void TimelineUploader::upload_loop_activity() {
    while (!uploading_.isStopped()) {
        error err = process();
        if (err != noError) {
            logger().error(err);
        }
        sleep();
    }
}

error TimelineUploader::process() {
    {
        std::stringstream out;
        out << "upload_loop_activity (current interval "
            << current_upload_interval_seconds_ << "s)";
        logger().debug(out.str());
    }

    if (uploading_.isStopped()) {
        return noError;
    }

    TimelineBatch batch;
    error err = timeline_datasource_->CreateTimelineBatch(&batch);
    if (err != noError) {
        return err;
    }

    if (!batch.Events().size()) {
        return noError;
    }

    if (uploading_.isStopped()) {
        return noError;
    }

    err = upload(&batch);
    if (err != noError) {
        return err;
    }

    {
        std::stringstream out;
        out << "Sync of " << batch.Events().size()
            << " event(s) was successful.";
        logger().debug(out.str());
    }

    return timeline_datasource_->DeleteTimelineBatch(batch.Events());
}

error TimelineUploader::upload(TimelineBatch *batch) {
    HTTPSClient client;

    std::stringstream out;
    out << "Uploading " << batch->Events().size()
        << " event(s) of user " << batch->UserID();
    logger().debug(out.str());

    std::string json = convert_timeline_to_json(batch->Events(),
                       batch->DesktopID());
    std::string response_body("");
    return client.PostJSON("/api/v8/timeline",
                           json,
                           batch->APIToken(),
                           "api_token",
                           &response_body);
}

void TimelineUploader::backoff() {
    logger().warning("backoff");
    current_upload_interval_seconds_ *= 2;
    if (current_upload_interval_seconds_ > kTimelineUploadMaxBackoffSeconds) {
        logger().warning("Max upload interval reached.");
        current_upload_interval_seconds_ = kTimelineUploadMaxBackoffSeconds;
    }
    std::stringstream out;
    out << "Upload interval set to " << current_upload_interval_seconds_ << "s";
    logger().debug(out.str());
}

void TimelineUploader::reset_backoff() {
    logger().debug("reset_backoff");
    current_upload_interval_seconds_ = kTimelineUploadIntervalSeconds;
}

error TimelineUploader::start() {
    try {
        uploading_.start();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error TimelineUploader::Shutdown() {
    try {
        if (uploading_.isRunning()) {
            uploading_.stop();
            uploading_.wait();
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

}  // namespace toggl
