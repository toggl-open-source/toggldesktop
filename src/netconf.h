// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_NETCONF_H_
#define SRC_NETCONF_H_

#include <string>
#include <vector>

#include "./types.h"

namespace Poco {

namespace Net {

class HTTPSClientSession;

}  // namespace Net

}  // namespace Poco

namespace toggl {

class Netconf {
 public:
    Netconf() {}
    virtual ~Netconf() {}

    static error ConfigureProxy(
        const std::string &encoded_url,
        Poco::Net::HTTPSClientSession *session);

 private:
    static error autodetectProxy(
        const std::string &encoded_url,
        std::vector<std::string> *proxy_strings);
};

}  // namespace toggl

#endif  // SRC_NETCONF_H_
