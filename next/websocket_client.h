// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_WEBSOCKET_CLIENT_H_
#define SRC_WEBSOCKET_CLIENT_H_

#include "error.h"

#include <string>
#include <vector>
#include <ctime>

#include "event_queue.h"

namespace Poco {
class Logger;

namespace Net {
class HTTPSClientSession;
class HTTPRequest;
class HTTPResponse;
class WebSocket;
}
}

namespace toggl {

class TogglClient;

typedef void (*WebSocketMessageCallback)(
    void *callback,
    std::string json);

class WebSocketClient : public Event {
 public:
    WebSocketClient(EventQueue *queue) : Event(queue),
    session_(nullptr),
    req_(nullptr),
    res_(nullptr),
    ws_(nullptr),
    on_websocket_message_(nullptr),
    ctx_(nullptr),
    last_connection_at_(0),
    api_token_("") {}
    virtual ~WebSocketClient();

    virtual void Start(
        void *ctx,
        const std::string api_token,
        WebSocketMessageCallback on_websocket_message);
    virtual void Shutdown();

 protected:
    void runActivity();

 private:
    error createSession();

    void authenticate();

    error poll();

    std::string parseWebSocketMessageType(const std::string json);

    error receiveWebSocketMessage(std::string *message);

    void deleteSession();

    int nextWebsocketRestartInterval();

    Poco::Logger &logger() const;

    Poco::Net::HTTPSClientSession *session_;
    Poco::Net::HTTPRequest *req_;
    Poco::Net::HTTPResponse *res_;
    Poco::Net::WebSocket *ws_;
    WebSocketMessageCallback on_websocket_message_;
    void *ctx_;
    TogglClient *https_client_;

    std::time_t last_connection_at_;

    std::string api_token_;
};
}  // namespace toggl

#endif  // SRC_WEBSOCKET_CLIENT_H_
