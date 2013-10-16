// Copyright 2013 Tanel Lebedev

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
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/WebSocket.h"

#include "./libjson.h"

namespace kopsik {

const std::string kTogglServerURL = "https://www.toggl.com";
// const std::string kTogglServerURL = "http://localhost:8080";

// const std::string kTogglWebSocketServerURL = "https://stream.toggl.com";
const std::string kTogglWebSocketServerURL = "https://localhost:8088";
// const std::string kTogglWebSocketServerURL = "https://echo.websocket.org";

const int kWebsocketBufSize = 1024;

error HTTPSClient::ListenToWebsocket(std::string api_token) {
  try {
    const Poco::URI uri(kTogglWebSocketServerURL);
    const Poco::Net::Context::Ptr context(new Poco::Net::Context(
      Poco::Net::Context::CLIENT_USE, "", "", "",
      Poco::Net::Context::VERIFY_NONE, 9, false,
      "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"));
    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(),
      context);
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, "/ws",
      Poco::Net::HTTPMessage::HTTP_1_1);
    req.set("Origin", "https://localhost");
    Poco::Net::HTTPResponse res;
    Poco::Net::WebSocket ws(session, req, res);

    Poco::Logger &logger = Poco::Logger::get("https_client");
    logger.debug("WebSocket connection established.");

    int flags = Poco::Net::WebSocket::FRAME_BINARY;

    // Authenticate
    JSONNODE *c = json_new(JSON_NODE);
    json_push_back(c, json_new_a("type", "authenticate"));
    json_push_back(c, json_new_a("api_token", api_token.c_str()));
    json_char *jc = json_write_formatted(c);
    std::string payload(jc);
    json_free(jc);
    json_delete(c);
    ws.sendFrame(payload.data(), payload.size(), flags);

    char buffer[kWebsocketBufSize];
    int n = ws.receiveFrame(buffer, kWebsocketBufSize, flags);
    std::stringstream ss;
    ss << "WebSocket authentication response: " << buffer;
    logger.debug(ss.str());
  } catch(const Poco::Exception& exc) {
    return exc.displayText();
  } catch(const std::exception& ex) {
    return ex.what();
  } catch(const std::string& ex) {
    return ex;
  }
  return noError;
}

error HTTPSClient::PostJSON(std::string relative_url,
        std::string json,
        std::string basic_auth_username,
        std::string basic_auth_password,
        std::string *response_body) {
    return requestJSON(Poco::Net::HTTPRequest::HTTP_POST,
        relative_url,
        json,
        basic_auth_username,
        basic_auth_password,
        response_body);
}

error HTTPSClient::GetJSON(std::string relative_url,
        std::string basic_auth_username,
        std::string basic_auth_password,
        std::string *response_body) {
    return requestJSON(Poco::Net::HTTPRequest::HTTP_GET,
        relative_url,
        "",
        basic_auth_username,
        basic_auth_password,
        response_body);
}

error HTTPSClient::requestJSON(std::string method,
        std::string relative_url,
        std::string json,
        std::string basic_auth_username,
        std::string basic_auth_password,
        std::string *response_body) {
    poco_assert(!method.empty());
    poco_assert(!relative_url.empty());
    poco_assert(response_body);
    *response_body = "";
    try {
        const Poco::URI uri(kTogglServerURL);

        const Poco::Net::Context::Ptr context(new Poco::Net::Context(
            Poco::Net::Context::CLIENT_USE, "", "", "",
            Poco::Net::Context::VERIFY_NONE, 9, false,
            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"));

        Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(),
            context);
        session.setKeepAlive(false);

        std::istringstream requestStream(json);
        Poco::DeflatingInputStream gzipRequest(requestStream,
            Poco::DeflatingStreamBuf::STREAM_GZIP);
        Poco::DeflatingStreamBuf *pBuff = gzipRequest.rdbuf();

        Poco::Int64 size = pBuff->pubseekoff(0, std::ios::end, std::ios::in);
        pBuff->pubseekpos(0, std::ios::in);

        Poco::Net::HTTPRequest req(method,
            relative_url, Poco::Net::HTTPMessage::HTTP_1_1);
        req.setKeepAlive(false);
        req.setContentType("application/json");
        req.setContentLength(size);
        req.set("Content-Encoding", "gzip");
        req.set("Accept-Encoding", "gzip");
        req.setChunkedTransferEncoding(true);

        Poco::Logger &logger = Poco::Logger::get("https_client");
        logger.debug("Sending request..");

        Poco::Net::HTTPBasicCredentials cred(
            basic_auth_username, basic_auth_password);
        cred.authenticate(req);
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
        Poco::InflatingInputStream inflater(is,
            Poco::InflatingStreamBuf::STREAM_GZIP);
        std::stringstream ss;
        ss << inflater.rdbuf();
        *response_body = ss.str();

        // Log out response contents
        std::stringstream response_string;
        response_string << "Response status: " << response.getStatus()
            << ", reason: " << response.getReason()
            << ", Content type: " << response.getContentType()
            << ", Content-Encoding: " << response.get("Content-Encoding");
        logger.debug(response_string.str());
        logger.debug(*response_body);

        if (!isStatusOK(response.getStatus())) {
            // FIXME: backoff
            return "Data push failed with error: " + *response_body;
        }

        // FIXME: reset backoff
    } catch(const Poco::Exception& exc) {
        // FIXME: backoff
        return exc.displayText();
    } catch(const std::exception& ex) {
        // FIXME: backoff
        return ex.what();
    } catch(const std::string& ex) {
        // FIXME: backoff
        return ex;
    }
    return noError;
}

}   // namespace kopsik
