#include "online_test_dispatcher.h"

#include <iostream>
#include <thread>

#include "online_test_model.h"
#include "online_test.h"
#include "online_test_app.h"

namespace test {

std::deque<std::pair<std::function  <void(std::vector<TestType>)>, std::vector<TestType>> > Dispatcher::tasks;
std::recursive_mutex Dispatcher::tasks_lock;
App *Dispatcher::app = nullptr;

void Dispatcher::Main::on_app(const std::vector<TestType> &args) {
    app->on_app(std::get<bool>(args[0]));
}

void Dispatcher::Main::on_pomodoro(const std::vector<TestType> &args) {
    app->on_pomodoro(std::get<std::string>(args[0]),
                     std::get<std::string>(args[1]));
}

void Dispatcher::Main::on_pomodoro_break(const std::vector<TestType> &args) {
    app->on_pomodoro_break(std::get<std::string>(args[0]),
                           std::get<std::string>(args[1]));
}

void Dispatcher::Main::on_sync_state(const std::vector<TestType> &args) {
    app->on_sync_state(std::get<int64_t>(args[0]));
}

void Dispatcher::Main::on_update(const std::vector<TestType> &args) {
    app->on_update(std::get<std::string>(args[0]));
}

void Dispatcher::Main::on_unsynced_items(const std::vector<TestType> &args) {
    app->on_unsynced_items(std::get<int64_t>(args[0]));
}

void Dispatcher::Main::on_error(const std::vector<TestType> &args) {
    app->on_error(std::get<std::string>(args[0]),
                  std::get<bool>(args[1]));
}

void Dispatcher::Main::on_online_state(const std::vector<TestType> &args) {
    app->on_online_state(std::get<int64_t>(args[0]));
}

void Dispatcher::Main::on_url(const std::vector<TestType> &args) {
    app->on_url(std::get<std::string>(args[0]));
}

void Dispatcher::Main::on_login(const std::vector<TestType> &args) {
    app->on_login(std::get<bool>(args[0]),
                  std::get<uint64_t>(args[1]));
}

void Dispatcher::Main::on_reminder(const std::vector<TestType> &args) {
    app->on_reminder(std::get<std::string>(args[0]),
                     std::get<std::string>(args[1]));
}

void Dispatcher::Main::on_help_articles(const std::vector<TestType> &args) {
    app->on_help_articles(std::get<std::list<HelpArticle>>(args[0]));
}

void Dispatcher::Main::on_time_entry_list(const std::vector<TestType> &args) {
    app->on_time_entry_list(std::get<bool>(args[0]),
                            std::get<std::list<TimeEntry>>(args[1]),
                            std::get<bool>(args[2]));
}

void Dispatcher::Main::on_time_entry_autocomplete(const std::vector<TestType> &args) {
    app->on_time_entry_autocomplete(std::get<std::list<Autocomplete>>(args[0]));
}

void Dispatcher::Main::on_mini_timer_autocomplete(const std::vector<TestType> &args) {
    app->on_mini_timer_autocomplete(std::get<std::list<Autocomplete>>(args[0]));
}

void Dispatcher::Main::on_project_autocomplete(const std::vector<TestType> &args) {
    app->on_project_autocomplete(std::get<std::list<Autocomplete>>(args[0]));
}

void Dispatcher::Main::on_client_select(const std::vector<TestType> &args) {
    app->on_client_select(std::get<std::list<Client>>(args[0]));
}

void Dispatcher::Main::on_workspace_select(const std::vector<TestType> &args) {
    app->on_workspace_select(std::get<std::list<Workspace>>(args[0]));
}

void Dispatcher::Main::on_tags(const std::vector<TestType> &args) {
    app->on_tags(std::get<std::list<Tag>>(args[0]));
}

void Dispatcher::Main::on_time_entry_editor(const std::vector<TestType> &args) {
    app->on_time_entry_editor(std::get<bool>(args[0]),
                              std::get<TimeEntry>(args[1]),
                              std::get<std::string>(args[2]));
}

void Dispatcher::Main::on_display_settings(const std::vector<TestType> &args) {
    app->on_display_settings(std::get<bool>(args[0]),
                             std::get<Settings>(args[1]));
}

void Dispatcher::Main::on_project_colors(const std::vector<TestType> &args) {
    app->on_project_colors(std::get<std::list<std::string>>(args[0]),
                           std::get<uint64_t>(args[1]));
}

void Dispatcher::Main::on_obm_experiment(const std::vector<TestType> &args) {
    app->on_obm_experiment(std::get<uint64_t>(args[0]),
                           std::get<bool>(args[1]),
                           std::get<bool>(args[2]));
}

void Dispatcher::Main::on_display_timer_state(const std::vector<TestType> &args) {
    app->on_display_timer_state(std::get<TimeEntry>(args[0]));
}

void Dispatcher::Main::on_display_idle_notification(const std::vector<TestType> &args) {
    app->on_display_idle_notification(std::get<std::string>(args[0]),
                                      std::get<std::string>(args[1]),
                                      std::get<std::string>(args[2]),
                                      std::get<uint64_t>(args[3]),
                                      std::get<std::string>(args[4]),
                                      std::get<std::string>(args[5]),
                                      std::get<std::string>(args[6]),
                                      std::get<std::string>(args[7]));
}

void Dispatcher::Main::on_countries(const std::vector<TestType> &args) {
    app->on_countries(std::get<std::list<Country>>(args[0]));
}

void Dispatcher::Main::on_display_overlay(const std::vector<TestType> &args) {
    app->on_display_overlay(std::get<int64_t>(args[0]));
}

void Dispatcher::Main::on_display_promotion(const std::vector<TestType> &args) {
    app->on_display_promotion(std::get<int64_t>(args[0]));
}

void Dispatcher::Main::on_display_update_download_state(const std::vector<TestType> &args) {
    app->on_display_update_download_state(std::get<std::string>(args[0]),
                                          std::get<int64_t>(args[1]));
}

void Dispatcher::Worker::on_app(const bool_t open) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_app, std::vector<TestType>{ static_cast<bool>(open) } ));
}

void Dispatcher::Worker::on_pomodoro(const char_t *title, const char_t *informative_text) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_pomodoro, std::vector<TestType>{ std::string(title), std::string(informative_text) } ));
}

void Dispatcher::Worker::on_pomodoro_break(const char_t *title, const char_t *informative_text) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_pomodoro_break, std::vector<TestType>{ std::string(title), std::string(informative_text) } ));
}

void Dispatcher::Worker::on_sync_state(const int64_t sync_state) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_sync_state, std::vector<TestType>{ sync_state } ));
}

void Dispatcher::Worker::on_unsynced_items(const int64_t count) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_unsynced_items, std::vector<TestType>{ count } ));
}

void Dispatcher::Worker::on_online_state(const int64_t state) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_online_state, std::vector<TestType>{ state } ));
}

void Dispatcher::Worker::on_url(const char *url) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_url, std::vector<TestType>{ std::string(url) } ));
}

void Dispatcher::Worker::on_reminder(const char *title, const char *informative_text) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_reminder, std::vector<TestType>{ std::string(title), std::string(informative_text) } ));
}

void Dispatcher::Worker::on_help_articles(TogglHelpArticleView *first) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_help_articles, std::vector<TestType>{ listFromView<test::HelpArticle>(first) } ));
}

void Dispatcher::Worker::on_time_entry_autocomplete(TogglAutocompleteView *first) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_time_entry_autocomplete, std::vector<TestType>{ listFromView<test::Autocomplete>(first) } ));
}

void Dispatcher::Worker::on_mini_timer_autocomplete(TogglAutocompleteView *first) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_mini_timer_autocomplete, std::vector<TestType>{ listFromView<test::Autocomplete>(first) } ));
}

void Dispatcher::Worker::on_project_autocomplete(TogglAutocompleteView *first) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_project_autocomplete, std::vector<TestType>{ listFromView<test::Autocomplete>(first) } ));
}

void Dispatcher::Worker::on_client_select(TogglGenericView *first) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_client_select, std::vector<TestType>{ listFromView<test::Client>(first) } ));
}

void Dispatcher::Worker::on_workspace_select(TogglGenericView *first) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_workspace_select, std::vector<TestType>{ listFromView<test::Workspace>(first) } ));
}

void Dispatcher::Worker::on_tags(TogglGenericView *first) {

    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_tags, std::vector<TestType>{ listFromView<test::Tag>(first) } ));
}

void Dispatcher::Worker::on_project_colors(string_list_t color_list, const uint64_t color_count) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_project_colors, std::vector<TestType>{ listFromView<std::string, char**>(color_list), color_count } ));
}

void Dispatcher::Worker::on_display_timer_state(TogglTimeEntryView *te) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_display_timer_state, std::vector<TestType>{ oneFromView<test::TimeEntry>(te) } ));
}

void Dispatcher::Worker::on_display_idle_notification(const char_t *guid, const char_t *since, const char_t *duration, const int64_t started, const char_t *description, const char_t *project, const char_t *task, const char_t *projectColor) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_display_idle_notification, std::vector<TestType>{ std::string(guid), std::string(since), std::string(duration), started, std::string(description), std::string(project), std::string(task), std::string(projectColor) } ));
}

void Dispatcher::Worker::on_countries(TogglCountryView *first) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_countries, std::vector<TestType>{ listFromView<test::Country>(first) } ));
}

void Dispatcher::Worker::on_obm_experiment(const uint64_t nr, const bool_t included, const bool_t seen) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_obm_experiment, std::vector<TestType>{ nr, static_cast<bool>(included), static_cast<bool>(seen) } ));
}

void Dispatcher::Worker::on_display_settings(const bool_t open, TogglSettingsView *settings) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_display_settings, std::vector<TestType>{ static_cast<bool>(open), oneFromView<test::Settings>(settings) } ));
}

void Dispatcher::Worker::on_time_entry_editor(const bool_t open, TogglTimeEntryView *te, const char *focused_field_name) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_time_entry_editor, std::vector<TestType>{ static_cast<bool>(open), oneFromView<test::TimeEntry>(te), std::string(focused_field_name) } ));
}

void Dispatcher::Worker::on_time_entry_list(const bool_t open, TogglTimeEntryView *first, const bool_t show_load_more) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_time_entry_list, std::vector<TestType>{ static_cast<bool>(open), listFromView<test::TimeEntry>(first), static_cast<bool>(show_load_more) } ));
}

void Dispatcher::Worker::on_login(const bool_t open, const uint64_t user_id) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_login, std::vector<TestType>{ static_cast<bool>(open), user_id } ));
}

void Dispatcher::Worker::on_error(const char *errmsg, const bool_t user_error) {
    std::cerr << "Error occurred: " << errmsg << std::endl;
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_error, std::vector<TestType>{ std::string(errmsg), static_cast<bool>(user_error) } ));
}

void Dispatcher::Worker::on_update(const char_t *url) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_update, std::vector<TestType>{ std::string(url) } ));
}

void Dispatcher::Worker::on_display_overlay(const int64_t type) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_display_overlay, std::vector<TestType>{ type } ));
}

void Dispatcher::Worker::on_display_promotion(const int64_t promotion_type) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_display_promotion, std::vector<TestType>{ promotion_type } ));
}

void Dispatcher::Worker::on_display_update_download_state(const char *version, const int64_t download_state) {
    std::scoped_lock l(tasks_lock);
    tasks.emplace_back(std::make_pair( Main::on_display_update_download_state, std::vector<TestType>{ std::string(version), download_state } ));
}

void Dispatcher::dispatch(bool waitForEvents) {
    bool hasDispatched = false;
    while (true) {
        std::unique_lock l(tasks_lock);
        if (tasks.empty()) {
            l.unlock();
            if (waitForEvents && !hasDispatched) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            else {
                return;
            }
        }
        else {
            hasDispatched = true;
            auto task = tasks.front();
            tasks.pop_front();
            l.unlock();
            task.first(task.second);
        }
    }
}

void Dispatcher::wireUp(void *context, App *app) {
    Dispatcher::app = app;

    toggl_on_error(context, Worker::on_error);
    toggl_on_show_app(context, Worker::on_app);
    toggl_on_sync_state(context, Worker::on_sync_state);
    toggl_on_unsynced_items(context, Worker::on_unsynced_items);
    toggl_on_update(context, Worker::on_update);
    toggl_on_online_state(context, Worker::on_online_state);
    toggl_on_login(context, Worker::on_login);
    toggl_on_url(context, Worker::on_url);
    toggl_on_reminder(context, Worker::on_reminder);
    toggl_on_time_entry_list(context, Worker::on_time_entry_list);
    toggl_on_time_entry_autocomplete(context, Worker::on_time_entry_autocomplete);
    toggl_on_mini_timer_autocomplete(context, Worker::on_mini_timer_autocomplete);
    toggl_on_project_autocomplete(context, Worker::on_project_autocomplete);
    toggl_on_workspace_select(context, Worker::on_workspace_select);
    toggl_on_client_select(context, Worker::on_client_select);
    toggl_on_tags(context, Worker::on_tags);
    toggl_on_time_entry_editor(context, Worker::on_time_entry_editor);
    toggl_on_settings(context, Worker::on_display_settings);
    toggl_on_timer_state(context, Worker::on_display_timer_state);
    toggl_on_idle_notification(context, Worker::on_display_idle_notification);
    toggl_on_project_colors(context, Worker::on_project_colors);
    toggl_on_help_articles(context, Worker::on_help_articles);
    toggl_on_obm_experiment(context, Worker::on_obm_experiment);
    toggl_on_pomodoro(context, Worker::on_pomodoro);
    toggl_on_pomodoro_break(context, Worker::on_pomodoro_break);
    toggl_on_countries(context, Worker::on_countries);
    toggl_on_overlay(context, Worker::on_display_overlay);
    toggl_on_promotion(context, Worker::on_display_promotion);
    toggl_on_update_download_state(context, Worker::on_display_update_download_state);
}

} // namespace test
