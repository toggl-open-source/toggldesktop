// Copyright 2013 Tanel Lebedev

#ifndef SRC_HTTPS_CLIENT_H_
#define SRC_HTTPS_CLIENT_H_

#include <string>
#include <vector>

#include "Poco/Activity.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"

#include "./types.h"

namespace kopsik {

  class HTTPSClient {
  public:
    HTTPSClient() {}
    virtual ~HTTPSClient() {}
    virtual error PostJSON(std::string relative_url,
      std::string json,
      std::string basic_auth_username,
      std::string basic_auth_password,
      std::string *response_body);
    virtual error GetJSON(std::string relative_url,
      std::string basic_auth_username,
      std::string basic_auth_password,
      std::string *response_body);

  private:
    error requestJSON(std::string method,
      std::string relative_url,
      std::string json,
      std::string basic_auth_username,
      std::string basic_auth_password,
      std::string *response_body);
  };
}  // namespace kopsik

#endif  // SRC_HTTPS_CLIENT_H_
