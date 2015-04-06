// Copyright 2014 Toggl Desktop developers.

#include "../src/https_client.h"

#include <json/json.h>

#include <string>
#include <sstream>

#include "./const.h"
#include "./formatter.h"
#include "./netconf.h"

#include "Poco/DeflatingStream.h"
#include "Poco/Environment.h"
#include "Poco/Exception.h"
#include "Poco/InflatingStream.h"
#include "Poco/Logger.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/HTTPCredentials.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/InvalidCertificateHandler.h"
#include "Poco/Net/NameValueCollection.h"
#include "Poco/Net/PrivateKeyPassphraseHandler.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/Session.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/NumberParser.h"
#include "Poco/TextEncoding.h"
#include "Poco/URI.h"
#include "Poco/UTF8Encoding.h"

namespace toggl {

void ServerStatus::startStatusCheck() {
    std::stringstream ss;
    ss << "startStatusCheck fast_retry=" << fast_retry_;
    logger().debug(ss.str());

    if (checker_.isRunning()) {
        return;
    }
    checker_.start();
}

void ServerStatus::stopStatusCheck(const std::string reason) {
    if (!checker_.isRunning() || checker_.isStopped()) {
        return;
    }

    std::stringstream ss;
    ss << "stopStatusCheck, because " << reason;
    logger().debug(ss.str());

    checker_.stop();
    checker_.wait();
}

Poco::Logger &ServerStatus::logger() const {
    return Poco::Logger::get("ServerStatus");
}

void ServerStatus::runActivity() {
    int delay_seconds = 60*3;
    if (!fast_retry_) {
        delay_seconds = 60*15;
    }

    {
        std::stringstream ss;
        ss << "runActivity loop starting, delay_seconds=" << delay_seconds;
        logger().debug(ss.str());
    }

    while (!checker_.isStopped()) {
        {
            std::stringstream ss;
            ss << "runActivity delay_seconds=" << delay_seconds;
            logger().debug(ss.str());
        }

        // Sleep a bit
        for (int i = 0; i < delay_seconds; i++) {
            if (checker_.isStopped()) {
                return;
            }
            Poco::Thread::sleep(1000);
            if (checker_.isStopped()) {
                return;
            }
        }

        // Check server status
        HTTPSClient client;
        std::string response;
        error err = client.Get(
            kAPIURL, "/api/v8/status", "", "", &response);
        if (noError != err) {
            logger().error(err);

            srand(static_cast<unsigned>(time(0)));
            float low(1.0), high(1.5);
            if (!fast_retry_) {
                low = 1.5;
                high = 2.0;
            }
            float r = low + static_cast<float>(rand()) /  // NOLINT
                      (static_cast<float>(RAND_MAX / (high - low)));
            delay_seconds = delay_seconds * r;

            {
                std::stringstream ss;
                ss << "err=" << err
                   << ", random=" << r
                   << ", delay_seconds=" << delay_seconds;
                logger().debug(ss.str());
            }

            continue;
        }

        stopStatusCheck("No error from backend");
    }
}

error ServerStatus::Status() {
    if (gone_) {
        return kEndpointGoneError;
    }
    if (checker_.isRunning() && !checker_.isStopped()) {
        return kBackendIsDownError;
    }
    return noError;
}

void ServerStatus::UpdateStatus(const Poco::Int64 code) {
    {
        std::stringstream ss;
        ss << "UpdateStatus status_code=" << code;
        logger().debug(ss.str());
    }

    gone_ = 410 == code;

    if (code >= 500 && code < 600) {
        fast_retry_ = 500 != code;
        startStatusCheck();
        return;
    }

    std::stringstream ss;
    ss << "Status code " << code;
    stopStatusCheck(ss.str());
}

HTTPSClientConfig HTTPSClient::Config;
std::map<std::string, Poco::Timestamp> HTTPSClient::banned_until_;

Poco::Logger &HTTPSClient::logger() const {
    return Poco::Logger::get("HTTPSClient");
}

error HTTPSClient::statusCodeToError(const Poco::Int64 status_code) const {
    switch (status_code) {
    case 200:
    case 201:
    case 202:
        return noError;
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
        return kBackendIsDownError;
    case 501:
    case 502:
    case 503:
    case 504:
    case 505:
        return kBackendIsDownError;
    }

    {
        std::stringstream ss;
        ss << "Unexpected HTTP status code: " << status_code;
        logger().error(ss.str());
    }

    return kCannotConnectError;
}

error HTTPSClient::Post(
    const std::string host,
    const std::string relative_url,
    const std::string json,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body,
    Poco::Net::HTMLForm *form) {
    Poco::Int64 status_code(0);
    return request(Poco::Net::HTTPRequest::HTTP_POST,
                   host,
                   relative_url,
                   json,
                   basic_auth_username,
                   basic_auth_password,
                   response_body,
                   &status_code,
                   form);
}

error HTTPSClient::Get(
    const std::string host,
    const std::string relative_url,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body) {
    Poco::Int64 status_code(0);
    return request(Poco::Net::HTTPRequest::HTTP_GET,
                   host,
                   relative_url,
                   "",
                   basic_auth_username,
                   basic_auth_password,
                   response_body,
                   &status_code);
}

error HTTPSClient::request(
    const std::string method,
    const std::string host,
    const std::string relative_url,
    const std::string payload,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body,
    Poco::Int64 *status_code,
    Poco::Net::HTMLForm *form) {

    std::map<std::string, Poco::Timestamp>::const_iterator cit =
        banned_until_.find(host);
    if (cit != banned_until_.end()) {
        if (cit->second >= Poco::Timestamp()) {
            logger().warning(
                "Cannot connect, because we made too many requests");
            return kCannotConnectError;
        }
    }

    if (host.empty()) {
        return error("Cannot make a HTTP request without a host");
    }
    if (method.empty()) {
        return error("Cannot make a HTTP request without a method");
    }
    if (relative_url.empty()) {
        return error("Cannot make a HTTP request without a relative URL");
    }
    if (HTTPSClient::Config.CACertPath.empty()) {
        return error("Cannot make a HTTP request without certificates");
    }

    poco_check_ptr(response_body);
    poco_check_ptr(status_code);

    *response_body = "";
    *status_code = 0;

    try {
        Poco::URI uri(host);

        Poco::SharedPtr<Poco::Net::InvalidCertificateHandler>
        acceptCertHandler =
            new Poco::Net::AcceptCertificateHandler(true);

        Poco::Net::Context::VerificationMode verification_mode =
            Poco::Net::Context::VERIFY_RELAXED;
        if (HTTPSClient::Config.IgnoreCert) {
            verification_mode = Poco::Net::Context::VERIFY_NONE;
        }
        Poco::Net::Context::Ptr context = new Poco::Net::Context(
            Poco::Net::Context::CLIENT_USE, "", "",
            HTTPSClient::Config.CACertPath,
            verification_mode, 9, true, "ALL");

        Poco::Net::SSLManager::instance().initializeClient(
            0, acceptCertHandler, context);

        Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(),
                                              context);

        session.setKeepAlive(false);
        session.setTimeout(Poco::Timespan(kHTTPClientTimeoutSeconds
                                          * Poco::Timespan::SECONDS));

        {
            std::stringstream ss;
            ss << "Sending request to " << host << relative_url << " ..";
            logger().debug(ss.str());
        }

        std::string encoded_url("");
        Poco::URI::encode(relative_url, "", encoded_url);

        error err = Netconf::ConfigureProxy(host + encoded_url, &session);
        if (err != noError) {
            logger().error("Error while configuring proxy: " + err);
            return err;
        }

        Poco::Net::HTTPRequest req(method,
                                   encoded_url,
                                   Poco::Net::HTTPMessage::HTTP_1_1);
        req.setKeepAlive(false);

        // FIXME: should get content type as parameter instead
        if (payload.size()) {
            req.setContentType(kContentTypeApplicationJSON);
        }
        req.set("User-Agent", HTTPSClient::Config.UserAgent());
        req.setChunkedTransferEncoding(true);

        Poco::Net::HTTPBasicCredentials cred(
            basic_auth_username, basic_auth_password);
        if (!basic_auth_username.empty() && !basic_auth_password.empty()) {
            cred.authenticate(req);
        }


        if (!form) {
            std::istringstream requestStream(payload);

            Poco::DeflatingInputStream gzipRequest(
                requestStream,
                Poco::DeflatingStreamBuf::STREAM_GZIP);
            Poco::DeflatingStreamBuf *pBuff = gzipRequest.rdbuf();

            Poco::Int64 size =
                pBuff->pubseekoff(0, std::ios::end, std::ios::in);
            pBuff->pubseekpos(0, std::ios::in);

            req.setContentLength(size);
            req.set("Content-Encoding", "gzip");

            session.sendRequest(req) << pBuff << std::flush;
        } else {
            form->prepareSubmit(req);
            std::ostream& send = session.sendRequest(req);
            form->write(send);
        }

        req.set("Accept-Encoding", "gzip");

        // Log out request contents
        std::stringstream request_string;
        req.write(request_string);
        logger().debug(request_string.str());

        logger().debug("Request sent. Receiving response..");

        // Receive response
        Poco::Net::HTTPResponse response;
        std::istream& is = session.receiveResponse(response);

        *status_code = response.getStatus();

        {
            std::stringstream ss;
            ss << "Response status code " << response.getStatus()
               << ", content length " << response.getContentLength()
               << ", content type " << response.getContentType();
            if (response.has("Content-Encoding")) {
                ss << ", content encoding " << response.get("Content-Encoding");
            } else {
                ss << ", unknown content encoding";
            }
            logger().debug(ss.str());
        }

        // Inflate, if gzip was sent
        if (response.has("Content-Encoding") &&
                "gzip" == response.get("Content-Encoding")) {
            Poco::InflatingInputStream inflater(
                is,
                Poco::InflatingStreamBuf::STREAM_GZIP);
            {
                std::stringstream ss;
                ss << inflater.rdbuf();
                *response_body = ss.str();
            }
        } else {
            std::istreambuf_iterator<char> eos;
            *response_body =
                std::string(std::istreambuf_iterator<char>(is), eos);
        }

        if (response_body->size() < 1204 * 10) {
            logger().trace(*response_body);
        }

        if (429 == *status_code) {
            Poco::Timestamp ts = Poco::Timestamp() + (60 * kOneSecondInMicros);
            banned_until_[host] = ts;

            std::stringstream ss;
            ss << "Server indicated we're making too many requests to host "
               << host << ". So we cannot make new requests until "
               << Formatter::Format8601(ts);
            logger().debug(ss.str());
        }

        return statusCodeToError(*status_code);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
}

ServerStatus TogglClient::TogglStatus;

Poco::Logger &TogglClient::logger() const {
    return Poco::Logger::get("TogglClient");
}

error TogglClient::request(
    const std::string method,
    const std::string host,
    const std::string relative_url,
    const std::string json,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body,
    Poco::Int64 *status_code,
    Poco::Net::HTMLForm *form) {

    error err = TogglStatus.Status();
    if (err != noError) {
        std::stringstream ss;
        ss << "Will not connect, because of known bad Toggl status: " << err;
        logger().error(ss.str());
        return err;
    }

    if (monitor_) {
        monitor_->DisplaySyncState(kSyncStateWork);
    }

    err = HTTPSClient::request(
        method,
        host,
        relative_url,
        json,
        basic_auth_username,
        basic_auth_password,
        response_body,
        status_code,
        form);

    if (monitor_) {
        monitor_->DisplaySyncState(kSyncStateIdle);
    }

    // We only update Toggl status from this
    // client, not websocket or regular http client,
    // as they are not critical.
    TogglStatus.UpdateStatus(*status_code);

    return err;
}

}   // namespace toggl
