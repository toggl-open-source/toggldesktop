// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CONST_H_
#define SRC_CONST_H_

#define kOneSecondInMicros 1000000

#define kMaxTimeEntryDurationSeconds 3600000
#define kHTTPClientTimeoutSeconds 10
#define kPeriodicSyncIntervalSeconds 900

// 86400 seconds = 24 hours
#define kCheckUpdateIntervalSeconds 86400
#define kRequestThrottleSeconds 2
#define kReminderThrottleSeconds 600

#define kAutocompleteItemTE  0
#define kAutocompleteItemTask 1
#define kAutocompleteItemProject 2
#define kAutocompleteItemWorkspace 3

#define kAPIURL "https://www.toggl.com"
#define kTimelineUploadURL "https://timeline.toggl.com"
#define kWebSocketURL "https://stream.toggl.com"
#define kLostPasswordURL "https://www.toggl.com/forgot-password"
#define kSupportURL "http://support.toggl.com/toggl-on-my-desktop/"

#define kEndpointGoneError "The API endpoint used by this app is gone. Please contact Toggl support!"  // NOLINT

#endif  // SRC_CONST_H_
