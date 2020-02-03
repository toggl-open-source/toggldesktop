// Copyright 2020 Toggl Desktop developers

#ifndef ERROR_H
#define ERROR_H

#include "logger.h"

#include <string>
#include <map>

#include <Poco/Net/HTTPResponse.h>

namespace toggl {

class Error {
public:
    enum Code {
        kInvalidError = -1,
        kNoError = 0,

        kOverMaxDurationError,
        kMaxTagsPerTimeEntryError,
        kInvalidStartTimeError,
        kInvalidStopTimeError,
        kInvalidDateError,
        kStartNotBeforeStopError,
        kMaximumDescriptionLengthError,

        kCheckYourSignupError,
        kEndpointGoneError,
        kForbiddenError,
        kUnsupportedAppError,
        kUnauthorizedError,
        kCannotConnectError,
        kCannotSyncInTestEnv,
        kBackendIsDownError,
        kBadRequestError,
        kRequestIsNotPossible,
        kPaymentRequiredError,
        kCannotAccessWorkspaceError,
        kEmailNotFoundCannotLogInOffline,
        kEmptyEmail,
        kEmptyPassword,
        kInvalidPassword,
        kMissingApiToken,
        kCannotEstablishProxyConnection,
        kCertificateVerifyFailed,
        kCheckYourProxySetup,
        kCheckYourFirewall,
        kProxyAuthenticationRequired,
        kCertificateValidationError,
        kUnacceptableCertificate,
        kMissingCACertificate,
        kCannotUpgradeToWebSocketConnection,
        kSSLException,
        kRateLimit,
        kCannotWriteFile,
        kIsSuspended,
        kRequestToServerFailedWithStatusCode403,
        kMissingWorkspaceID,
        kCannotContinueDeletedTimeEntry,
        kCannotDeleteDeletedTimeEntry,
        kErrorRuleAlreadyExists,
        kPleaseSelectAWorkspace,
        kClientNameMustNotBeEmpty,
        kProjectNameMustNotBeEmpty,
        // kProjectNameAlready, // huh, what is this?
        kProjectNameAlreadyExists,
        kClientNameAlreadyExists,
        kDatabaseDiskMalformed,
        kMissingWS,
        kOutOfDatePleaseUpgrade,
        kThisEntryCantBeSavedPleaseAdd,

        // these were hardcoded as strings
        kFailedToParseData,
        kExportWithoutGUID,
        kDeleteRuleWithoutID,
        kNameAlreadyTaken, // is this the same as kClientNameAlreadyExists?
        kCannotModifyWorkspaceData,
        kMissingFeedbackTopic,
        kMissingFeedbackDetails,
        kMissingUICallbacks,
        kCannotCheckForUpdates,
        kInvalidUpdateChannel,
        kCannotChangeLockedTE,
        kInvalidTimeFormat,
        kCannotCreateObmAction,
        kCannotStartAutotrackerEvent,

        kTimeEntryNotFound,
        kTimeEntryCreatedWithInvalid,
        kCannotAccessProjectError, // should these be a single error?
        kCannotAccessTaskError,
        kBillableIsAPremiumFeature, // should this be the same as kPaymentRequiredError?

        kMissingArgument, // I don't think more info is necessary about WHAT argument is missing

        kTimeEntryWouldBeLockedAfterProjectChange,

        kOfflineLoginMissingEmail,
        kOfflineLoginMissingPassword,
        kCannotLoadUserDataWhenLoggedOut,
        kCannotLoadUserDataWithoutApiToken,
        kCannotSaveUserDataWhenLoggedOut,
        kCannotSaveUserDataWithoutApiToken,

        kBackendChangedTheID,
        kClientIsNotPresentInWorkspace,
        kOnlyAdminsCanChangeVisibility,

        kCannotDecryptOfflineDataWithoutEmail,
        kCannotDecryptOfflineDataWithoutPassword,
        kCannotDecryptOfflineDataWhenEmpty,
        kCannotEnableOfflineLoginWithoutEmail,
        kCannotEnableOfflineLoginWithoutPassword,
        kCannotEnableOfflineLoginWithoutApiToken,
        kOfflineDecryptionFailed,

        kWebsocketClosedConnection,


        // REMOVE THESE WHEN COMMITTING
        REMOVE_LATER_BATCH_UPDATE_ERROR,
        REMOVE_LATER_EXCEPTION_HANDLER,
        REMOVE_LATER_NETWORK_RESPONSE,
        REMOVE_LATER_DATABASE_LAST_ERROR,
        REMOVE_LATER_DATABASE_STORE_ERROR,
        REMOVE_LATER_CLIENT_NOT_PRESENT_WITH_ID,

        kLastErrorCode
    };
    inline static const std::map<Code, std::string> values {
        { kNoError, {} },

        { kOverMaxDurationError, "Max allowed duration per 1 time entry is 999 hours" },
        { kMaxTagsPerTimeEntryError, "Tags are limited to 50 per task" },
        { kInvalidStartTimeError, "Start time year must be between 2006 and 2030" },
        { kInvalidStopTimeError, "Stop time year must be between 2006 and 2030" },
        { kInvalidDateError, "Date year must be between 2006 and 2030" },
        { kStartNotBeforeStopError, "Stop time must be after start time" },
        { kMaximumDescriptionLengthError, "Maximum length for description (3000 chars) exceeded" },

        { kCheckYourSignupError, "Signup failed - please check your details. The e-mail might be already taken." },
        { kEndpointGoneError, "The API endpoint used by this app is gone. Please contact Toggl support!" },
        { kForbiddenError, "Invalid e-mail or password!" },
        { kUnsupportedAppError, "This version of the app is not supported any more. Please visit Toggl website to download a supported app." },
        { kUnauthorizedError, "Unauthorized! Please login again." },
        { kCannotConnectError, "Cannot connect to Toggl" },
        { kCannotSyncInTestEnv, "Cannot sync in test env" },
        { kBackendIsDownError, "Backend is down" },
        { kBadRequestError, "Data that you are sending is not valid/acceptable" },
        { kRequestIsNotPossible, "Request is not possible" },
        { kPaymentRequiredError, "Requested action allowed only for Non-Free workspaces. Please upgrade!" },
        { kCannotAccessWorkspaceError, "cannot access workspace" },
        { kEmailNotFoundCannotLogInOffline, "Login failed. Are you online?" },
        { kInvalidPassword, "Invalid password" },
        { kMissingApiToken, "cannot pull user data without API token" },
        { kCannotEstablishProxyConnection, "Cannot establish proxy connection" },
        { kCertificateVerifyFailed, "certificate verify failed" },
        { kCheckYourProxySetup, "Check your proxy setup" },
        { kCheckYourFirewall, "Check your firewall" },
        { kProxyAuthenticationRequired, "Proxy Authentication Required" },
        { kCertificateValidationError, "Certificate validation error" },
        { kUnacceptableCertificate, "Unacceptable certificate from www.toggl.com" },
        { kMissingCACertificate, "Missing CA certifcate, cannot start Websocket" },
        { kCannotUpgradeToWebSocketConnection, "Cannot upgrade to WebSocket connection" },
        { kSSLException, "SSL Exception" },
        { kRateLimit, "Too many requests, sync delayed by 1 minute" },
        { kCannotWriteFile, "Cannot write file" },
        { kIsSuspended, "is suspended" },
        { kRequestToServerFailedWithStatusCode403, "Request to server failed with status code: 403" },
        { kMissingWorkspaceID, "Missing workspace ID" },
        { kCannotContinueDeletedTimeEntry, "Cannot continue deleted time entry" },
        { kCannotDeleteDeletedTimeEntry, "Cannot delete deleted time entry" },
        { kErrorRuleAlreadyExists, "rule already exists" },
        { kPleaseSelectAWorkspace, "Please select a workspace" },
        { kClientNameMustNotBeEmpty, "Client name must not be empty" },
        { kProjectNameMustNotBeEmpty, "Project name must not be empty" },
        //{ kProjectNameAlready, "Project name already" },
        { kProjectNameAlreadyExists, "Project name already exists" },
        { kClientNameAlreadyExists, "Client name already exists" },
        { kDatabaseDiskMalformed, "The database disk image is malformed" },
        { kMissingWS, "You no longer have access to your last workspace" },
        { kOutOfDatePleaseUpgrade, "Your version of Toggl Desktop is out of date, please upgrade!" },
        { kThisEntryCantBeSavedPleaseAdd, "This entry can't be saved - please add" },

        { kFailedToParseData, "Failed to parse data string" },
        { kExportWithoutGUID, "Cannot export model to batch update without a GUID" },
        { kDeleteRuleWithoutID, "cannot delete rule without an ID" },
        { kNameAlreadyTaken, "Name has already been taken" },
        { kCannotModifyWorkspaceData, "User cannot add or edit clients in workspace" },
        { kMissingFeedbackTopic, "Missing topic" },
        { kMissingFeedbackDetails, "Missing details" },
        { kMissingUICallbacks, "UI is not properly wired up!"},
        { kCannotCheckForUpdates, "This version cannot check for updates. This has been probably already fixed. Please check https://toggl.com/toggl-desktop/ for a newer version." },
        { kInvalidUpdateChannel, "Invalid update channel" },
        { kCannotChangeLockedTE, "Cannot change locked time entry." },
        { kInvalidTimeFormat, "invalid time format" },

        { kTimeEntryNotFound, "Time entry not found" },
        { kTimeEntryCreatedWithInvalid, "created_with needs to be provided an a valid string" },
        { kCannotAccessProjectError, "User cannot access the selected project" },
        { kCannotAccessTaskError, "User cannot access selected task" },
        { kBillableIsAPremiumFeature, "Billable is a premium feature" },

        { kTimeEntryWouldBeLockedAfterProjectChange, "Cannot change project: would end up with locked time entry" },

        { kOfflineLoginMissingEmail, "cannot login offline without an e-mail" },
        { kOfflineLoginMissingPassword, "cannot login offline without a password" },
        { kCannotLoadUserDataWhenLoggedOut, "cannot load user data when logged out" },
        { kCannotLoadUserDataWithoutApiToken, "cannot pull user data without API token" },
        { kCannotSaveUserDataWhenLoggedOut, "cannot push changes when logged out"},
        { kCannotSaveUserDataWithoutApiToken, "cannot push changes without API token"},

        { kBackendChangedTheID, "Backend has changed the ID of the entry" },
        { kClientIsNotPresentInWorkspace, "client is in another workspace" },
        { kOnlyAdminsCanChangeVisibility, "Only admins can change project visibility" },

        { kCannotDecryptOfflineDataWithoutEmail, "cannot decrypt offline data without an e-mail" },
        { kCannotDecryptOfflineDataWithoutPassword, "cannot decrypt offline data without a password" },
        { kCannotDecryptOfflineDataWhenEmpty, "cannot decrypt empty string" },
        { kCannotEnableOfflineLoginWithoutEmail, "cannot enable offline login without an e-mail" },
        { kCannotEnableOfflineLoginWithoutPassword, "cannot enable offline login without a password" },
        { kCannotEnableOfflineLoginWithoutApiToken, "cannot enable offline login without an API token" },
        { kOfflineDecryptionFailed, "offline login encryption failed" },

        { kWebsocketClosedConnection, "WebSocket closed the connection" },
    };

    Error(Code c) : code_(c) {}
    bool operator==(const Code &c) const;
    bool operator!=(const Code &c) const;
    bool operator==(const Error &o) const;
    bool operator!=(const Error &o) const;
    bool operator<(const Error &rhs) const;
    Error &operator=(Code &c);

    bool IsNoError() const;
    bool isValid() const;
    bool IsNetworkingError() const;
    bool IsUserError() const;

    std::string MakeErrorActionable() const;
    std::string String() const;

    static Error fromString(const std::string &message);

    static Error fromHttpStatus(Poco::Net::HTTPResponse::HTTPStatus http_status);
    static Error fromHttpStatus(int64_t http_status);
    static Error fromServerError(const std::string &message);
private:
    Code code_;
};

std::ostream &operator<<(std::ostream &out, const Error &t);

} // namespace toggl

#endif // ERROR_H
