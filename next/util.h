
#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include <string>
#include <Poco/ErrorHandler.h>

namespace toggl {

std::string trim_whitespace(const std::string str);

char *copy_string(const std::string s);
int compare_string(const char *s1, const char *s2);

class CustomPocoErrorHandler : public Poco::ErrorHandler {
 public:
    void exception(const Poco::Exception& exc);
    void exception(const std::exception& exc);
    void exception();
};


} // namespace toggl

#endif // SRC_UTIL_H_
