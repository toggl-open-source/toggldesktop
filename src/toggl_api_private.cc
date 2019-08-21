// Copyright 2014 Toggl Desktop developers.

#include "../src/toggl_api_private.h"

#include <cstdlib>

#include "./client.h"
#include "./context.h"
#include "./formatter.h"
#include "./project.h"
#include "./time_entry.h"
#include "./timeline_event.h"
#include "./workspace.h"

#include "Poco/Logger.h"
#include "Poco/UnicodeConverter.h"

std::string to_string(const char_t *s) {
    if (!s) {
        return std::string("");
    }
#if defined(_WIN32) || defined(WIN32)
    std::wstring ws(s);
    std::string res("");
    Poco::UnicodeConverter::toUTF8(ws, res);
    return res;
#else
    return std::string(s);
#endif
}

char_t *copy_string(const std::string s) {
#if defined(_WIN32) || defined(WIN32)
    std::wstring ws;
    Poco::UnicodeConverter::toUTF16(s, ws);
    return wcsdup(ws.c_str());
#else
    return strdup(s.c_str());
#endif
}

int compare_string(const char_t *s1, const char_t *s2) {
#if defined(_WIN32) || defined(WIN32)
    return wcscmp(s1, s2);
#else
    return strcmp(s1, s2);
#endif
}

Poco::Logger &logger() {
    return Poco::Logger::get("toggl_api");
}

toggl::Context *app(void *context) {
    poco_check_ptr(context);

    return reinterpret_cast<toggl::Context *>(context);
}
