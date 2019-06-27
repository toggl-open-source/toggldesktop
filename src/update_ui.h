#ifndef UPDATE_UI_H
#define UPDATE_UI_H

#include "context.h"
#include "gui.h"

#include "Poco/Logger.h"
#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTaskAdapter.h"

namespace toggl {

class UIElements {
 public:
    UIElements()
        : first_load(false)
    , display_time_entries(false)
    , display_time_entry_autocomplete(false)
    , display_mini_timer_autocomplete(false)
    , display_project_autocomplete(false)
    , display_client_select(false)
    , display_workspace_select(false)
    , display_timer_state(false)
    , display_time_entry_editor(false)
    , open_settings(false)
    , open_time_entry_list(false)
    , open_time_entry_editor(false)
    , display_autotracker_rules(false)
    , display_settings(false)
    , time_entry_editor_guid("")
    , time_entry_editor_field("")
    , display_unsynced_items(false) {}

    static UIElements Reset();

    std::string String() const;

    void ApplyChanges(
        const std::string &editor_guid,
        const std::vector<ModelChange> &changes);

    bool first_load;
    bool display_time_entries;
    bool display_time_entry_autocomplete;
    bool display_mini_timer_autocomplete;
    bool display_project_autocomplete;
    bool display_client_select;
    bool display_workspace_select;
    bool display_timer_state;
    bool display_time_entry_editor;
    bool open_settings;
    bool open_time_entry_list;
    bool open_time_entry_editor;
    bool display_autotracker_rules;
    bool display_settings;
    std::string time_entry_editor_guid;
    std::string time_entry_editor_field;
    bool display_unsynced_items;
};

class UpdateUI
{
public:
    UpdateUI(GUI *ui);

    GUI *UI();

    Poco::Logger &logger() const;

    void update(const UIElements &what, User *user);

    void renderTimeEntryEditor(User *user, const UIElements &what);
    void renderWorkspaceSelect(User *user);
    void renderClientSelect(User *user);
    void renderTimerState(User *user);
    void renderTimeEntries(User *user, const UIElements &what);
    void renderSettings(User *user, const UIElements &what, Settings *settings);
    void renderUnsyncedItems(User *user);
    void renderAutotrackerRules(User *user);

    error ToggleEntriesGroup(std::string name);

    void OpenTimeEntryList();

    void ResetAutotrackerView();

private:
    GUI *ui_;

    // Schedule tasks using a timer:
    Poco::Mutex timer_m_;
    Poco::Util::Timer timer_;

    std::string time_entry_editor_guid_;

    std::set<std::string> autotracker_titles_;

    // To cache grouped entries open/close status
    std::map<std::string, bool_t> entry_groups;
};
}

#endif // UPDATE_UI_H
