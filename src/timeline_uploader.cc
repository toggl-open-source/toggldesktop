// Copyright 2014 Toggl Desktop developers.

#include "timeline_uploader.h"

#include <sstream>
#include <string>

#include "util/formatter.h"
#include "https_client.h"
#include "urls.h"

#include <Poco/Foundation.h>
#include <Poco/Thread.h>
#include <Poco/Util/Application.h>

#include <json/json.h>  // NOLINT

namespace toggl {

Logger TimelineUploader::logger() const {
    return { "timeline_uploader" };
}

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
    logger().debug("upload_loop_activity (current interval ", current_upload_interval_seconds_, "s)");

    if (uploading_.isStopped()) {
        return noError;
    }

    TimelineBatch batch;
    error err = timeline_datasource_->CreateCompressedTimelineBatchForUpload(
        &batch);
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

    logger().debug("Sync of ", batch.Events().size(), " event(s) was successful.");

    reset_backoff();

    return timeline_datasource_->MarkTimelineBatchAsUploaded(batch.Events());
}

error TimelineUploader::upload(TimelineBatch *batch) {
    TogglClient client;

    logger().debug("Uploading ", batch->Events().size(), " event(s) of user ", batch->UserID());

    std::string json = convertTimelineToJSON(
        batch->Events(),
        batch->DesktopID());
    logger().trace(json);

    // Not implemented in v9 as of 12.05.2017
    HTTPRequest req;
    req.host = urls::TimelineUpload();
    req.relative_url = "/api/v8/timeline";
    req.payload = json;
    req.basic_auth_username = batch->APIToken();
    req.basic_auth_password = "api_token";

    return client.Post(req).err;
}

std::string convertTimelineToJSON(
    const std::vector<TimelineEvent> &timeline_events,
    const std::string &desktop_id) {

    Json::Value root;

    for (std::vector<TimelineEvent>::const_iterator i = timeline_events.begin();
            i != timeline_events.end();
            ++i) {
        TimelineEvent event = *i;
        Json::Value n = event.SaveToJSON();
        n["desktop_id"] = desktop_id;
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
    logger().debug("Upload interval set to ", current_upload_interval_seconds_, "s");
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
    } catch(const std::string & ex) {
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
    } catch(const std::string & ex) {
        return ex;
    }
    return noError;
}

}  // namespace toggl
