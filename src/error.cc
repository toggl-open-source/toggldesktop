
// Copyright 2014 Toggl Desktop developers.

#include "./error.h"

#include <string>

#include "./const.h"

namespace toggl {

bool IsNetworkingError(const error err) {
    std::string value(err);
    if (value.find("Cannot establish proxy connection") != std::string::npos) {
        return true;
    }
    if (value.find("certificate verify failed") != std::string::npos) {
        return true;
    }
    if (value.find("Proxy Authentication Required") != std::string::npos) {
        return true;
    }
    if (value.find("Cannot assign requested address") != std::string::npos) {
        return true;
    }
    if (value.find("Certificate validation error") != std::string::npos) {
        return true;
    }
    if (value.find("Unacceptable certificate from www.toggl.com")
            != std::string::npos) {
        return true;
    }
    if (value.find("Host not found") != std::string::npos) {
        return true;
    }
    if (value.find("Cannot upgrade to WebSocket connection")
            != std::string::npos) { // NOLINT
        return true;
    }
    if (value.find("No message received") != std::string::npos) {
        return true;
    }
    if (value.find("Connection refused") != std::string::npos) {
        return true;
    }
    if (value.find("Connection timed out") != std::string::npos) {
        return true;
    }
    if (value.find("connect timed out") != std::string::npos) {
        return true;
    }
    if (value.find("SSL connection unexpectedly closed") != std::string::npos) {
        return true;
    }
    if (value.find("Network is down") != std::string::npos) {
        return true;
    }
    if (value.find("Network is unreachable") != std::string::npos) {
        return true;
    }
    if (value.find("Host is down") != std::string::npos) {
        return true;
    }
    if (value.find("No route to host") != std::string::npos) {
        return true;
    }
    if ((value.find("I/O error: 1") != std::string::npos)
            && (value.find(":443") != std::string::npos)) {
        return true;
    }
    if (value.find("The request timed out") != std::string::npos) {
        return true;
    }
    if (value.find("Could not connect to the server") != std::string::npos) {
        return true;
    }
    if (value.find("Connection reset by peer") != std::string::npos) {
        return true;
    }
    if (value.find("The Internet connection appears to be offline")
            != std::string::npos) {
        return true;
    }
    if (value.find("Timeout") != std::string::npos) {
        return true;
    }
    if (value.find("SSL Exception") != std::string::npos) {
        return true;
    }
    if (value.find("An internal server error occurred.") != std::string::npos) {
        return true;
    }
    return false;
}

bool IsUserError(const error err) {
    if (noError == err) {
        return false;
    }
    std::string value(err);
    if (value.find("is suspended") != std::string::npos) {
        return true;
    }
    if (value.find("Request to server failed with status code: 403")
            != std::string::npos) {
        return true;
    }
    if (value.find("This version of the app is not supported")
            != std::string::npos) {
        return true;
    }
    if (value.find("Stop time must be after start time")
            != std::string::npos) {
        return true;
    }
    if (value.find("Invalid e-mail or password") != std::string::npos) {
        return true;
    }
    if (value.find("Maximum length for description") != std::string::npos) {
        return true;
    }
    if (value.find("Start time year must be between 2010 and 2100")
            != std::string::npos) {
        return true;
    }
    if (value.find("Missing workspace ID") != std::string::npos) {
        return true;
    }
    if (value.find(kEndpointGoneError) != std::string::npos) {
        return true;
    }
    if (value.find("Password should be at least") != std::string::npos) {
        return true;
    }
    if (value.find("User with this email already exists") !=
            std::string::npos) {
        return true;
    }
    if (value.find("Invalid e-mail") != std::string::npos) {
        return true;
    }
    return false;
}

}  // namespace toggl
