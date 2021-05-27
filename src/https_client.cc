// Copyright 2014 Toggl Desktop developers.

#include "https_client.h"

#include <json/json.h>

#include <string>
#include <sstream>
#include <memory>

#include "util/formatter.h"
#include "netconf.h"
#include "urls.h"
#include "toggl_api.h"

#include <Poco/DeflatingStream.h>
#include <Poco/Environment.h>
#include <Poco/Exception.h>
#include <Poco/FileStream.h>
#include <Poco/InflatingStream.h>
#include <Poco/Logger.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPBasicCredentials.h>
#include <Poco/Net/HTTPCredentials.h>
#include <Poco/Net/HTTPMessage.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/InvalidCertificateHandler.h>
#include <Poco/Net/NameValueCollection.h>
#include <Poco/Net/PrivateKeyPassphraseHandler.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/Session.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/NumberParser.h>
#include <Poco/StreamCopier.h>
#include <Poco/TextEncoding.h>
#include <Poco/UTF8Encoding.h>

namespace toggl {

void HTTPClient::SetCACertPath(const std::string& path) {
    if (path.compare(Config.CACertPath()) == 0) {
        return;
    }
    // Re-initialize the Poco Context
    Config.SetCACertPath(path);
    resetPocoContext();
}
void HTTPClient::SetIgnoreCert(bool ignore) {
    if (ignore == Config.IgnoreCert()) {
        return;
    }
    // Re-initialize the Poco Context
    Config.SetIgnoreCert(ignore);
    resetPocoContext();
}

void HTTPClient::resetPocoContext() {
    Poco::SharedPtr<Poco::Net::InvalidCertificateHandler>
    acceptCertHandler = new Poco::Net::AcceptCertificateHandler(true);

    Poco::Net::Context::VerificationMode verification_mode =
        Poco::Net::Context::VERIFY_RELAXED;
    if (HTTPClient::Config.IgnoreCert()) {
        verification_mode = Poco::Net::Context::VERIFY_NONE;
    }
    Poco::Net::Context::Ptr _context = new Poco::Net::Context(
        Poco::Net::Context::CLIENT_USE, "", "",
        HTTPClient::Config.CACertPath(),
        verification_mode, 9, true, "ALL");
    Poco::Net::SSLManager::instance().initializeClient(
        nullptr, acceptCertHandler, _context);
    context = std::move(_context);
}

void ServerStatus::startStatusCheck() {
    logger().debug("startStatusCheck fast_retry=", fast_retry_);

    if (checker_.isRunning()) {
        return;
    }
    checker_.start();
}

void ServerStatus::stopStatusCheck(const std::string &reason) {
    if (!checker_.isRunning() || checker_.isStopped()) {
        return;
    }

    logger().debug("stopStatusCheck, because ", reason);

    checker_.stop();
    checker_.wait();
}

Logger ServerStatus::logger() const {
    return { "ServerStatus" };
}

void ServerStatus::runActivity() {
    int delay_seconds = 60*3;
    if (!fast_retry_) {
        delay_seconds = 60*15;
    }

    logger().debug( "runActivity loop starting, delay_seconds=", delay_seconds);

    while (!checker_.isStopped()) {
        logger().debug("runActivity delay_seconds=", delay_seconds);

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
        HTTPRequest req;
        req.host = urls::API();
        req.relative_url = "/api/v9/status";

        HTTPResponse resp = TogglClient::GetInstance().silentGet(req);
        if (noError != resp.err) {
            logger().error(resp.err);

            srand(static_cast<unsigned>(time(nullptr)));
            float low(1.0), high(1.5);
            if (!fast_retry_) {
                low = 1.5;
                high = 2.0;
            }
            float r = low + static_cast<float>(rand()) /
                      (static_cast<float>(RAND_MAX) / (high - low));
            delay_seconds = static_cast<int>(delay_seconds * r);

            logger().debug("err=", resp.err, ", random=", r, ", delay_seconds=", delay_seconds);

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
    logger().debug("UpdateStatus status_code=", code);

    gone_ = 410 == code;

    if (code >= 500 && code < 600) {
        fast_retry_ = 500 != code;
        startStatusCheck();
        return;
    }

    stopStatusCheck("Status code " + std::to_string(code));
}

HTTPClientConfig HTTPClient::Config;
std::map<std::string, Poco::Timestamp> HTTPClient::banned_until_;

Logger HTTPClient::logger() const {
    return { "HTTPClient" };
}

bool HTTPClient::isRedirect(const Poco::Int64 status_code) const {
    return (status_code >= 300 && status_code < 400);
}

error HTTPClient::StatusCodeToError(const Poco::Int64 status_code) {
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

    Logger("HTTPClient").error("Unexpected HTTP status code: ", status_code);

    return kCannotConnectError;
}

error HTTPClient::accountLockingError(int remainingLogins) const {
    switch (remainingLogins) {
    case 1:
        return kOneLoginAttemptLeft;
    case 0:
        return kAccountIsLocked;
    default:
        return kIncorrectEmailOrPassword;
    }
}

HTTPResponse HTTPClient::Post(
    HTTPRequest req) const {
    req.method = Poco::Net::HTTPRequest::HTTP_POST;
    return request(req);
}

HTTPResponse HTTPClient::Get(
    HTTPRequest req) const {
    req.method = Poco::Net::HTTPRequest::HTTP_GET;
    return request(req);
}

HTTPResponse HTTPClient::Delete(
    HTTPRequest req) const {
    req.method = Poco::Net::HTTPRequest::HTTP_DELETE;
    return request(req);
}

HTTPResponse HTTPClient::Put(
    HTTPRequest req) const {
    req.method = Poco::Net::HTTPRequest::HTTP_PUT;
    return request(req);
}

HTTPResponse HTTPClient::request(
    HTTPRequest req, bool_t loggingOn) const {
    HTTPResponse resp = makeHttpRequest(req, loggingOn);

    if (kCannotConnectError == resp.err && isRedirect(resp.status_code)) {
        // Reattempt request to the given location.
        Poco::URI uri(resp.body);

        req.host = uri.getScheme() + "://" + uri.getHost();
        req.relative_url = uri.getPathEtc();

        if (loggingOn)
            logger().debug("Redirect to URL=", resp.body, " host=", req.host, " relative_url=", req.relative_url);
        resp = makeHttpRequest(req, loggingOn);
    }
    return resp;
}

HTTPResponse HTTPClient::makeHttpRequest(
    HTTPRequest req, bool_t loggingOn) const {

    HTTPResponse resp;

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
    if (HTTPClient::Config.CACertPath().empty()) {
        resp.err = error("Cannot make a HTTP request without certificates");
        return resp;
    }

    try {
        Poco::URI uri(req.host);

        std::shared_ptr<Poco::Net::HTTPClientSession> session;
        if (uri.getScheme() == "http") {
            session = std::make_shared<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());
        }
        else {
            session = std::make_shared<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort(), context);
        }

        session->setKeepAlive(true);
        session->setTimeout(
            Poco::Timespan(req.timeout_seconds * Poco::Timespan::SECONDS));

        if (loggingOn)
            logger().debug("Sending request to ", req.host, req.relative_url, " ..");

        std::string encoded_url("");
        Poco::URI::encode(req.relative_url, "", encoded_url);

        // Perform percent-encoded for request that has Query
        if (req.query) {
            Poco::URI url = Poco::URI(encoded_url);
            url.setQueryParameters(*req.query);
            encoded_url = url.getPathAndQuery();
        }

        error err = Netconf::ConfigureProxy(req.host + encoded_url, session.get());
        if (err != noError) {
            resp.err = error("Error while configuring proxy: " + err);
            logger().error(resp.err);
            return resp;
        }

        Poco::Net::HTTPRequest poco_req(req.method,
                                        encoded_url,
                                        Poco::Net::HTTPMessage::HTTP_1_1);
        poco_req.setKeepAlive(true);

        // Require new header for all Apple token
        std::string clientID = this->clientIDForRefererHeader();
        if (!clientID.empty()) {
            poco_req.set("Referer", clientID);
        }

        // FIXME: should get content type as parameter instead
        if (req.payload.size()) {
            poco_req.setContentType(kContentTypeApplicationJSON);
        }
        poco_req.set("User-Agent", HTTPClient::Config.UserAgent());

        if (!req.basic_auth_username.empty()
                && !req.basic_auth_password.empty()) {
            Poco::Net::HTTPBasicCredentials cred(
                req.basic_auth_username, req.basic_auth_password);
            cred.authenticate(poco_req);
        }

        // Request gzip unless downloading files
        poco_req.set("Accept-Encoding", "gzip");

        // Set the Switching board header for Sync server requests
        poco_req.set("X-Toggl-Client", "desktop");

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

            session->sendRequest(poco_req) << pBuff << std::flush;
        } else {
            req.form->prepareSubmit(poco_req);
            std::ostream& send = session->sendRequest(poco_req);
            req.form->write(send);
        }

        // Remove Auth info
        poco_req.erase("Authorization");

        // Log out request contents
        std::stringstream request_string;
        poco_req.write(request_string);
        if (loggingOn) {
            logger().debug(request_string.str());
            logger().debug("Request sent. Receiving response..");
        }

        // Receive response
        Poco::Net::HTTPResponse response;
        std::istream& is = session->receiveResponse(response);

        resp.status_code = response.getStatus();

        if (loggingOn)
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
        }

        // When we get redirect, set the Location as response body
        if (isRedirect(resp.status_code) && response.has("Location")) {
            std::string decoded_url("");
            Poco::URI::decode(response.get("Location"), decoded_url);
            resp.body = std::move(decoded_url);

            // Inflate, if gzip was sent
        } else if (response.has("Content-Encoding") &&
                   "gzip" == response.get("Content-Encoding")) {
            Poco::InflatingInputStream inflater(is, Poco::InflatingStreamBuf::STREAM_GZIP);
            {
                std::stringstream ss;
                ss << inflater.rdbuf();
                resp.body = ss.str();
            }

            // Write the response to string
        } else {
            std::streamsize n = Poco::StreamCopier::copyToString(is, resp.body);
            if (loggingOn)
                logger().debug(n, " characters transferred with download");
        }

        if (loggingOn)
            logger().trace(resp.body);

        if (429 == resp.status_code) {
            Poco::Timestamp ts = Poco::Timestamp() + (60 * kOneSecondInMicros);
            banned_until_[req.host] = ts;

            logger().debug("Server indicated we're making too many requests to host ", req.host,
                           ". So we cannot make new requests until ", Formatter::Format8601(ts));
        }

        resp.err = StatusCodeToError(resp.status_code);

        if (resp.status_code == 401 || resp.status_code == 403) {
            if (response.has("X-Remaining-Login-Attempts")) {
                int remainingLogins;
                if (Poco::NumberParser::tryParse(response.get("X-Remaining-Login-Attempts"), remainingLogins)) {
                    resp.err = accountLockingError(remainingLogins);
                }
            }
        }

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

std::string HTTPClient::clientIDForRefererHeader() const {
    if (POCO_OS_MAC_OS_X == POCO_OS) {
        return kTogglDesktopClientID_MacOS;
    }
    // Extend in the future
    return "";
}

ServerStatus TogglClient::TogglStatus;

Logger TogglClient::logger() const {
    return { "TogglClient" };
}

HTTPResponse TogglClient::request(
    HTTPRequest req, bool_t loggingOn) const {

    error err = TogglStatus.Status();
    if (err != noError) {
        logger().error("Will not connect, because of known bad Toggl status: ", err);
        HTTPResponse resp;
        resp.err = std::move(err);
        return resp;
    }

    if (monitor_) {
        monitor_->DisplaySyncState(kSyncStateWork);
    }

    HTTPResponse resp = HTTPClient::request(req);

    if (monitor_) {
        monitor_->DisplaySyncState(kSyncStateIdle);
    }

    // We only update Toggl status from this
    // client, not websocket or regular http client,
    // as they are not critical.
    TogglStatus.UpdateStatus(resp.status_code);

    return resp;
}

HTTPResponse TogglClient::silentPost(
    HTTPRequest req) const {
    req.method = Poco::Net::HTTPRequest::HTTP_POST;
    return HTTPClient::request(req);
}

HTTPResponse TogglClient::silentGet(
    HTTPRequest req) const {
    req.method = Poco::Net::HTTPRequest::HTTP_GET;
    return HTTPClient::request(req, false);
}

HTTPResponse TogglClient::silentDelete(
    HTTPRequest req) const {
    req.method = Poco::Net::HTTPRequest::HTTP_DELETE;
    return HTTPClient::request(req);
}

HTTPResponse TogglClient::silentPut(
    HTTPRequest req) const {
    req.method = Poco::Net::HTTPRequest::HTTP_PUT;
    return HTTPClient::request(req);
}
}   // namespace toggl
