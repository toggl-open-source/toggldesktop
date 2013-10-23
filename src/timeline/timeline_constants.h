
// Copyright (c) 2013 Toggl

#ifndef SRC_TIMELINE_TIMELINE_CONSTANTS_H_
#define SRC_TIMELINE_TIMELINE_CONSTANTS_H_

const unsigned int kTimelineUploadIntervalSeconds = 60;
const unsigned int kTimelineUploadMaxBackoffSeconds =
    kTimelineUploadIntervalSeconds * 10;
static char const kTimelineUploadHost[] = "https://www.toggl.com";
static char const kTimelineUploadPath[] = "/api/v8/timeline_with_upload_token";

const unsigned int kWindowFocusThresholdSeconds = 5;
const unsigned int kWindowChangeRecordingIntervalMillis = 500;

#endif  // SRC_TIMELINE_TIMELINE_CONSTANTS_H_
