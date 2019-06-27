#include "gui_update.h"
#include "model/client.h"
#include "model/time_entry.h"
#include "model/project.h"

template <typename T>
static void link_vector(std::vector<T> &v) {
    T *previous = nullptr;
    for (auto &i : v) {
        if (previous)
            i.Next = previous;
        previous = &i;
    }
    if (previous)
        previous->Next = nullptr;
}

namespace toggl {



UIElements UIElements::Reset() {
    UIElements render;
    render.first_load = true;
    render.display_time_entries = true;
    render.display_time_entry_autocomplete = true;
    render.display_mini_timer_autocomplete = true;
    render.display_project_autocomplete = true;
    render.display_client_select = true;
    render.display_workspace_select = true;
    render.display_timer_state = true;
    render.display_time_entry_editor = true;
    render.display_autotracker_rules = true;
    render.display_settings = true;
    render.display_unsynced_items = true;

    render.open_time_entry_list = true;

    return render;
}

std::string UIElements::String() const {
    std::stringstream ss;
    if (display_time_entries) {
        ss << "display_time_entries ";
    }
    if (display_time_entry_autocomplete) {
        ss << "display_time_entry_autocomplete ";
    }
    if (display_mini_timer_autocomplete) {
        ss << "display_mini_timer_autocomplete ";
    }
    if (display_project_autocomplete) {
        ss << "display_project_autocomplete ";
    }
    if (display_client_select) {
        ss << "display_client_select ";
    }
    if (display_client_select) {
        ss << "display_client_select ";
    }
    if (display_workspace_select) {
        ss << "display_workspace_select ";
    }
    if (display_timer_state) {
        ss << "display_timer_state ";
    }
    if (display_time_entry_editor) {
        ss << "display_time_entry_editor ";
    }
    if (open_settings) {
        ss << "open_settings ";
    }
    if (open_time_entry_list) {
        ss << "open_time_entry_list ";
    }
    if (open_time_entry_editor) {
        ss << "open_time_entry_editor ";
    }
    if (display_autotracker_rules) {
        ss << "display_autotracker_rules ";
    }
    if (display_settings) {
        ss << "display_settings ";
    }
    if (!time_entry_editor_guid.empty()) {
        ss << "time_entry_editor_guid=" << time_entry_editor_guid << " ";
    }
    if (!time_entry_editor_field.empty()) {
        ss << "time_entry_editor_field=" << time_entry_editor_field << " ";
    }
    if (display_unsynced_items) {
        ss << "display_unsynced_items ";
    }
    return ss.str();
}

void UIElements::ApplyChanges(
    const std::string &editor_guid,
    const std::vector<ModelChange> &changes) {

    time_entry_editor_guid = editor_guid;

    // Check what needs to be updated in UI
    for (std::vector<ModelChange>::const_iterator it =
        changes.begin();
            it != changes.end();
            ++it) {
        ModelChange ch = *it;

        if (ch.ModelType() == kModelWorkspace
                || ch.ModelType() == kModelClient
                || ch.ModelType() == kModelProject
                || ch.ModelType() == kModelTask
                || ch.ModelType() == kModelTimeEntry) {
            display_time_entry_autocomplete = true;
            display_time_entries = true;
            display_mini_timer_autocomplete = true;
        }

        if (ch.ModelType() == kModelWorkspace
                || ch.ModelType() == kModelClient
                || ch.ModelType() == kModelProject
                || ch.ModelType() == kModelTask) {
            display_project_autocomplete = true;
        }

        if (ch.ModelType() == kModelClient
                || ch.ModelType() == kModelWorkspace) {
            display_client_select = true;
        }

        // Check if time entry editor needs to be updated
        if (ch.ModelType() == kModelTimeEntry) {
            display_timer_state = true;
            // If time entry was edited, check further
            if (time_entry_editor_guid == ch.GUID()) {
                // If time entry was deleted, close editor
                // and open list view
                if (ch.ChangeType() == kChangeTypeDelete) {
                    open_time_entry_list = true;
                    display_time_entries = true;
                } else {
                    display_time_entry_editor = true;
                }
            }
        }

        if (ch.ModelType() == kModelAutotrackerRule) {
            display_autotracker_rules = true;
        }

        if (ch.ModelType() == kModelSettings) {
            display_settings = true;
        }
    }
}



GUIUpdate::GUIUpdate(Context *ctx)
    : context_(ctx)
{

}

Context *GUIUpdate::context() {
    return context_;
}

GUI *GUIUpdate::UI() {
    return context()->UI();
}

User *GUIUpdate::user() {
    return context()->user();
}

Poco::Logger &GUIUpdate::logger() const {
    return Poco::Logger::get("context");
}

void GUIUpdate::renderEverything() {

}

void GUIUpdate::update(const UIElements &what) {
    logger().debug("GUIUpdate " + what.String());

    std::vector<view::Autocomplete> time_entry_autocompletes;
    std::vector<view::Autocomplete> minitimer_autocompletes;
    std::vector<view::Autocomplete> project_autocompletes;



    // If user is already editing the time entry, toggle the editor
    // instead of doing nothing
    /*
    if (what.open_time_entry_editor && what.display_time_entry_editor && time_entry_editor_guid_ == te->GUID()) {
        what.open_time_entry_editor = false;
        what.display_time_entry_editor = false;
        what.time_entry_editor_guid = "";
        what.time_entry_editor_field = "";

        what.open_time_entry_list = true;
        what.display_time_entries = true;
    }
    */



    //////////////////////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////////////////////////


    // Collect data
    {
        //Poco::Mutex::ScopedLock lock(user_m_);

        // Render data
        if (what.display_time_entry_editor)
            renderTimeEntryEditor(what);

        if (what.display_time_entries)
            renderTimeEntries(what);

        if (what.display_time_entry_autocomplete) {
            if (what.first_load) {
                if (user()) {
                    user()->related.TimeEntryAutocompleteItems(&time_entry_autocompletes);
                }
                link_vector(time_entry_autocompletes);
                UI()->DisplayTimeEntryAutocomplete(&time_entry_autocompletes);
            } else {
                //Poco::Util::TimerTask::Ptr teTask = new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onTimeEntryAutocompletes);
                //timer_.schedule(teTask, Poco::Timestamp());
            }
        }

        if (what.display_mini_timer_autocomplete) {
            if (what.first_load) {
                if (user()) {
                    user()->related.MinitimerAutocompleteItems(&minitimer_autocompletes);
                }
                link_vector(minitimer_autocompletes);
                UI()->DisplayMinitimerAutocomplete(&minitimer_autocompletes);
            } else {
                //Poco::Util::TimerTask::Ptr mtTask = new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onMiniTimerAutocompletes);
                //timer_.schedule(mtTask, Poco::Timestamp());
            }
        }

        if (what.display_workspace_select)
            renderWorkspaceSelect();

        if (what.display_client_select)
            renderClientSelect();

        if (what.display_timer_state)
            renderTimerState();

        if (what.display_autotracker_rules)
            renderAutotrackerRules();
/*
        if (what.display_settings)
            renderSettings(user, what, Settings::instance());
            */

        // Apply autocomplete as last element,
        // as its depending on selects on Windows
        if (what.display_project_autocomplete) {
            if (what.first_load) {
                if (user()) {
                    user()->related.ProjectAutocompleteItems(&project_autocompletes);
                }
                link_vector(project_autocompletes);
                UI()->DisplayProjectAutocomplete(&project_autocompletes);
            } else {
                //Poco::Util::TimerTask::Ptr prTask = new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onProjectAutocompletes);
                //timer_.schedule(prTask, Poco::Timestamp());
            }
        }

        if (what.display_unsynced_items)
            renderUnsyncedItems();
    }
}

void GUIUpdate::renderTimeEntryEditor(const UIElements &what) {
    view::TimeEntry editor_time_entry_view;
    std::vector<view::Generic> tag_views;

    TimeEntry *editor_time_entry =
        user()->related.TimeEntryByGUID(what.time_entry_editor_guid);
    if (editor_time_entry) {
        if (what.open_time_entry_editor) {
            time_entry_editor_guid_ = editor_time_entry->GUID();
        }

        editor_time_entry_view.Fill(editor_time_entry);
        if (editor_time_entry->IsTracking()) {
            editor_time_entry_view.Duration =
                toggl::Formatter::FormatDuration(
                    editor_time_entry->DurationInSeconds(),
                    Format::Classic);
        } else {
            editor_time_entry_view.Duration =
                toggl::Formatter::FormatDuration(
                    editor_time_entry->DurationInSeconds(),
                    Formatter::DurationFormat);
        }
        editor_time_entry_view.DateDuration =
            Formatter::FormatDurationForDateHeader(
                user()->related.TotalDurationForDate(
                    editor_time_entry));
        user()->related.ProjectLabelAndColorCode(
            editor_time_entry,
            &editor_time_entry_view);

        // Various fields in TE editor related to workspace
        // and user permissions
        Workspace *ws = nullptr;
        if (editor_time_entry->WID()) {
            ws = user()->related.WorkspaceByID(editor_time_entry->WID());
        }
        if (ws) {
            editor_time_entry_view.CanAddProjects =
                ws->Admin() || !ws->OnlyAdminsMayCreateProjects();
        } else {
            editor_time_entry_view.CanAddProjects =
                user()->CanAddProjects();
        }
        editor_time_entry_view.CanSeeBillable =
            user()->CanSeeBillable(ws);
        editor_time_entry_view.DefaultWID = user()->DefaultWID();

        editor_time_entry_view.Locked = isTimeEntryLocked(
            editor_time_entry);

        // Display tags also when time entry is being edited,
        // because tags are filtered by TE WID
        std::vector<std::string> tags;
        user()->related.TagList(&tags, editor_time_entry->WID());
        for (std::vector<std::string>::const_iterator
                it = tags.begin();
                it != tags.end();
                ++it) {
            view::Generic view;
            view.Name = *it;
            tag_views.push_back(view);
        }
    }

    if (!editor_time_entry_view.GUID.empty()) {
        link_vector(tag_views);
        UI()->DisplayTags(tag_views);
        UI()->DisplayTimeEntryEditor(
            what.open_time_entry_editor,
            editor_time_entry_view,
            what.time_entry_editor_field);
    }
}

void GUIUpdate::renderWorkspaceSelect() {
    std::vector<view::Generic> workspace_views;

    std::vector<Workspace *> workspaces;
    user()->related.WorkspaceList(&workspaces);
    for (std::vector<Workspace *>::const_iterator
            it = workspaces.begin();
            it != workspaces.end();
            ++it) {
        Workspace *ws = *it;
        view::Generic view;
        view.GUID = ws->GUID();
        view.ID = ws->ID();
        view.WID = ws->ID();
        view.Name = ws->Name();
        view.WorkspaceName = ws->Name();
        view.Premium = ws->Premium();
        workspace_views.push_back(view);
    }

    link_vector(workspace_views);
    UI()->DisplayWorkspaceSelect(workspace_views);
}

void GUIUpdate::renderClientSelect() {
    std::vector<view::Generic> client_views;

    std::vector<Client *> models;
    user()->related.ClientList(&models);
    for (std::vector<Client *>::const_iterator it = models.begin();
            it != models.end();
            ++it) {
        Client *c = *it;
        view::Generic view;
        view.GUID = c->GUID();
        view.ID = c->ID();
        view.WID = c->WID();
        view.Name = c->Name();
        if (c->WID()) {
            Workspace *ws = user()->related.WorkspaceByID(c->WID());
            if (ws) {
                view.WorkspaceName = ws->Name();
                view.Premium = ws->Premium();
            }
        }
        client_views.push_back(view);
    }

    link_vector(client_views);
    UI()->DisplayClientSelect(client_views);
}

void GUIUpdate::renderTimerState() {
    view::TimeEntry running_entry_view;

    TimeEntry *running_entry = user()->RunningTimeEntry();
    if (running_entry) {
        running_entry_view.Fill(running_entry);
        running_entry_view.Duration =
            toggl::Formatter::FormatDuration(
                running_entry->DurationInSeconds(),
                Format::Classic);
        running_entry_view.DateDuration =
            Formatter::FormatDurationForDateHeader(
                user()->related.TotalDurationForDate(
                    running_entry));
        user()->related.ProjectLabelAndColorCode(
            running_entry,
            &running_entry_view);
    }

    if (!running_entry_view.GUID.empty() && user()) {
        UI()->DisplayTimerState(running_entry_view);
    } else {
        UI()->DisplayEmptyTimerState();
    }
}

void GUIUpdate::renderTimeEntries(const UIElements &what) {
    std::vector<view::TimeEntry> time_entry_views;

    if (what.open_time_entry_list) {
        time_entry_editor_guid_ = "";
    }

    // Get a sorted list of time entries
    std::vector<TimeEntry *> time_entries =
        user()->related.VisibleTimeEntries();
    std::sort(time_entries.begin(), time_entries.end(),
              CompareByStart);

    // Collect the time entries into a list
    std::map<std::string, Poco::Int64> date_durations;

    // Group data maps
    std::map<std::string, Poco::Int64> group_durations;
    std::map<std::string, Poco::UInt64> group_header_id;
    std::map<std::string, std::vector<Poco::UInt64> > group_items;

    for (unsigned int i = 0; i < time_entries.size(); i++) {
        TimeEntry *te = time_entries[i];

        std::string date_header =
            toggl::Formatter::FormatDateHeader(te->Start());

        // Calculate total duration for each date:
        // will be displayed in date header
        Poco::Int64 duration = date_durations[date_header];
        duration += Formatter::AbsDuration(te->Duration());
        date_durations[date_header] = duration;

        // Dont render running entry in list,
        // although its calculated into totals per date.
        if (te->Duration() < 0) {
            // Don't display running entries
            continue;
        }

        // Calculate total duration of group
        if (user()->CollapseEntries()) {
            std::stringstream ss;
            ss << date_header << te->Description()
               << te->PID() << te->TID()
               << te->ProjectGUID()
               << te->Billable() << te->Tags();
            std::string group_name = ss.str();

            group_header_id[group_name] = i;
            duration = group_durations[group_name];
            duration += Formatter::AbsDuration(te->Duration());
            group_durations[group_name] = duration;
            group_items[group_name].push_back(i);
        }
    }

    // Assign the date durations we calculated previously
    for (unsigned int i = 0; i < time_entries.size(); i++) {
        TimeEntry *te = time_entries[i];

        // Dont render running entry in list,
        // although its calculated into totals per date.
        if (te->Duration() < 0) {
            // Don't display running entries
            continue;
        }

        view::TimeEntry view;
        view.Fill(te);

        // Assign group info
        if (user()->CollapseEntries()) {
            if (group_items[view.GroupName].size() > 1) {
                if (group_header_id[view.GroupName] == i) {
                    // If Group open add all entries in group
                    if (entry_groups[view.GroupName]) {
                        for (unsigned int j = 0; j < group_items[view.GroupName].size(); j++) {
                            TimeEntry *group_entry =
                                time_entries[group_items[view.GroupName][j]];

                            view::TimeEntry group_entry_view;
                            group_entry_view.Fill(group_entry);

                            group_entry_view.GroupOpen = entry_groups[view.GroupName];

                            user()->related.ProjectLabelAndColorCode(
                                group_entry,
                                &group_entry_view);

                            group_entry_view.Locked = isTimeEntryLocked(group_entry);

                            group_entry_view.Duration = toggl::Formatter::FormatDuration(
                                group_entry_view.DurationInSeconds,
                                Formatter::DurationFormat);
                            group_entry_view.DateDuration =
                                Formatter::FormatDurationForDateHeader(
                                    date_durations[group_entry_view.DateHeader]);
                            group_entry_view.GroupItemCount = entry_groups[group_entry_view.GroupName];
                            time_entry_views.push_back(group_entry_view);
                        }
                    }

                    // Add Group header
                    view::TimeEntry group_view;
                    group_view.Fill(te);
                    user()->related.ProjectLabelAndColorCode(
                        te,
                        &group_view);
                    group_view.Group = true;
                    group_view.GroupOpen = entry_groups[group_view.GroupName];
                    group_view.Duration =
                        Formatter::FormatDuration(
                            group_durations[view.GroupName],
                            Formatter::DurationFormat);
                    group_view.DateDuration =
                        Formatter::FormatDurationForDateHeader(
                            date_durations[view.DateHeader]);
                    group_view.GroupItemCount = group_items[group_view.GroupName].size();
                    time_entry_views.push_back(group_view);
                }
                continue;
            }
            view.GroupItemCount = 1;
        }
        user()->related.ProjectLabelAndColorCode(
            te,
            &view);
    }
    // RENDERING PART //
    link_vector(time_entry_views);
    UI()->DisplayTimeEntryList(
        what.open_time_entry_list,
        time_entry_views,
        !user()->HasLoadedMore());
    last_time_entry_list_render_at_ = Poco::LocalDateTime();
}

void GUIUpdate::renderSettings(Settings *settings) {
    view::Settings settings_view;

    Proxy proxy;
    bool use_proxy(false);
    bool record_timeline(false);
/* TODO
    error err = db()->LoadSettings(&settings);
    if (err != noError) {
        setUser(nullptr);
        displayError(err);
        return;
    }
    err = db()->LoadProxySettings(&use_proxy, &proxy);
    if (err != noError) {
        setUser(nullptr);
        displayError(err);
        return;
    }
    if (user) {
        record_timeline = user()->RecordTimeline();
    }
    idle_.SetSettings(settings);
    */

    // TODO this is crazy, let's do something about it in the future
    settings_view.UseIdleDetection = settings->use_idle_detection;
    settings_view.MenubarTimer = settings->menubar_timer;
    settings_view.MenubarProject = settings->menubar_project;
    settings_view.DockIcon = settings->dock_icon;
    settings_view.OnTop = settings->on_top;
    settings_view.Reminder = settings->reminder;
    settings_view.IdleMinutes = settings->idle_minutes;
    settings_view.FocusOnShortcut = settings->focus_on_shortcut;
    settings_view.ReminderMinutes = settings->reminder_minutes;
    settings_view.ManualMode = settings->manual_mode;
    settings_view.RemindMon = settings->remind_mon;
    settings_view.RemindTue = settings->remind_tue;
    settings_view.RemindWed = settings->remind_wed;
    settings_view.RemindThu = settings->remind_thu;
    settings_view.RemindFri = settings->remind_fri;
    settings_view.RemindSat = settings->remind_sat;
    settings_view.RemindSun = settings->remind_sun;
    settings_view.RemindStarts = settings->remind_starts;
    settings_view.RemindEnds = settings->remind_ends;
    settings_view.Autotrack = settings->autotrack;
    settings_view.OpenEditorOnShortcut = settings->open_editor_on_shortcut;
    //has_seen_beta_offering;
    settings_view.Pomodoro = settings->pomodoro;
    settings_view.PomodoroBreak = settings->pomodoro_break;
    settings_view.PomodoroMinutes = settings->pomodoro_minutes;
    settings_view.PomodoroBreakMinutes = settings->pomodoro_break_minutes;
    settings_view.StopEntryOnShutdownSleep = settings->stop_entry_on_shutdown_sleep;

    settings_view.UseProxy = use_proxy;
    settings_view.ProxyHost = proxy.Host();
    settings_view.ProxyPort = proxy.Port();
    settings_view.ProxyPassword = proxy.Password();
    settings_view.ProxyUsername = proxy.Username();
    settings_view.AutodetectProxy = settings->autodetect_proxy;

    settings_view.RecordTimeline = record_timeline;
/* TODO
    UI()->DisplaySettings(what.open_settings,
                          &settings_view);
    // Tracking Settings
    if ("production" == environment_) {
        analytics_.TrackSettings(db_->AnalyticsClientID(),
                                 settings_view.RecordTimeline,
                                 settings,
                                 settings_view.UseProxy,
                                 proxy);
    }
    */
}

void GUIUpdate::renderUnsyncedItems() {
    Poco::Int64 unsynced_item_count = user()->related.NumberOfUnsyncedTimeEntries();
    UI()->DisplayUnsyncedItems(unsynced_item_count);
}

void GUIUpdate::renderAutotrackerRules() {
    std::vector<view::AutotrackerRule> autotracker_rule_views;
    std::vector<std::string> autotracker_title_views;

    if (UI()->CanDisplayAutotrackerRules()) {
        // Collect rules
        for (std::vector<toggl::AutotrackerRule *>::const_iterator
                it = user()->related.AutotrackerRules.begin();
                it != user()->related.AutotrackerRules.end();
                ++it) {
            AutotrackerRule *model = *it;
            Project *p = user()->related.ProjectByID(model->PID());
            Task *t = user()->related.TaskByID(model->TID());

            view::AutotrackerRule rule;
            rule.ProjectName = Formatter::JoinTaskName(t, p);
            rule.ID = model->LocalID();
            rule.Term = model->Term();
            autotracker_rule_views.push_back(rule);
        }

        // Collect titles
        for (std::set<std::string>::const_iterator
                it = autotracker_titles_.begin();
                it != autotracker_titles_.end();
                ++it) {
            autotracker_title_views.push_back(*it);
        }
        std::sort(autotracker_title_views.begin(),
                  autotracker_title_views.end(),
                  CompareAutotrackerTitles);
    }
    if (UI()->CanDisplayAutotrackerRules()) {
        link_vector(autotracker_rule_views);
        UI()->DisplayAutotrackerRules(
            autotracker_rule_views,
            autotracker_title_views);
    }
}

error GUIUpdate::ToggleEntriesGroup(std::string name) {
    entry_groups[name] = !entry_groups[name];
    OpenTimeEntryList();
    return noError;
}

void GUIUpdate::OpenTimeEntryList() {
    logger().debug("OpenTimeEntryList");

    UIElements render;
    render.open_time_entry_list = true;
    render.display_time_entries = true;
    update(render);
}

void GUIUpdate::ResetAutotrackerView() {
    // Reset autotracker view
    // Autotracker rules has a project autocomplete, too
    autotracker_titles_.clear();
    UIElements render;
    render.display_autotracker_rules = true;
    render.display_project_autocomplete = true;
    update(render);
}

Poco::LocalDateTime GUIUpdate::LastTimeEntryRenderTime() {
    return last_time_entry_list_render_at_;
}

bool GUIUpdate::isTimeEntryLocked(TimeEntry* te) {
    return isTimeLockedInWorkspace(te->Start(), user()->related.WorkspaceByID(te->WID()));
}

bool GUIUpdate::canChangeStartTimeTo(TimeEntry* te, time_t t) {
    return !isTimeLockedInWorkspace(t, user()->related.WorkspaceByID(te->WID()));
}

bool GUIUpdate::canChangeProjectTo(TimeEntry* te, Project* p) {
    return !isTimeLockedInWorkspace(te->Start(), user()->related.WorkspaceByID(p->WID()));
}

bool GUIUpdate::isTimeLockedInWorkspace(time_t t, Workspace* ws) {
    if (!ws)
        return false;
    if (!ws->Business())
        return false;
    if (ws->Admin())
        return false;
    auto lockedTime = ws->LockedTime();
    if (lockedTime == 0)
        return false;
    return t < lockedTime;
}


} // namespace toggl
