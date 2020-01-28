// Copyright 2020 Toggl Desktop developers

#include "error.h"
#include "util/logger.h"
#include "const.h"

#include <set>

namespace toggl {

std::string Error::String() const {
    try {
        return values.at(code_);
    }
    catch (std::out_of_range &) {
        return "Unknown error (" + std::to_string(code_) + ")";
    }
}

Error Error::fromString(const std::string &message) {
    for (int i = kNoError; i < kLastErrorCode; i++) {
        auto c = static_cast<Code>(i);
        if (Error(c).String() == message)
            return c;
    }
    return kInvalidError;
}

bool Error::operator==(const Error::Code &c) const { return code_ == c; }
bool Error::operator!=(const Error::Code &c) const { return code_ != c; }
bool Error::operator==(const Error &o) const { return code_ == o.code_; }
bool Error::operator!=(const Error &o) const { return code_ != o.code_; }

bool Error::operator<(const Error &rhs) const {
    return code_ < rhs.code_;
}

Error &Error::operator=(Error::Code &c) {
    code_ = c;
    return *this;
}

bool Error::IsNoError() const {
    return code_ == kNoError;
}

bool Error::isValid() const {
    return code_ >= kNoError;
}

bool Error::IsNetworkingError() const {
    if (IsNoError()) {
        return false;
    }
    if (code_ == kCannotConnectError) {
        return true;
    }
    if (code_ == kBackendIsDownError) {
        return true;
    }
    if (code_ == kCannotEstablishProxyConnection) {
        return true;
    }
    if (code_ == kCertificateVerifyFailed) {
        return true;
    }
    if (code_ == kProxyAuthenticationRequired) {
        return true;
    }
    if (code_ == kCertificateValidationError) {
        return true;
    }
    if (code_ == kUnacceptableCertificate) {
        return true;
    }
    if (code_ == kCannotUpgradeToWebSocketConnection) {
        return true;
    }
    if (code_ == kSSLException) {
        return true;
    }
    /* TODO
        if (err.find("Cannot assign requested address") != std::string::npos) {
            return true;
        }
        if (err.find("Host not found") != std::string::npos) {
            return true;
        }
        if (err.find("No message received") != std::string::npos) {
            return true;
        }
        if (err.find("Connection refused") != std::string::npos) {
            return true;
        }
        if (err.find("Connection timed out") != std::string::npos) {
            return true;
        }
        if (err.find("connect timed out") != std::string::npos) {
            return true;
        }
        if (err.find("SSL connection unexpectedly closed") != std::string::npos) {
            return true;
        }
        if (err.find("Network is down") != std::string::npos) {
            return true;
        }
        if (err.find("Network is unreachable") != std::string::npos) {
            return true;
        }
        if (err.find("Host is down") != std::string::npos) {
            return true;
        }
        if (err.find("No route to host") != std::string::npos) {
            return true;
        }
        if ((err.find("I/O error: 1") != std::string::npos)
                && (err.find(":443") != std::string::npos)) {
            return true;
        }
        if (err.find("The request timed out") != std::string::npos) {
            return true;
        }
        if (err.find("Could not connect to the server") != std::string::npos) {
            return true;
        }
        if (err.find("Connection reset by peer") != std::string::npos) {
            return true;
        }
        if (err.find("The Internet connection appears to be offline")
                != std::string::npos) {
            return true;
        }
        if (err.find("Timeout") != std::string::npos) {
            return true;
        }
        if (err.find("An internal server error occurred.") != std::string::npos) {
            return true;
        }
        */
    return false;
}

bool Error::IsUserError() const {
    if (IsNoError()) {
        return false;
    }
    if (code_ == kErrorRuleAlreadyExists) {
        return true;
    }
    if (code_ == kCheckYourSignupError) {
        return true;
    }
    if (code_ == kEmailNotFoundCannotLogInOffline) {
        return true;
    }
    if (code_ == kInvalidPassword) {
        return true;
    }
    if (code_ == kPaymentRequiredError) {
        return true;
    }
    if (code_ == kBadRequestError) {
        return true;
    }
    if (code_ == kUnauthorizedError) {
        return true;
    }
    if (code_ == kCannotWriteFile) {
        return true;
    }
    if (code_ == kIsSuspended) {
        return true;
    }
    if (code_ == kRequestToServerFailedWithStatusCode403) {
        return true;
    }
    if (code_ == kUnsupportedAppError) {
        return true;
    }
    if (code_ == kMissingWorkspaceID) {
        return true;
    }
    if (code_ == kEndpointGoneError) {
        return true;
    }
    if (code_ == kCannotAccessWorkspaceError) {
        return true;
    }
    if (code_ == kCannotSyncInTestEnv) {
        return true;
    }
    if (code_ == kCannotContinueDeletedTimeEntry) {
        return true;
    }
    if (code_ == kCannotDeleteDeletedTimeEntry) {
        return true;
    }
    if (code_ == kPleaseSelectAWorkspace) {
        return true;
    }
    if (code_ == kClientNameMustNotBeEmpty) {
        return true;
    }
    if (code_ == kProjectNameMustNotBeEmpty) {
        return true;
    }
    if (code_ == kClientNameAlreadyExists) {
        return true;
    }
    if (code_ == kProjectNameAlreadyExists) {
        return true;
    }
    if (code_ == kThisEntryCantBeSavedPleaseAdd) {
        return true;
    }
    /* TODO
        if (err.find("File not found") != std::string::npos) {
            return true;
        }
        if (err.find("SSL context exception") != std::string::npos) {
            return true;
        }
        if (err.find("Access to file denied") != std::string::npos) {
            return true;
        }
        if (err.find("Stop time must be after start time")
                != std::string::npos) {
            return true;
        }
        if (err.find("Invalid e-mail or password") != std::string::npos) {
            return true;
        }
        if (err.find("Maximum length for description") != std::string::npos) {
            return true;
        }
        if (err.find("Start time year must be between 2010 and 2100")
                != std::string::npos) {
            return true;
        }
        if (err.find("Password should be at least") != std::string::npos) {
            return true;
        }
        if (err.find("User with this email already exists") !=
                std::string::npos) {
            return true;
        }
        if (err.find("Invalid e-mail") != std::string::npos) {
            return true;
        }
        */
    return false;
}

std::string Error::MakeErrorActionable() const {
    if (IsNoError()) {
        return String();
    }
    if (code_ == kCannotEstablishProxyConnection) {
        return Error(kCheckYourProxySetup).String();
    }
    if (code_ == kCertificateVerifyFailed) {
        return Error(kCheckYourFirewall).String();
    }
    if (code_ == kProxyAuthenticationRequired) {
        return Error(kCheckYourProxySetup).String();
    }
    if (code_ == kCertificateValidationError) {
        return Error(kCheckYourFirewall).String();
    }
    if (code_ == kUnacceptableCertificate) {
        return Error(kCheckYourFirewall).String();
    }
    if (code_ == kCannotUpgradeToWebSocketConnection) {
        return Error(kCheckYourFirewall).String();
    }
    if (code_ == kSSLException) {
        return Error(kCheckYourFirewall).String();
    }
    if (code_ == kCannotWriteFile) {
        return "Check your user permissions";
    }
    if (code_ == kIsSuspended) {
        return "The workspace is suspended, please check your payments";
    }
    if (code_ == kRequestToServerFailedWithStatusCode403) {
        return "You do not have access to this workspace";
    }
    if (code_ == kMissingWorkspaceID) {
        return "Please select a project";
    }
    if (code_ == kDatabaseDiskMalformed) {
        return "Local database is corrupt. Please clear local data to recreate local database.";
    }
    if (code_ == kEndpointGoneError) {
        return Error(kOutOfDatePleaseUpgrade).String();
    }
    return String();
}

Error Error::fromHttpStatus(Poco::Net::HTTPResponse::HTTPStatus http_status) {
    return fromHttpStatus((int64_t) http_status);
}

Error Error::fromHttpStatus(int64_t http_status) {
    switch (http_status) {
    case 200:
    case 201:
    case 202:
        return kNoError;
    case 400:
        // data that you sending is not valid/acceptable
        return kBadRequestError;
    case 401:
        // ask user to enter login again, do not obtain new token automatically
        return kUnauthorizedError;
    case 402:
        // requested action allowed only for pro workspace show user upsell
        // page / ask for workspace pro upgrade. do not retry same request
        // unless known that client is pro
        return kPaymentRequiredError;
    case 403:
        // client has no right to perform given request. Server
        return kForbiddenError;
    case 404:
        // request is not possible
        // (or not allowed and server does not tell why)
        return kRequestIsNotPossible;
    case 410:
        return kEndpointGoneError;
    case 418:
        return kUnsupportedAppError;
    case 429:
        return kCannotConnectError;
    case 500:
    case 501:
    case 502:
    case 503:
    case 504:
    case 505:
        return kBackendIsDownError;
    default: {
        Logger("error").error("Unexpected HTTP status code: " + std::to_string(http_status));
        return kCannotConnectError;
    }
    }
}

Error Error::fromServerError(const std::string &message) {
    const static std::set<Code> codes {
        kTimeEntryNotFound, kTimeEntryCreatedWithInvalid, kCannotAccessProjectError, kCannotAccessTaskError,
        kOverMaxDurationError, kInvalidStartTimeError, kStartNotBeforeStopError, kBillableIsAPremiumFeature
    };
    for (auto i : codes) {
        if (message.find(Error(i).String()) != std::string::npos)
            return i;
    }
    return kNoError;
}

std::ostream &operator<<(std::ostream &out, const Error &t) {
    out << t.String();
    return out;
}

} // namespace toggl
