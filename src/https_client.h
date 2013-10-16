// Copyright 2013 Tanel Lebedev

#ifndef SRC_HTTPS_CLIENT_H_
#define SRC_HTTPS_CLIENT_H_

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

  class HTTPSClient {
  public:
    HTTPSClient() : activity_(this, &HTTPSClient::runActivity),
      session_(0),
      req_(0),
      res_(0),
      ws_(0),
      on_websocket_message_(0),
      ctx_(0) {}
    virtual ~HTTPSClient() {
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
    virtual error StartWebSocketActivity(
      void *ctx,
      std::string api_token,
      WebSocketMessageCallback on_websocket_message);
    virtual void StopWebSocketActivity();
    virtual error PostJSON(std::string relative_url,
      std::string json,
      std::string basic_auth_username,
      std::string basic_auth_password,
      std::string *response_body);
    virtual error GetJSON(std::string relative_url,
      std::string basic_auth_username,
      std::string basic_auth_password,
      std::string *response_body);

  protected:
    void runActivity();

  private:
    error requestJSON(std::string method,
      std::string relative_url,
      std::string json,
      std::string basic_auth_username,
      std::string basic_auth_password,
      std::string *response_body);

    std::string parseWebSocketMessageType(std::string json);
    std::string receiveWebSocketMessage();

    Poco::Activity<HTTPSClient> activity_;
    Poco::Net::HTTPSClientSession *session_;
    Poco::Net::HTTPRequest *req_;
    Poco::Net::HTTPResponse *res_;
    Poco::Net::WebSocket *ws_;
    WebSocketMessageCallback on_websocket_message_;
    void *ctx_;
  };
}  // namespace kopsik

#endif  // SRC_HTTPS_CLIENT_H_
