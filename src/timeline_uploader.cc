// Copyright 2014 Toggl Desktop developers.

#include "../src/timeline_uploader.h"

#include <sstream>
#include <string>

#include "./timeline_constants.h"
#include "./https_client.h"
#include "./formatter.h"

#include "Poco/Foundation.h"
#include "Poco/Util/Application.h"
#include "Poco/Thread.h"

namespace toggl {

void TimelineUploader::sleep() {
    // Sleep in increments for faster shutdown.
    for (unsigned int i = 0; i < current_upload_interval_seconds_*4; i++) {
        if (uploading_.isStopped()) {
            return;
        }
        Poco::Thread::sleep(250);
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
        backoff();
        return err;
    }

    {
        std::stringstream out;
        out << "Sync of " << batch.Events().size()
            << " event(s) was successful.";
        logger().debug(out.str());
    }

    reset_backoff();

    return timeline_datasource_->DeleteTimelineBatch(batch.Events());
}

error TimelineUploader::upload(TimelineBatch *batch) {
    HTTPSClient client;

    std::stringstream out;
    out << "Uploading " << batch->Events().size()
        << " event(s) of user " << batch->UserID();
    logger().debug(out.str());

    std::string json = convertTimelineToJSON(batch->Events(),
                       batch->DesktopID());
    std::string response_body("");
    return client.PostJSON("/api/v8/timeline",
                           json,
                           batch->APIToken(),
                           "api_token",
                           &response_body);
}

std::string convertTimelineToJSON(
    const std::vector<TimelineEvent> &timeline_events,
    const std::string &desktop_id) {

    Json::Value root;

    for (std::vector<TimelineEvent>::const_iterator i = timeline_events.begin();
            i != timeline_events.end();
            ++i) {
        const TimelineEvent &event = *i;
        // initialize new event node
        Json::Value n;
        // add fields to event node
        if (event.idle) {
            n["idle"] = true;
        } else {
            n["filename"] = event.filename;
            n["title"] = event.title;
        }
        n["start_time"] = Json::Int64(event.start_time);
        n["end_time"] = Json::Int64(event.end_time);
        n["desktop_id"] = desktop_id;
        n["created_with"] = "timeline";

        // Push event node to array
        root.append(n);
    }

    Json::StyledWriter writer;
    return writer.write(root);
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
