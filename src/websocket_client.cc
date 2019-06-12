// Copyright 2014 Toggl Desktop developers.

#include "../src/websocket_client.h"

#include <string>
#include <sstream>

#include <json/json.h>  // NOLINT

#include "Poco/Exception.h"
#include "Poco/Logger.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/InvalidCertificateHandler.h"
#include "Poco/Net/PrivateKeyPassphraseHandler.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Random.h"
#include "Poco/URI.h"

#include "./const.h"
#include "./https_client.h"
#include "./netconf.h"
#include "./urls.h"

namespace toggl {

WebSocketClient::~WebSocketClient() {
    deleteSession();
}

void WebSocketClient::Start(
    void *ctx,
    const std::string &api_token,
    WebSocketMessageCallback on_websocket_message) {

    poco_check_ptr(ctx);
    poco_check_ptr(on_websocket_message);

    if (api_token.empty()) {
        logger().error("API token is empty, cannot start websocket");
        return;
    }

    Poco::Mutex::ScopedLock lock(mutex_);

    if (activity_.isRunning()) {
        return;
    }

    activity_.start();

    ctx_ = ctx;
    on_websocket_message_ = on_websocket_message;
    api_token_ = api_token;
}

void WebSocketClient::Shutdown() {
    logger().debug("Shutdown");

    if (!activity_.isRunning()) {
        return;
    }
    activity_.stop();  // request stop
    activity_.wait();  // wait until activity actually stops

    deleteSession();

    logger().debug("Shutdown done");
}

error WebSocketClient::createSession() {
    logger().debug("createSession");

    if (HTTPSClient::Config.CACertPath.empty()) {
        return error("Missing CA certifcate, cannot start Websocket");
    }

    Poco::Mutex::ScopedLock lock(mutex_);

    deleteSession();

    last_connection_at_ = time(nullptr);

    error err = TogglClient::TogglStatus.Status();
    if (err != noError) {
        std::stringstream ss;
        ss << "Will not start Websocket sessions, ";
        ss << "because of known bad Toggl status: " << err;
        logger().error(ss.str());
        return err;
    }

    try {
        Poco::URI uri(urls::WebSocket());

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

        session_ = new Poco::Net::HTTPSClientSession(
            uri.getHost(),
            uri.getPort(),
            context);

        Netconf::ConfigureProxy(urls::WebSocket(), session_);

        req_ = new Poco::Net::HTTPRequest(
            Poco::Net::HTTPRequest::HTTP_GET, "/ws",
            Poco::Net::HTTPMessage::HTTP_1_1);
        req_->set("Origin", "https://localhost");
        req_->set("User-Agent", HTTPSClient::Config.UserAgent());
        res_ = new Poco::Net::HTTPResponse();
        ws_ = new Poco::Net::WebSocket(*session_, *req_, *res_);
        ws_->setBlocking(false);
        ws_->setReceiveTimeout(Poco::Timespan(3 * Poco::Timespan::SECONDS));
        ws_->setSendTimeout(Poco::Timespan(3 * Poco::Timespan::SECONDS));

        authenticate();
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string & ex) {
        return ex;
    }

    return noError;
}

void WebSocketClient::authenticate() {
    logger().debug("authenticate");

    Json::Value c;
    c["type"] = "authenticate";
    c["api_token"] = api_token_;

    Json::StyledWriter writer;
    std::string payload = writer.write(c);

    ws_->sendFrame(payload.data(),
                   static_cast<int>(payload.size()),
                   Poco::Net::WebSocket::FRAME_BINARY);
}

std::string WebSocketClient::parseWebSocketMessageType(
    const std::string &json) {

    if (json.empty()) {
        return "";
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json, root)) {
        return "";
    }

    if (root.isMember("type")) {
        return root["type"].asString();
    }

    return "data";
}

const int kWebsocketBufSize = 1024 * 10;

error WebSocketClient::receiveWebSocketMessage(std::string *message) {
    int flags = Poco::Net::WebSocket::FRAME_BINARY;
    std::string json("");
    try {
        char buf[kWebsocketBufSize];
        int n = ws_->receiveFrame(buf, kWebsocketBufSize, flags);
        if (n > 0) {
            json.append(buf, static_cast<unsigned>(n));
        }
    } catch(const Poco::Exception& exc) {
        return error(exc.displayText());
    } catch(const std::exception& ex) {
        return error(ex.what());
    } catch(const std::string & ex) {
        return error(ex);
    }
    *message = json;
    return noError;
}

const std::string &kPong("{\"type\": \"pong\"}");

error WebSocketClient::poll() {
    try {
        Poco::Timespan span(250 * Poco::Timespan::MILLISECONDS);
        if (!ws_->poll(span, Poco::Net::Socket::SELECT_READ)) {
            return noError;
        }

        std::string json("");
        error err = receiveWebSocketMessage(&json);
        if (err != noError) {
            return err;
        }
        if (json.empty()) {
            return error("WebSocket closed the connection");
        }
        std::stringstream ss;
        ss << "WebSocket message: " << json;
        logger().trace(ss.str());

        last_connection_at_ = time(nullptr);

        std::string type = parseWebSocketMessageType(json);

        if (activity_.isStopped()) {
            return noError;
        }

        if ("ping" == type) {
            ws_->sendFrame(kPong.data(),
                           static_cast<int>(kPong.size()),
                           Poco::Net::WebSocket::FRAME_BINARY);
            return noError;
        }

        if ("data" == type) {
            on_websocket_message_(ctx_, json);
        }
    } catch(const Poco::Exception& exc) {
        return error(exc.displayText());
    } catch(const std::exception& ex) {
        return error(ex.what());
    } catch(const std::string & ex) {
        return error(ex);
    }
    return noError;
}

void WebSocketClient::runActivity() {
    int restart_interval = nextWebsocketRestartInterval();
    while (!activity_.isStopped()) {
        if (ws_) {
            error err = poll();
            if (err != noError) {
                logger().error(err);
                logger().debug("encountered an error and will delete session");
                deleteSession();
                logger().debug("will sleep for 10 sec");
                for (int i = 0; i < 20; i++) {
                    if (activity_.isStopped()) {
                        return;
                    }
                    Poco::Thread::sleep(500);
                }
                logger().debug("sleep done");
            }
        }

        if (time(nullptr) - last_connection_at_ > restart_interval) {
            restart_interval = nextWebsocketRestartInterval();
            logger().debug("restarting");
            error err = createSession();
            if (err != noError) {
                logger().error(err);
                for (int i = 0; i < 20; i++) {
                    if (activity_.isStopped()) {
                        return;
                    }
                    Poco::Thread::sleep(500);
                }
            }
        }

        Poco::Thread::sleep(1000);
    }

    logger().debug("activity finished");
}

void WebSocketClient::deleteSession() {
    logger().debug("deleteSession");

    Poco::Mutex::ScopedLock lock(mutex_);

    if (ws_) {
        delete ws_;
        ws_ = nullptr;
    }
    if (res_) {
        delete res_;
        res_ = nullptr;
    }
    if (req_) {
        delete req_;
        req_ = nullptr;
    }
    if (session_) {
        delete session_;
        session_ = nullptr;
    }

    logger().debug("session deleted");
}

Poco::Logger &WebSocketClient::logger() const {
    return Poco::Logger::get("websocket_client");
}

int WebSocketClient::nextWebsocketRestartInterval() {
    Poco::Random random;
    random.seed();
    int res = static_cast<int>(random.next(kWebsocketRestartRangeSeconds)) + 1;
    std::stringstream ss;
    ss << "Next websocket restart in " << res << " seconds";
    logger().trace(ss.str());
    return res;
}

}   // namespace toggl
