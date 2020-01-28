// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CONST_H_
#define SRC_CONST_H_

// used later in the file
#ifndef TOGGL_BUILD_TYPE
#define TOGGL_BUILD_TYPE ""
#endif

#define kOneSecondInMicros 1000000

#define kMaxTimeEntryDurationSeconds 3596400
#define kHTTPClientTimeoutSeconds 30
#define kSyncIntervalRangeSeconds 900
#define kWebsocketRestartRangeSeconds 45
#define kCheckUpdateIntervalSeconds 86400
#define kCheckInAppMessageIntervalSeconds 14400
#define kRequestThrottleSeconds 2
#define kTimerStartInterval 10
#define kTimelineSecondsToKeep 604800
#define kWindowFocusThresholdSeconds 10
#define kAutotrackerThresholdSeconds 10
#define kBetaChannelPercentage 25
#define kTimelineChunkSeconds 900
#define kEnterpriseInstall false
#define kDebianPackage (TOGGL_BUILD_TYPE == std::string("deb"))
#define kTimelineUploadIntervalSeconds 60
#define kTimelineUploadMaxBackoffSeconds (kTimelineUploadIntervalSeconds * 10)  // NOLINT
#define kMaxFileSize 5242880  // 5MB
#define kMaxDurationSeconds (999 * 3600)
#define kMaxTagsPerTimeEntry 50
#define kMinimumAllowedYear 2006
#define kMaximumAllowedYear 2030
#define kMaximumDescriptionLength 3000
#define kTimeComparisonEpsilonMicroSeconds 100000 // 100 ms

#define kLostPasswordURL "https://toggl.com/forgot-password?desktop=true"
#define kGeneralSupportURL "https://support.toggl.com/toggl-on-my-desktop/"
#define kLinuxSupportURL "https://support.toggl.com/toggl-desktop-for-linux/"
#define kMacSupportURL "https://support.toggl.com/toggl-desktop-for-mac-osx/"
#define kTOSURL "https://toggl.com/legal/terms/"
#define kPrivacyPolicyURL "https://toggl.com/legal/privacy/"

#define kContentTypeMultipartFormData "multipart/form-data"
#define kContentTypeApplicationJSON "application/json"

#define kModelAutotrackerRule "autotracker_rule"
#define kModelClient "client"
#define kModelProject "project"
#define kModelSettings "settings"
#define kModelTag "tag"
#define kModelTask "task"
#define kModelTimeEntry "time_entry"
#define kModelTimelineEvent "timeline_event"
#define kModelUser "user"
#define kModelWorkspace "workspace"
#define kModelObmAction "obm_action"
#define kModelObmExperiment "obm_experiment"

#define kChangeTypeInsert "insert"
#define kChangeTypeUpdate "update"
#define kChangeTypeDelete "delete"

#define kAutocompleteItemTE  0
#define kAutocompleteItemTask 1
#define kAutocompleteItemProject 2
#define kAutocompleteItemWorkspace 3

#endif  // SRC_CONST_H_
