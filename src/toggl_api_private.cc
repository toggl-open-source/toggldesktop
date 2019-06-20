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

std::string trim_whitespace(const std::string &str)
{
    const std::string & whitespace = " \t";
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

char_t *copy_string(const std::string &s) {
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

int compare_string(const std::string &s1, const std::string &s2) {
    return compare_string(s1.c_str(), s2.c_str());
}

TogglHelpArticleView *help_artice_init(
    const toggl::HelpArticle &item) {
    TogglHelpArticleView *result = new TogglHelpArticleView();
    /*
    result->Category = copy_string(item.Type);
    result->Name = copy_string(item.Name);
    result->URL = copy_string(item.URL);
    result->Next = nullptr;

    */
    return result;
}

void help_article_clear(TogglHelpArticleView *item) {
    if (!item) {
        return;
    }

    /*
    free(item->Category);
    item->Category = nullptr;

    free(item->Name);
    item->Name = nullptr;

    free(item->URL);
    item->URL = nullptr;

    if (item->Next) {
        TogglHelpArticleView *next =
            reinterpret_cast<TogglHelpArticleView *>(item->Next);
        help_article_clear(next);
        item->Next = nullptr;
    }

    delete item;
    */
}

TogglHelpArticleView *help_article_list_init(const std::vector<toggl::HelpArticle> &items) {
    TogglHelpArticleView *first = nullptr;
    for (std::vector<toggl::HelpArticle>::const_reverse_iterator it =
        items.rbegin();
            it != items.rend();
            ++it) {
        TogglHelpArticleView *item = help_artice_init(*it);
        //item->Next = first;
        first = item;
    }
    return first;
}

Poco::Logger &logger() {
    return Poco::Logger::get("toggl_api");
}

toggl::Context *app(void *context) {
    poco_check_ptr(context);

    return reinterpret_cast<toggl::Context *>(context);
}
