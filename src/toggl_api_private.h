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

TogglGenericView *generic_to_view_item(
    const toggl::view::Generic &c);

TogglGenericView *generic_to_view_item_list(
    const std::vector<toggl::view::Generic> &list);

TogglAutotrackerRuleView *autotracker_rule_to_view_item(
    const toggl::view::AutotrackerRule &model);

void autotracker_view_item_clear(TogglAutotrackerRuleView *view);

TogglAutocompleteView *autocomplete_item_init(
    const toggl::view::Autocomplete &item);

void view_item_clear(TogglGenericView *item);

void autocomplete_item_clear(TogglAutocompleteView *item);

TogglCountryView *country_list_init(
    std::vector<TogglCountryView> *items);

void country_item_clear(TogglCountryView *item);

TogglCountryView *country_view_item_init(
    const Json::Value v);

TogglTimeEntryView *time_entry_view_item_init(
    const toggl::view::TimeEntry &te);

void time_entry_view_item_clear(TogglTimeEntryView *item);

TogglSettingsView *settings_view_item_init(
    const bool_t record_timeline,
    const toggl::Settings &settings,
    const bool_t use_proxy,
    const toggl::Proxy &proxy);

void settings_view_item_clear(TogglSettingsView *view);

TogglAutocompleteView *autocomplete_list_init(
    std::vector<toggl::view::Autocomplete> *items);

TogglHelpArticleView *help_article_list_init(
    const std::vector<toggl::HelpArticle> &items);

void help_article_clear(
    TogglHelpArticleView *first);

Poco::Logger &logger();

toggl::Context *app(void *context);

#endif  // SRC_TOGGL_API_PRIVATE_H_
