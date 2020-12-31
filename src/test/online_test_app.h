#ifndef ONLINE_TEST_APP_H
#define ONLINE_TEST_APP_H

#include "online_test_model.h"

#include <set>

#define TESTDB "test.db"

namespace test {

class Dispatcher;

class App {
public:
    App();
    ~App();

    static std::string randomString(size_t length);
    static std::string randomUser();
    static std::string randomPassword();

    std::string errorSinceLastTime();

    void getCountries();

    void uiStart();
    bool isStarted() const;

    bool signup(std::string name, std::string password, const Country &country);
    bool signup(std::string name, std::string password);
    bool login(std::string name, std::string password);
    bool logout();
    bool isLoggedIn() const;

    std::string start(const std::string &description);
    const TimeEntry &runningTimeEntry() const;
    bool stop();

    bool timeEntry_setDescription(const std::string &guid, const std::string &value);
    bool timeEntry_setStart(const std::string &guid, const std::string &value);
    bool timeEntry_setEnd(const std::string &guid, const std::string &value);
    bool timeEntry_setDuration(const std::string &guid, const std::string &value);
    bool timeEntry_setBillable(const std::string &guid, bool value);
    bool timeEntry_setTags(const std::string &guid, const std::list<std::string> &value);
    bool timeEntry_setDate(const std::string &guid, int64_t value);

    std::string client_create(const std::string &name);

    std::string timeEntry_addProject(const std::string &teGuid, const std::string &cGuid, const std::string &name, const std::string &color = "#06aaf5");
    bool timeEntry_setProject(const std::string &teGuid, const std::string &pGuid);

    bool settings_remindDays(bool remind_mon, bool remind_tue, bool remind_wed, bool remind_thu, bool remind_fri, bool remind_sat, bool remind_sun);
    bool settings_remindTimes(const std::string &remind_starts, const std::string &remind_ends);
    bool settings_useIdleDetection(bool use_idle_detection);
    bool settings_Autotrack(bool value);
    bool settings_openEditorOnShortcut(bool value);
    bool settings_autodetectProxy(bool autodetect_proxy);
    bool settings_menubarTimer(bool menubar_timer);
    bool settings_menubarProject(bool menubar_project);
    bool settings_dockIcon(bool dock_icon);
    bool settings_onTop(bool on_top);
    bool settings_reminder(bool reminder);
    bool settings_pomodoro(bool pomodoro);
    bool settings_pomodoroBreak(bool pomodoro_break);
    bool settings_stopEntryOnShutdownSleep(bool stop_entry);
    bool settings_idleMinutes(uint64_t idle_minutes);
    bool settings_focusOnShortcut(bool focus_on_shortcut);
    bool settings_reminderMinutes(uint64_t reminder_minutes);
    bool settings_pomodoroMinutes(uint64_t pomodoro_minutes);
    bool settings_pomodoroBreakMinutes(uint64_t pomodoro_break_minutes);
    bool settings_manualMode(bool manual_mode);
    bool settings_analyticsOptedOut(bool analytics_opted_out);
    bool settings_proxy(bool use_proxy, const std::string &proxy_host, uint64_t proxy_port, const std::string &proxy_username, const std::string &proxy_password);

    const std::set<Country> &countries() const;
    const std::list<TimeEntry> &timeEntries() const;
    const std::set<Client> &clients() const;
    const Settings &settings() const;

private:
    void *context_;
    std::string lastError_;

    bool isStarted_;

    std::string loggedInEmail_;
    uint64_t loggedInId_;

    TimeEntry runningTimeEntry_;
    Workspace selectedWorkspace_;

    std::set<Country> countries_;
    std::set<Workspace> workspaces_;
    std::set<Project> projects_;
    std::set<Client> clients_;
    std::list<TimeEntry> timeEntries_;
    std::set<Tag> tags_;
    std::set<Autocomplete> timeEntryAutocomplete_;
    std::set<Autocomplete> miniTimerAutocomplete_;
    std::set<Autocomplete> projectAutocomplete_;
    Settings settings_;

    // callbacks
    friend class Dispatcher;
    void on_app(bool open);
    void on_pomodoro(const std::string &title, const std::string &informative_text);
    void on_pomodoro_break(const std::string &title, const std::string &informative_text);
    void on_sync_state(int64_t sync_state);
    void on_update(const std::string &url);
    void on_unsynced_items(int64_t count);
    void on_error(const std::string &errmsg, bool user_error);
    void on_online_state(int64_t state);
    void on_url(const std::string &url);
    void on_login(bool open, uint64_t user_id);
    void on_reminder(const std::string &title, const std::string &informative_text);
    void on_help_articles(const std::list<HelpArticle> &list);
    void on_time_entry_list(bool open, const std::list<TimeEntry> &list, bool show_load_more);
    void on_time_entry_autocomplete(const std::list<Autocomplete> &list);
    void on_mini_timer_autocomplete(const std::list<Autocomplete> &list);
    void on_project_autocomplete(const std::list<Autocomplete> &list);
    void on_client_select(const std::list<Client> &list);
    void on_workspace_select(const std::list<Workspace> &list);
    void on_tags(const std::list<Tag> &list);
    void on_time_entry_editor(bool open, const TimeEntry &te, const std::string &focused_field_name);
    void on_display_settings(bool open, const Settings &settings);
    void on_project_colors(const std::list<std::string> &color_list, uint64_t color_count);
    void on_display_timer_state(const TimeEntry &te);
    void on_display_idle_notification(const std::string &guid, const std::string &since, const std::string &duration, uint64_t started, const std::string &description, const std::string &project, const std::string &task, const std::string &projectColor);
    void on_countries(const std::list<Country> &list);
    void on_display_overlay(int64_t type);
    void on_display_promotion(int64_t promotion_type);
    void on_display_update_download_state(const std::string &version, int64_t download_state);
};

} // namespace test;

#endif // ONLINE_TEST_APP_H
