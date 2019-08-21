
#include "util.h"

#include <iostream>
#include <cstring>

namespace toggl {

std::string trim_whitespace(const std::string str)
{
    const std::string& whitespace = " \t";
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

void CustomPocoErrorHandler::exception(const Poco::Exception &exc) {
    std::cerr << "unhandled exception! " << exc.displayText() << std::endl;
}

void CustomPocoErrorHandler::exception(const std::exception &exc) {
    std::cerr << "unhandled exception! " << exc.what() << std::endl;
}

void CustomPocoErrorHandler::exception() {
    std::cerr << "unhandled exception! unknown exception" << std::endl;
}

char *copy_string(const std::string s) {
#if defined(_WIN32) || defined(WIN32)
    std::wstring ws;
    Poco::UnicodeConverter::toUTF16(s, ws);
    return wcsdup(ws.c_str());
#else
    return strdup(s.c_str());
#endif
}

int compare_string(const char *s1, const char *s2) {
#if defined(_WIN32) || defined(WIN32)
    return wcscmp(s1, s2);
#else
    return strcmp(s1, s2);
#endif
}

} // namespace toggl
