// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CONST_H_
#define SRC_CONST_H_

#define kOneSecondInMicros 1000000

#define kIdleThresholdSeconds 300
#define kMaxTimeEntryDurationSeconds 3600000
#define kHTTPClientTimeoutSeconds 10
#define kPeriodicSyncIntervalSeconds 900

#define kCheckUpdateIntervalMicros 86400000000
#define kRequestThrottleMicros 2000000
#define kReminderThrottleMicros 600000000

#define kAutocompleteItemTE  0
#define kAutocompleteItemTask 1
#define kAutocompleteItemProject 2

#define kAPIURL "https://www.toggl.com"
#define kTimelineUploadURL "https://timeline.toggl.com"
#define kWebSocketURL "https://stream.toggl.com"
#define kLostPasswordURL "https://www.toggl.com/forgot-password"
#define kSupportURL "http://support.toggl.com/toggl-on-my-desktop/"

#endif  // SRC_CONST_H_
