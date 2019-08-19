
#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include <string>
#include <Poco/ErrorHandler.h>

namespace toggl {

std::string trim_whitespace(const std::string str);

class CustomPocoErrorHandler : public Poco::ErrorHandler {
 public:
    void exception(const Poco::Exception& exc);
    void exception(const std::exception& exc);
    void exception();
};


} // namespace toggl

#endif // SRC_UTIL_H_
