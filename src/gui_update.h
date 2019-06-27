#ifndef GUI_UPDATE_H
#define GUI_UPDATE_H

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

class GUIUpdate
{
public:
    GUIUpdate(Context *context);

    Context *context();
    GUI *UI();
    User *user();

    Poco::Logger &logger() const;

    void update(const UIElements &what);

    void renderEverything();

    void renderTimeEntryEditor(const UIElements &what);
    void renderWorkspaceSelect();
    void renderClientSelect();
    void renderTimerState();
    void renderTimeEntries(bool open = false);
    void renderSettings(Settings *settings, bool open = false);
    void renderUnsyncedItems();
    void renderAutotrackerRules();

    error ToggleEntriesGroup(std::string name);

    void OpenTimeEntryList();

    void ResetAutotrackerView();

    Poco::LocalDateTime LastTimeEntryRenderTime();

    // TODO remove this later
    bool isTimeEntryLocked(TimeEntry* te);
    bool isTimeLockedInWorkspace(time_t t, Workspace* ws);
    bool canChangeStartTimeTo(TimeEntry* te, time_t t);
    bool canChangeProjectTo(TimeEntry* te, Project* p);

private:
    Context *context_;

    // Schedule tasks using a timer:
    Poco::Mutex timer_m_;
    Poco::Util::Timer timer_;

    std::string time_entry_editor_guid_;

    std::set<std::string> autotracker_titles_;

    // To cache grouped entries open/close status
    std::map<std::string, bool_t> entry_groups;

    Poco::LocalDateTime last_time_entry_list_render_at_;
};
}

#endif // GUI_UPDATE_H
