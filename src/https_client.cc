// Copyright 2014 Toggl Desktop developers.

#include "./https_client.h"

#include <string>
#include <sstream>

#include "Poco/Exception.h"
#include "Poco/InflatingStream.h"
#include "Poco/DeflatingStream.h"
#include "Poco/Logger.h"
#include "Poco/URI.h"
#include "Poco/NumberParser.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/NameValueCollection.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/InvalidCertificateHandler.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/PrivateKeyPassphraseHandler.h"
#include "Poco/Net/SecureStreamSocket.h"

#include "./libjson.h"
#include "./const.h"

namespace kopsik {

std::string HTTPSClient::AppName = std::string("");
std::string HTTPSClient::AppVersion = std::string("");
std::string HTTPSClient::APIURL = std::string(kAPIURL);
bool HTTPSClient::UseProxy = false;
bool HTTPSClient::IgnoreCert = false;
kopsik::Proxy HTTPSClient::ProxySettings = Proxy();

error HTTPSClient::PostJSON(
    const std::string relative_url,
    const std::string json,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body) {
    return requestJSON(Poco::Net::HTTPRequest::HTTP_POST,
                       relative_url,
                       json,
                       basic_auth_username,
                       basic_auth_password,
                       response_body);
}

error HTTPSClient::GetJSON(
    const std::string relative_url,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body) {
    return requestJSON(Poco::Net::HTTPRequest::HTTP_GET,
                       relative_url,
                       "",
                       basic_auth_username,
                       basic_auth_password,
                       response_body);
}

error HTTPSClient::requestJSON(
    const std::string method,
    const std::string relative_url,
    const std::string json,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body) {
    return request(
        method,
        relative_url,
        json,
        basic_auth_username,
        basic_auth_password,
        response_body);
}

error HTTPSClient::request(
    const std::string method,
    const std::string relative_url,
    const std::string payload,
    const std::string basic_auth_username,
    const std::string basic_auth_password,
    std::string *response_body) {

    poco_assert(!method.empty());
    poco_assert(!relative_url.empty());

    poco_check_ptr(response_body);

    *response_body = "";

    try {
        Poco::URI uri(APIURL);

        Poco::SharedPtr<Poco::Net::InvalidCertificateHandler>
        acceptCertHandler =
            new Poco::Net::AcceptCertificateHandler(true);

        Poco::Net::Context::VerificationMode verification_mode =
            Poco::Net::Context::VERIFY_RELAXED;
        if (IgnoreCert) {
            verification_mode = Poco::Net::Context::VERIFY_NONE;
        }
        Poco::Net::Context::Ptr context = new Poco::Net::Context(
            Poco::Net::Context::CLIENT_USE, "",
            verification_mode, 9, true, "ALL");

        Poco::Net::SSLManager::instance().initializeClient(
            0, acceptCertHandler, context);

        Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(),
                                              context);
        if (ProxySettings.IsConfigured()) {
            session.setProxy(ProxySettings.host, ProxySettings.port);
            if (ProxySettings.HasCredentials()) {
                session.setProxyCredentials(ProxySettings.username,
                                            ProxySettings.password);
            }
        }
        session.setKeepAlive(false);
        session.setTimeout(Poco::Timespan(10 * Poco::Timespan::SECONDS));

        Poco::Logger &logger = Poco::Logger::get("https_client");
        {
            std::stringstream ss;
            ss << "Sending request to " << relative_url << " ..";
            logger.debug(ss.str());
        }

        Poco::Net::HTTPRequest req(method,
                                   relative_url,
                                   Poco::Net::HTTPMessage::HTTP_1_1);
        req.setKeepAlive(false);
        req.setContentType("application/json");
        req.set("User-Agent", UserAgent());
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

        // Inflate
        Poco::InflatingInputStream inflater(
            is,
            Poco::InflatingStreamBuf::STREAM_GZIP);
        std::stringstream ss;
        ss << inflater.rdbuf();
        *response_body = ss.str();

        // Log out response contents
        std::stringstream response_string;
        response_string << "Response status: " << response.getStatus()
                        << ", reason: " << response.getReason()
                        << ", Content type: " << response.getContentType();
        if (response.has("Content-Encoding")) {
            response_string << ", Content-Encoding: "
                            << response.get("Content-Encoding");
        }
        logger.debug(response_string.str());
        logger.trace(*response_body);

        if (response.getStatus() < 200 || response.getStatus() >= 300) {
            if (response_body->empty()) {
                std::stringstream description;
                description << "Request to server failed with status code: "
                            << response.getStatus();
                return description.str();
            }
            return "Data push failed with error: " + *response_body;
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

}   // namespace kopsik
