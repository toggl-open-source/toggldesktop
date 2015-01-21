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

ServerStatus::ServerStatus() {
    m_ = new Poco::Mutex();
}

ServerStatus::~ServerStatus() {
    delete m_;
    m_ = 0;
}

void ServerStatus::SetGone(const std::string host, const bool value) {
    Poco::Mutex::ScopedLock lock(*m_);
    if (value) {
        gone_.insert(host);
        return;
    }
    std::set<std::string>::iterator it = gone_.find(host);
    if (it != gone_.end()) {
        gone_.erase(it);
    }
}

bool ServerStatus::Gone(const std::string host) {
    Poco::Mutex::ScopedLock lock(*m_);
    return gone_.find(host) != gone_.end();
}

HTTPSClientConfig HTTPSClient::Config = HTTPSClientConfig();
ServerStatus HTTPSClient::BackendStatus = ServerStatus();

error HTTPSClient::PostJSON(
    const std::string host,
    const std::string relative_url,
    const std::string json,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body) {
    return requestJSON(Poco::Net::HTTPRequest::HTTP_POST,
                       host,
                       relative_url,
                       json,
                       basic_auth_username,
                       basic_auth_password,
                       response_body);
}

error HTTPSClient::GetJSON(
    const std::string host,
    const std::string relative_url,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body) {
    return requestJSON(Poco::Net::HTTPRequest::HTTP_GET,
                       host,
                       relative_url,
                       "",
                       basic_auth_username,
                       basic_auth_password,
                       response_body);
}

error HTTPSClient::requestJSON(
    const std::string method,
    const std::string host,
    const std::string relative_url,
    const std::string json,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body) {

    if (BackendStatus.Gone(host)) {
        return kEndpointGoneError;
    }

    int response_status(0);
    error err = request(
        method,
        host,
        relative_url,
        json,
        basic_auth_username,
        basic_auth_password,
        response_body,
        &response_status);

    // Some exception occurred
    if (err != noError) {
        return err;
    }

    // HTTP status gone away
    if (410 == response_status) {
        BackendStatus.SetGone(host, true);
        return kEndpointGoneError;
    }

    // Other HTTP status
    if (response_status < 200 || response_status >= 300) {
        if (response_body->empty()) {
            std::stringstream description;
            description << "Request to server failed with status code: "
                        << response_status;
            return description.str();
        }
        return *response_body;
    }

    return noError;
}

error HTTPSClient::request(
    const std::string method,
    const std::string host,
    const std::string relative_url,
    const std::string payload,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body,
    int *response_status) {

    poco_assert(!host.empty());
    poco_assert(!method.empty());
    poco_assert(!relative_url.empty());
    poco_assert(!HTTPSClient::Config.CACertPath.empty());

    poco_check_ptr(response_body);

    *response_body = "";

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

        *response_status = response.getStatus();

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
