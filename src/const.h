// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CONST_H_
#define SRC_CONST_H_

#define kIdleThresholdSeconds 300

#define kRequestThrottleMicros 2000000

#define kAutocompleteItemTE  0
#define kAutocompleteItemTask 1
#define kAutocompleteItemProject 2

#define kAPIURL "https://www.toggl.com"
#define kTimelineUploadURL "https://timeline.toggl.com"
#define kWebSocketURL "https://stream.toggl.com"
#define kLostPasswordURL "https://www.toggl.com/lost_passwords/new"
#define kSupportURL "http://support.toggl.com/toggl-on-my-desktop/"
#define kTogglWebsiteURL "https://new.toggl.com/"

#define kCheckUpdateIntervalMicros 86400000000

#define kMaxTimeEntryDurationSeconds 3600000

#endif  // SRC_CONST_H_
