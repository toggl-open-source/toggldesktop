// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TOGGL_API_PRIVATE_H_
#define SRC_TOGGL_API_PRIVATE_H_

#include <string>
#include <vector>

#include "./autotracker.h"
#include "./help_article.h"
#include "./proxy.h"
#include "./settings.h"
#include "./toggl_api.h"

namespace Poco {
class Logger;
}

namespace toggl {
class Client;
class Context;
class TimeEntry;
class Workspace;
namespace view {
class AutotrackerRule;
class Generic;
class HelpArticle;
class TimeEntry;
}
}  // namespace toggl

int compare_string(const char_t *s1, const char_t *s2);
int compare_string(const std::string & s1, const std::string &s2);
char_t *copy_string(const std::string &s);
std::string to_string(const char_t *s);

/**
 Trim all leading and trailing whitespace from the string. Don't trim on middle of word.
 Ex: const std::string &foo = "    too much   space  ";
 const std::string &text = trim(foo);
 // Text = "too much   space"

 @param str The string need to be trimmed
 @return The string after trimming the leading and trailing whitespace
 */
std::string trim_whitespace(const std::string &str);

TogglHelpArticleView *help_article_list_init(
    const std::vector<toggl::HelpArticle> &items);

void help_article_clear(
    TogglHelpArticleView *first);

Poco::Logger &logger();

toggl::Context *app(void *context);

#endif  // SRC_TOGGL_API_PRIVATE_H_
