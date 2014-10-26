// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIMELINE_CONSTANTS_H_
#define SRC_TIMELINE_CONSTANTS_H_

const unsigned int kTimelineUploadIntervalSeconds = 60;
const unsigned int kTimelineUploadMaxBackoffSeconds =
    kTimelineUploadIntervalSeconds * 10;

const unsigned int kWindowFocusThresholdSeconds = 5;

#endif  // SRC_TIMELINE_CONSTANTS_H_
