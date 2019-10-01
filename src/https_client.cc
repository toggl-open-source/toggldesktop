// Copyright 2014 Toggl Desktop developers.

#include "../src/https_client.h"

#include <json/json.h>

#include <string>
#include <sstream>

#include "./formatter.h"
#include "./netconf.h"
#include "./urls.h"
#include "./toggl_api.h"

#include "Poco/DeflatingStream.h"
#include "Poco/Environment.h"
#include "Poco/Exception.h"
#include "Poco/FileStream.h"
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
#include "Poco/StreamCopier.h"
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

void ServerStatus::stopStatusCheck(const std::string &reason) {
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
        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/status";

        HTTPSResponse resp = client.Get(req);
        if (noError != resp.err) {
            logger().error(resp.err);

            srand(static_cast<unsigned>(time(nullptr)));
            float low(1.0), high(1.5);
            if (!fast_retry_) {
                low = 1.5;
                high = 2.0;
            }
            float r = low + static_cast<float>(rand()) /
                      (static_cast<float>(RAND_MAX / (high - low)));
            delay_seconds = static_cast<int>(delay_seconds * r);

            {
                std::stringstream ss;
                ss << "err=" << resp.err
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

bool HTTPSClient::isRedirect(const Poco::Int64 status_code) const {
    return (status_code >= 300 && status_code < 400);
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

HTTPSResponse HTTPSClient::Post(
    HTTPSRequest req) {
    req.method = Poco::Net::HTTPRequest::HTTP_POST;
    return request(req);
}

HTTPSResponse HTTPSClient::Get(
    HTTPSRequest req) {
    req.method = Poco::Net::HTTPRequest::HTTP_GET;
    return request(req);
}

HTTPSResponse HTTPSClient::GetFile(
    HTTPSRequest req) {
    req.method = Poco::Net::HTTPRequest::HTTP_GET;
    req.timeout_seconds = kHTTPClientTimeoutSeconds * 10;
    return request(req);
}

HTTPSResponse HTTPSClient::Delete(
    HTTPSRequest req) {
    req.method = Poco::Net::HTTPRequest::HTTP_DELETE;
    return request(req);
}

HTTPSResponse HTTPSClient::Put(
    HTTPSRequest req) {
    req.method = Poco::Net::HTTPRequest::HTTP_PUT;
    return request(req);
}

HTTPSResponse HTTPSClient::request(
    HTTPSRequest req) {
    HTTPSResponse resp = makeHttpRequest(req);

    if (kCannotConnectError == resp.err && isRedirect(resp.status_code)) {
        // Reattempt request to the given location.
        Poco::URI uri(resp.body);

        req.host = uri.getScheme() + "://" + uri.getHost();
        req.relative_url = uri.getPathEtc();
        {
            std::stringstream ss;
            ss << "Redirect to URL=" << resp.body
               << " host=" << req.host
               << " relative_url=" << req.relative_url;
            logger().debug(ss.str());
        }
        resp = makeHttpRequest(req);
    }
    return resp;
}

HTTPSResponse HTTPSClient::makeHttpRequest(
    HTTPSRequest req) {

    HTTPSResponse resp;

    if (!urls::RequestsAllowed()) {
        resp.err = error(kCannotSyncInTestEnv);
        return resp;
    }

    if (urls::ImATeapot()) {
        resp.err = error(kUnsupportedAppError);
        return resp;
    }

    std::map<std::string, Poco::Timestamp>::const_iterator cit =
        banned_until_.find(req.host);
    if (cit != banned_until_.end()) {
        if (cit->second >= Poco::Timestamp()) {
            logger().warning(
                "Cannot connect, because we made too many requests");
            resp.err = kCannotConnectError;
            return resp;
        }
    }

    if (req.host.empty()) {
        resp.err = error("Cannot make a HTTP request without a host");
        return resp;
    }
    if (req.method.empty()) {
        resp.err = error("Cannot make a HTTP request without a method");
        return resp;
    }
    if (req.relative_url.empty()) {
        resp.err = error("Cannot make a HTTP request without a relative URL");
        return resp;
    }
    if (HTTPSClient::Config.CACertPath.empty()) {
        resp.err = error("Cannot make a HTTP request without certificates");
        return resp;
    }

    try {
        Poco::URI uri(req.host);

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
            nullptr, acceptCertHandler, context);

        Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(),
                                              context);

        session.setKeepAlive(false);
        session.setTimeout(
            Poco::Timespan(req.timeout_seconds * Poco::Timespan::SECONDS));

        {
            std::stringstream ss;
            ss << "Sending request to "
               << req.host << req.relative_url << " ..";
            logger().debug(ss.str());
        }

        std::string encoded_url("");
        Poco::URI::encode(req.relative_url, "", encoded_url);

        error err = Netconf::ConfigureProxy(req.host + encoded_url, &session);
        if (err != noError) {
            resp.err = error("Error while configuring proxy: " + err);
            logger().error(resp.err);
            return resp;
        }

        Poco::Net::HTTPRequest poco_req(req.method,
                                        encoded_url,
                                        Poco::Net::HTTPMessage::HTTP_1_1);
        poco_req.setKeepAlive(false);

        // FIXME: should get content type as parameter instead
        if (req.payload.size()) {
            poco_req.setContentType(kContentTypeApplicationJSON);
        }
        poco_req.set("User-Agent", HTTPSClient::Config.UserAgent());

        Poco::Net::HTTPBasicCredentials cred(
            req.basic_auth_username, req.basic_auth_password);
        if (!req.basic_auth_username.empty()
                && !req.basic_auth_password.empty()) {
            cred.authenticate(poco_req);
        }

        if (!req.form) {
            std::istringstream requestStream(req.payload);

            Poco::DeflatingInputStream gzipRequest(
                requestStream,
                Poco::DeflatingStreamBuf::STREAM_GZIP);
            Poco::DeflatingStreamBuf *pBuff = gzipRequest.rdbuf();

            Poco::Int64 size =
                pBuff->pubseekoff(0, std::ios::end, std::ios::in);
            pBuff->pubseekpos(0, std::ios::in);

            if (req.method != Poco::Net::HTTPRequest::HTTP_GET) {
                poco_req.setContentLength(size);
                poco_req.set("Content-Encoding", "gzip");
                poco_req.setChunkedTransferEncoding(true);
            }

            session.sendRequest(poco_req) << pBuff << std::flush;
        } else {
            req.form->prepareSubmit(poco_req);
            std::ostream& send = session.sendRequest(poco_req);
            req.form->write(send);
        }

        // Request gzip unless downloading files
        poco_req.set("Accept-Encoding", "gzip");

        // Log out request contents
        std::stringstream request_string;
        poco_req.write(request_string);
        logger().debug(request_string.str());

        logger().debug("Request sent. Receiving response..");

        // Receive response
        Poco::Net::HTTPResponse response;
        std::istream& is = session.receiveResponse(response);

        resp.status_code = response.getStatus();

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

        // Log out X-Toggl-Request-Id, so failed requests can be traced
        if (response.has("X-Toggl-Request-Id")) {
            logger().debug("X-Toggl-Request-Id "
                           + response.get("X-Toggl-Request-Id"));
        }

        // Print out response headers
        Poco::Net::NameValueCollection::ConstIterator it = response.begin();
        while (it != response.end()) {
            logger().debug(it->first + ": " + it->second);
            ++it;
        }

        // When we get redirect, set the Location as response body
        if (isRedirect(resp.status_code) && response.has("Location")) {
            std::string decoded_url("");
            Poco::URI::decode(response.get("Location"), decoded_url);
            resp.body = decoded_url;

            // Inflate, if gzip was sent
        } else if (response.has("Content-Encoding") &&
                   "gzip" == response.get("Content-Encoding")) {
            Poco::InflatingInputStream inflater(
                is,
                Poco::InflatingStreamBuf::STREAM_GZIP);
            {
                std::stringstream ss;
                ss << inflater.rdbuf();
                resp.body = ss.str();
            }

            // Write the response to string
        } else {
            std::streamsize n =
                Poco::StreamCopier::copyToString(is, resp.body);
            std::stringstream ss;
            ss << n << " characters transferred with download";
            logger().debug(ss.str());
        }

        logger().trace(resp.body);

        if (429 == resp.status_code) {
            Poco::Timestamp ts = Poco::Timestamp() + (60 * kOneSecondInMicros);
            banned_until_[req.host] = ts;

            std::stringstream ss;
            ss << "Server indicated we're making too many requests to host "
               << req.host << ". So we cannot make new requests until "
               << Formatter::Format8601(ts);
            logger().debug(ss.str());
        }

        resp.err = statusCodeToError(resp.status_code);

        // Parse human-readable error message from response if Content Type JSON
        if (resp.err != noError &&
                response.getContentType().find(kContentTypeApplicationJSON) != std::string::npos) {
            Json::Value root;
            Json::Reader reader;
            if (reader.parse(resp.body, root)) {
                resp.body = root["error_message"].asString();
            }
        }
    } catch(const Poco::Exception& exc) {
        resp.err = exc.displayText();
        return resp;
    } catch(const std::exception& ex) {
        resp.err = ex.what();
        return resp;
    } catch(const std::string & ex) {
        resp.err = ex;
        return resp;
    }
    return resp;
}

ServerStatus TogglClient::TogglStatus;

Poco::Logger &TogglClient::logger() const {
    return Poco::Logger::get("TogglClient");
}

HTTPSResponse TogglClient::request(
    HTTPSRequest req) {

    error err = TogglStatus.Status();
    if (err != noError) {
        std::stringstream ss;
        ss << "Will not connect, because of known bad Toggl status: " << err;
        logger().error(ss.str());
        HTTPSResponse resp;
        resp.err = err;
        return resp;
    }

    if (monitor_) {
        monitor_->DisplaySyncState(kSyncStateWork);
    }

    HTTPSResponse resp = HTTPSClient::request(req);

    if (monitor_) {
        monitor_->DisplaySyncState(kSyncStateIdle);
    }

    // We only update Toggl status from this
    // client, not websocket or regular http client,
    // as they are not critical.
    TogglStatus.UpdateStatus(resp.status_code);

    return resp;
}

}   // namespace toggl
