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
class HelpArticle;
class Generic;
class AutotrackerRule;
}
}  // namespace toggl

int compare_string(const char_t *s1, const char_t *s2);
char_t *copy_string(const std::string s);
std::string to_string(const char_t *s);

TogglGenericView *view_item_init();

TogglGenericView *tag_to_view_item(
    const std::string tag_name);

TogglGenericView *workspace_to_view_item(
    toggl::Workspace * const);

TogglGenericView *client_to_view_item(
    const toggl::view::Generic c);

TogglAutotrackerRuleView *autotracker_rule_to_view_item(
    const toggl::view::AutotrackerRule model);

void autotracker_view_item_clear(TogglAutotrackerRuleView *view);

TogglAutocompleteView *autocomplete_item_init(
    const toggl::view::Autocomplete item);

void view_item_clear(TogglGenericView *item);

void autocomplete_item_clear(TogglAutocompleteView *item);

TogglTimeEntryView *time_entry_view_item_init(
    const toggl::TimeEntry *te,
    const std::string workspace_name,
    const std::string project_and_task_label,
    const std::string task_label,
    const std::string project_label,
    const std::string client_label,
    const std::string color,
    const std::string date_duration,
    const bool time_in_timer_format);

void time_entry_view_item_clear(TogglTimeEntryView *item);

TogglSettingsView *settings_view_item_init(
    const bool_t record_timeline,
    const toggl::Settings settings,
    const bool_t use_proxy,
    const toggl::Proxy proxy);

void settings_view_item_clear(TogglSettingsView *view);

TogglAutocompleteView *autocomplete_list_init(
    std::vector<toggl::view::Autocomplete> *items);

TogglHelpArticleView *help_article_list_init(
    const std::vector<toggl::HelpArticle> items);

void help_article_clear(
    TogglHelpArticleView *first);

Poco::Logger &logger();

toggl::Context *app(void *context);

#endif  // SRC_TOGGL_API_PRIVATE_H_
