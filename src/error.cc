
// Copyright 2014 Toggl Desktop developers.

#include "../src/error.h"

#include <string>

#include "./const.h"

namespace toggl {

bool IsNetworkingError(const error &err) {
    if (noError == err) {
        return false;
    }
    if (err.find(kCannotConnectError) != std::string::npos) {
        return true;
    }
    if (err.find(kBackendIsDownError) != std::string::npos) {
        return true;
    }
    if (err.find(kCannotEstablishProxyConnection) != std::string::npos) {
        return true;
    }
    if (err.find(kCertificateVerifyFailed) != std::string::npos) {
        return true;
    }
    if (err.find(kProxyAuthenticationRequired) != std::string::npos) {
        return true;
    }
    if (err.find("Cannot assign requested address") != std::string::npos) {
        return true;
    }
    if (err.find(kCertificateValidationError) != std::string::npos) {
        return true;
    }
    if (err.find(kUnacceptableCertificate) != std::string::npos) {
        return true;
    }
    if (err.find("Host not found") != std::string::npos) {
        return true;
    }
    if (err.find(kCannotUpgradeToWebSocketConnection) != std::string::npos) {
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
    if (err.find(kSSLException) != std::string::npos) {
        return true;
    }
    if (err.find("An internal server error occurred.") != std::string::npos) {
        return true;
    }
    return false;
}

bool IsUserError(const error &err) {
    if (noError == err) {
        return false;
    }
    if (err.find(kErrorRuleAlreadyExists) != std::string::npos) {
        return true;
    }
    if (err.find(kCheckYourSignupError) != std::string::npos) {
        return true;
    }
    if (err.find(kEmailNotFoundCannotLogInOffline) != std::string::npos) {
        return true;
    }
    if (err.find(kInvalidPassword) != std::string::npos) {
        return true;
    }
    if (err.find(kPaymentRequiredError) != std::string::npos) {
        return true;
    }
    if (err.find("File not found") != std::string::npos) {
        return true;
    }
    if (err.find("SSL context exception") != std::string::npos) {
        return true;
    }
    if (err.find("Access to file denied") != std::string::npos) {
        return true;
    }
    if (err.find(kBadRequestError) != std::string::npos) {
        return true;
    }
    if (err.find(kUnauthorizedError) != std::string::npos) {
        return true;
    }
    if (err.find(kCannotWriteFile) != std::string::npos) {
        return true;
    }
    if (err.find(kIsSuspended) != std::string::npos) {
        return true;
    }
    if (err.find(kRequestToServerFailedWithStatusCode403)
            != std::string::npos) {
        return true;
    }
    if (err.find(kUnsupportedAppError) != std::string::npos) {
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
    if (err.find(kMissingWorkspaceID) != std::string::npos) {
        return true;
    }
    if (err.find(kEndpointGoneError) != std::string::npos) {
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
    if (err.find(kCannotAccessWorkspaceError) != std::string::npos) {
        return true;
    }
    if (err.find(kCannotSyncInTestEnv) != std::string::npos) {
        return true;
    }
    if (err.find(kCannotContinueDeletedTimeEntry) != std::string::npos) {
        return true;
    }
    if (err.find(kCannotDeleteDeletedTimeEntry) != std::string::npos) {
        return true;
    }
    if (err.find(kPleaseSelectAWorkspace) != std::string::npos) {
        return true;
    }
    if (err.find(kClientNameMustNotBeEmpty) != std::string::npos) {
        return true;
    }
    if (err.find(kProjectNameMustNotBeEmpty) != std::string::npos) {
        return true;
    }
    if (err.find(kClientNameAlreadyExists) != std::string::npos) {
        return true;
    }
    if (err.find(kProjectNameAlreadyExists) != std::string::npos) {
        return true;
    }
    return false;
}

std::string MakeErrorActionable(const error &err) {
    if (noError == err) {
        return err;
    }
    if (err.find(kCannotEstablishProxyConnection) != std::string::npos) {
        return kCheckYourProxySetup;
    }
    if (err.find(kCertificateVerifyFailed) != std::string::npos) {
        return kCheckYourFirewall;
    }
    if (err.find(kProxyAuthenticationRequired) != std::string::npos) {
        return kCheckYourProxySetup;
    }
    if (err.find(kCertificateValidationError) != std::string::npos) {
        return kCheckYourFirewall;
    }
    if (err.find(kUnacceptableCertificate) != std::string::npos) {
        return kCheckYourFirewall;
    }
    if (err.find(kCannotUpgradeToWebSocketConnection) != std::string::npos) {
        return kCheckYourFirewall;
    }
    if (err.find(kSSLException) != std::string::npos) {
        return kCheckYourFirewall;
    }
    if (err.find(kCannotWriteFile) != std::string::npos) {
        return "Check your user permissions";
    }
    if (err.find(kIsSuspended) != std::string::npos) {
        return "The workspace is suspended, please check your payments";
    }
    if (err.find(kRequestToServerFailedWithStatusCode403)
            != std::string::npos) {
        return "You do not have access to this workspace";
    }
    if (err.find(kMissingWorkspaceID) != std::string::npos) {
        return "Please select a project";
    }
    if (err.find(kDatabaseDiskMalformed) != std::string::npos) {
        return "Local database is corrupt. Please clear local data to recreate local database.";
    }
    if (err.find(kEndpointGoneError) != std::string::npos) {
        return kOutOfDatePleaseUpgrade;
    }

    return err;
}

}  // namespace toggl
