// Copyright 2013 Tanel Lebedev

#ifndef SRC_WEBSOCKET_CLIENT_H_
#define SRC_WEBSOCKET_CLIENT_H_

#include <string>
#include <vector>

#include "Poco/Activity.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"

#include "./types.h"

namespace kopsik {

  typedef void (*WebSocketMessageCallback)(
    void *callback,
    std::string json);

  class WebSocketClient {
  public:
    explicit WebSocketClient(
        const std::string websocket_url,
        const std::string app_name,
        const std::string app_version) :
      activity_(this, &WebSocketClient::runActivity),
      session_(0),
      req_(0),
      res_(0),
      ws_(0),
      on_websocket_message_(0),
      ctx_(0),
      websocket_url_(websocket_url),
      app_name_(app_name),
      app_version_(app_version) {}
    virtual ~WebSocketClient() {
      if (ws_) {
        delete ws_;
        ws_ = 0;
      }
      if (res_) {
        delete res_;
        res_ = 0;
      }
      if (req_) {
        delete req_;
        req_ = 0;
      }
      if (session_) {
        delete session_;
        session_ = 0;
      }
    }
    virtual error Start(
      void *ctx,
      std::string api_token,
      WebSocketMessageCallback on_websocket_message);
    virtual void Stop();

    void SetWebsocketURL(std::string value) {
      websocket_url_ = value;
    }

  protected:
    void runActivity();

  private:
    std::string parseWebSocketMessageType(std::string json);
    std::string receiveWebSocketMessage();

    Poco::Activity<WebSocketClient> activity_;
    Poco::Net::HTTPSClientSession *session_;
    Poco::Net::HTTPRequest *req_;
    Poco::Net::HTTPResponse *res_;
    Poco::Net::WebSocket *ws_;
    WebSocketMessageCallback on_websocket_message_;
    void *ctx_;

    std::string websocket_url_;
    std::string app_name_;
    std::string app_version_;
  };
}  // namespace kopsik

#endif  // SRC_WEBSOCKET_CLIENT_H_
