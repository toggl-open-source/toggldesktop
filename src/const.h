// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CONST_H_
#define SRC_CONST_H_

#define kOneSecondInMicros 1000000

#define kMaxTimeEntryDurationSeconds 3600000
#define kHTTPClientTimeoutSeconds 30
#define kSyncIntervalRangeSeconds 900
#define kWebsocketRestartRangeSeconds 45
#define kCheckUpdateIntervalSeconds 86400
#define kRequestThrottleSeconds 2
#define kTimerStartInterval 10
#define kTimelineSecondsToKeep 604800
#define kWindowFocusThresholdSeconds 10
#define kBetaChannelPercentage 25
#define kTimelineChunkSeconds 900
#define kEnterpriseInstall false
#define kDebianPackage false

#define kAPIURL "https://www.toggl.com"
#define kTimelineUploadURL "https://timeline.toggl.com"
#define kWebSocketURL "https://stream.toggl.com"
#define kLostPasswordURL "https://www.toggl.com/forgot-password"
#define kSupportURL "http://support.toggl.com/toggl-on-my-desktop/"

#define kContentTypeMultipartFormData "multipart/form-data"
#define kContentTypeApplicationJSON "application/json"

#define kCheckYourSignupError "Signup failed - please check your details. The e-mail might be already taken."  // NOLINT
#define kEndpointGoneError "The API endpoint used by this app is gone. Please contact Toggl support!"  // NOLINT
#define kForbiddenError "Invalid e-mail or password!"
#define kUnsupportedAppError "This version of the app is not supported any more. Please visit Toggl website to download a supported app." // NOLINT
#define kUnauthorizedError "Unauthorized! Please login again."
#define kCannotConnectError "Cannot connect to Toggl"
#define kCannotSyncInTestEnv "Cannot sync in test env"
#define kBackendIsDownError "Backend is down"
#define kBadRequestError "Data that you are sending is not valid/acceptable"
#define kRequestIsNotPossible "Request is not possible"
#define kPaymentRequiredError "Requested action allowed only for Pro workspace. Please upgrade!"  // NOLINT
#define kCannotAccessWorkspaceError "cannot access workspace"
#define kEmailNotFoundCannotLogInOffline "Login failed. Are you online?"  // NOLINT
#define kInvalidPassword "Invalid password"
#define kCannotEstablishProxyConnection "Cannot establish proxy connection"
#define kCertificateVerifyFailed "certificate verify failed"
#define kCheckYourProxySetup "Check your proxy setup"
#define kCheckYourFirewall "Check your firewall"
#define kProxyAuthenticationRequired "Proxy Authentication Required"
#define kCertificateValidationError "Certificate validation error"
#define kUnacceptableCertificate "Unacceptable certificate from www.toggl.com"
#define kCannotUpgradeToWebSocketConnection "Cannot upgrade to WebSocket connection"  // NOLINT
#define kSSLException "SSL Exception"
#define kCannotWriteFile "Cannot write file"
#define kIsSuspended "is suspended"
#define kRequestToServerFailedWithStatusCode403 "Request to server failed with status code: 403"  // NOLINT
#define kMissingWorkspaceID "Missing workspace ID"
#define kCannotContinueDeletedTimeEntry "Cannot continue deleted time entry"
#define kCannotDeleteDeletedTimeEntry "Cannot delete deleted time entry"

#define kOnlineStateOnline 0
#define kOnlineStateNoNetwork 1
#define kOnlineStateBackendDown 2

#define kSyncStateIdle 0
#define kSyncStateWork 1

#endif  // SRC_CONST_H_
