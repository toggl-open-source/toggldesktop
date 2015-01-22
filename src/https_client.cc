// Copyright 2014 Toggl Desktop developers.

#include "../src/https_client.h"

#include <string>
#include <sstream>

#include <json/json.h>  // NOLINT

#include "Poco/DeflatingStream.h"
#include "Poco/Exception.h"
#include "Poco/InflatingStream.h"
#include "Poco/Logger.h"
#include "Poco/NumberParser.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPBasicCredentials.h"
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
#include "Poco/TextEncoding.h"
#include "Poco/URI.h"
#include "Poco/UTF8Encoding.h"

#include "./const.h"

namespace toggl {

error ServerStatus::Status() {
    if (gone()) {
        return kEndpointGoneError;
    }
    if (checkingStatus()) {
        return kCannotConnectError;
    }
    return noError;
}

error ServerStatus::UpdateStatus(const Poco::Int64 code) {
    {
        std::stringstream ss;
        ss << "UpdateStatus status_code=" << code;
        logger().debug(ss.str());
    }

    if (code >= 500 && code < 600) {
        fast_retry_ = 500 != code;
        startStatusCheck();
    } else {
        stopStatusCheck();
    }

    switch (code) {
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
        setGone(true);
        return kEndpointGoneError;
    case 418:
        return kUnsupportedAppError;
    case 500:
        return kCannotConnectError;
    case 501:
    case 502:
    case 503:
    case 504:
    case 505:
        return kCannotConnectError;
    }

    {
        std::stringstream ss;
        ss << "Unexpected HTTP status code from backend: " << code;
        logger().error(ss.str());
    }

    return kCannotConnectError;
}

void ServerStatus::setGone(const bool value) {
    std::stringstream ss;
    ss << "setGone value=" << value;
    logger().debug(ss.str());
    gone_ = value;
}

bool ServerStatus::gone() {
    return gone_;
}

void ServerStatus::startStatusCheck() {
    std::stringstream ss;
    ss << "startStatusCheck fast_retry=" << fast_retry_;
    logger().debug(ss.str());

    if (checker_.isRunning()) {
        return;
    }
    checker_.start();
}

void ServerStatus::stopStatusCheck() {
    if (!checker_.isRunning() || checker_.isStopped()) {
        return;
    }
    logger().debug("stopStatusCheck");
    checker_.stop();
    checker_.wait();
}

bool ServerStatus::checkingStatus() {
    return checker_.isRunning() && !checker_.isStopped();
}

Poco::Logger &ServerStatus::logger() const {
    return Poco::Logger::get("ServerStatus");
}

void ServerStatus::runActivity() {
    int delay_seconds = 10;
    if (!fast_retry_) {
        delay_seconds = 60;
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
            Poco::Thread::sleep(1000);
            if (checker_.isStopped()) {
                return;
            }
        }

        // Check server status
        HTTPSClient client;
        std::string response;
        error err = client.GetJSON(
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

        stopStatusCheck();
    }
}

HTTPSClientConfig HTTPSClient::Config;

ServerStatus TogglClient::TogglStatus;

error HTTPSClient::PostJSON(
    const std::string host,
    const std::string relative_url,
    const std::string json,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body) {
    int status_code(0);
    return request(Poco::Net::HTTPRequest::HTTP_POST,
                   host,
                   relative_url,
                   json,
                   basic_auth_username,
                   basic_auth_password,
                   response_body,
                   &status_code);
}

error HTTPSClient::GetJSON(
    const std::string host,
    const std::string relative_url,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body) {
    int status_code(0);
    return request(Poco::Net::HTTPRequest::HTTP_GET,
                   host,
                   relative_url,
                   "",
                   basic_auth_username,
                   basic_auth_password,
                   response_body,
                   &status_code);
}

error TogglClient::request(
    const std::string method,
    const std::string host,
    const std::string relative_url,
    const std::string json,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body,
    int *status_code) {

    error err = TogglStatus.Status();
    if (err != noError) {
        std::stringstream ss;
        ss << "Will not connect, because of known bad Toggl status: " << err;
        Poco::Logger::get("TogglClient").error(ss.str());
        return err;
    }

    err = HTTPSClient::request(
        method,
        host,
        relative_url,
        json,
        basic_auth_username,
        basic_auth_password,
        response_body,
        status_code);
    if (err != noError) {
        return err;
    }

    // We only update Toggl status from this
    // client, not websocket or regular http client,
    // as they are not critical.
    return TogglStatus.UpdateStatus(*status_code);
}

error HTTPSClient::request(
    const std::string method,
    const std::string host,
    const std::string relative_url,
    const std::string payload,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body,
    int *status_code) {

    poco_assert(!host.empty());
    poco_assert(!method.empty());
    poco_assert(!relative_url.empty());
    poco_assert(!HTTPSClient::Config.CACertPath.empty());

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
        if (HTTPSClient::Config.UseProxy &&
                HTTPSClient::Config.ProxySettings.IsConfigured()) {
            session.setProxy(
                HTTPSClient::Config.ProxySettings.Host(),
                static_cast<Poco::UInt16>(
                    HTTPSClient::Config.ProxySettings.Port()));
            if (HTTPSClient::Config.ProxySettings.HasCredentials()) {
                session.setProxyCredentials(
                    HTTPSClient::Config.ProxySettings.Username(),
                    HTTPSClient::Config.ProxySettings.Password());
            }
        }
        session.setKeepAlive(false);
        session.setTimeout(Poco::Timespan(kHTTPClientTimeoutSeconds
                                          * Poco::Timespan::SECONDS));

        Poco::Logger &logger = Poco::Logger::get("https_client");
        {
            std::stringstream ss;
            ss << "Sending request to " << relative_url << " ..";
            logger.debug(ss.str());
        }

        std::string encoded_url("");
        Poco::URI::encode(relative_url, "", encoded_url);
        Poco::Net::HTTPRequest req(method,
                                   encoded_url,
                                   Poco::Net::HTTPMessage::HTTP_1_1);
        req.setKeepAlive(false);
        req.setContentType("application/json");
        req.set("User-Agent", HTTPSClient::Config.UserAgent());
        req.setChunkedTransferEncoding(true);

        Poco::Net::HTTPBasicCredentials cred(
            basic_auth_username, basic_auth_password);
        if (!basic_auth_username.empty() && !basic_auth_password.empty()) {
            cred.authenticate(req);
        }

        std::istringstream requestStream(payload);
        Poco::DeflatingInputStream gzipRequest(
            requestStream,
            Poco::DeflatingStreamBuf::STREAM_GZIP);
        Poco::DeflatingStreamBuf *pBuff = gzipRequest.rdbuf();

        Poco::Int64 size = pBuff->pubseekoff(0, std::ios::end, std::ios::in);
        pBuff->pubseekpos(0, std::ios::in);

        req.setContentLength(size);
        req.set("Content-Encoding", "gzip");
        req.set("Accept-Encoding", "gzip");

        session.sendRequest(req) << pBuff << std::flush;

        // Log out request contents
        std::stringstream request_string;
        req.write(request_string);
        logger.debug(request_string.str());

        logger.debug("Request sent. Receiving response..");

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
            logger.debug(ss.str());
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

        logger.trace(*response_body);
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

}   // namespace toggl
