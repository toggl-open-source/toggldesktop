#ifndef ONLINE_TEST_DISPATCHER_H
#define ONLINE_TEST_DISPATCHER_H

#include "../toggl_api.h"

#include "online_test_model.h"

#include <list>
#include <any>
#include <mutex>
#include <deque>
#include <functional>
#include <variant>

namespace test {
class App;

// define the possible argument types here so we can use std::visit eventually (possible with std::variant but not with std::any)
typedef std::variant<
    bool,
    int64_t,
    uint64_t,
    std::string,
    // string list is used for the project color list
    std::list<std::string>,
    // settings is passed in a single object
    Settings,
    // TimeEntry can be passed singular on in a list
    TimeEntry,
    std::list<TimeEntry>,
    std::list<HelpArticle>,
    std::list<Autocomplete>,
    std::list<Tag>,
    std::list<Project>,
    std::list<Client>,
    std::list<Workspace>,
    std::list<Country>
> TestType;

class Dispatcher {
public:
    // wireUp is the initialization method that hooks up to the library callbacks
    static void wireUp(void *context, App *app);

    // dispatch looks if there are any pending calls from the library and executes them on the calling thread
    // if waitForEvents is true, it will freeze until new events appear in the queue
    static void dispatch(bool waitForEvents = false);

private:
    // all of these methods get called by the "dispatch" method
    // they get called in the same thread that called the "dispatch" method (that means main, in case of this test)
    // i'm using embedded classes because the calls look prettier then
    class Main {
    public:
        static void on_app(const std::vector<TestType> &args);
        static void on_pomodoro(const std::vector<TestType> &args);
        static void on_pomodoro_break(const std::vector<TestType> &args);
        static void on_sync_state(const std::vector<TestType> &args);
        static void on_update(const std::vector<TestType> &args);
        static void on_unsynced_items(const std::vector<TestType> &args);
        static void on_error(const std::vector<TestType> &args);
        static void on_online_state(const std::vector<TestType> &args);
        static void on_url(const std::vector<TestType> &args);
        static void on_login(const std::vector<TestType> &args);
        static void on_reminder(const std::vector<TestType> &args);
        static void on_help_articles(const std::vector<TestType> &args);
        static void on_time_entry_list(const std::vector<TestType> &args);
        static void on_time_entry_autocomplete(const std::vector<TestType> &args);
        static void on_mini_timer_autocomplete(const std::vector<TestType> &args);
        static void on_project_autocomplete(const std::vector<TestType> &args);
        static void on_client_select(const std::vector<TestType> &args);
        static void on_workspace_select(const std::vector<TestType> &args);
        static void on_tags(const std::vector<TestType> &args);
        static void on_time_entry_editor(const std::vector<TestType> &args);
        static void on_display_settings(const std::vector<TestType> &args);
        static void on_project_colors(const std::vector<TestType> &args);
        static void on_display_timer_state(const std::vector<TestType> &args);
        static void on_display_idle_notification(const std::vector<TestType> &args);
        static void on_countries(const std::vector<TestType> &args);
        static void on_display_overlay(const std::vector<TestType> &args);
        static void on_display_promotion(const std::vector<TestType> &args);
        static void on_display_update_download_state(const std::vector<TestType> &args);
    };

    // all of the methods here are called from the library directly
    // it is safe to assume they don't get called from the main thread
    class Worker {
    public:
        static void on_app(const bool_t open);
        static void on_pomodoro(const char_t *title, const char_t *informative_text);
        static void on_pomodoro_break(const char_t *title, const char_t *informative_text);
        static void on_sync_state(const int64_t sync_state);
        static void on_update(const char_t *url);
        static void on_unsynced_items(const int64_t count);
        static void on_error(const char *errmsg, const bool_t user_error);
        static void on_online_state(const int64_t state);
        static void on_url(const char *url);
        static void on_login(const bool_t open, const uint64_t user_id);
        static void on_reminder(const char *title, const char *informative_text);
        static void on_help_articles(TogglHelpArticleView *first);
        static void on_time_entry_list(const bool_t open, TogglTimeEntryView *first, const bool_t show_load_more);
        static void on_time_entry_autocomplete(TogglAutocompleteView *first);
        static void on_mini_timer_autocomplete(TogglAutocompleteView *first);
        static void on_project_autocomplete(TogglAutocompleteView *first);
        static void on_client_select(TogglGenericView *first);
        static void on_workspace_select(TogglGenericView *first);
        static void on_tags(TogglGenericView *first);
        static void on_time_entry_editor(const bool_t open, TogglTimeEntryView *te, const char *focused_field_name);
        static void on_display_settings(const bool_t open, TogglSettingsView *settings);
        static void on_project_colors(string_list_t color_list, const uint64_t color_count);
        static void on_display_timer_state(TogglTimeEntryView *te);
        static void on_display_idle_notification(const char_t *guid, const char_t *since, const char_t *duration, const int64_t started, const char_t *description, const char_t *project, const char_t *task, const char_t *projectColor);
        static void on_countries(TogglCountryView *first);
        static void on_display_overlay(const int64_t type);
        static void on_display_promotion(const int64_t promotion_type);
        static void on_display_update_download_state(const char_t *version, const int64_t download_state);
    };

    static std::deque<                                      // double ended queue
            std::pair<                                      // of pairs consisting of:
                std::function<void(std::vector<TestType>)>, // void functions that take a list of any objects as an argument
                std::vector<TestType>>                      // and the list of any objects
           > tasks;
    static std::recursive_mutex tasks_lock;                 // mutex to protect concurrent access to the list of tasks
    static App *app;
};

} // namespace test


#endif // ONLINE_TEST_DISPATCHER_H
