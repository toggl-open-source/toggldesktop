// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_NET_CONF_H_
#define SRC_NET_CONF_H_

#include <string>

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

    static void ConfigureProxy(
        const std::string encoded_url,
        Poco::Net::HTTPSClientSession *session);
};

}  // namespace toggl

#endif  // SRC_NET_CONF_H_
