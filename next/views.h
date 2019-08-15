
#include <string>
#include <cstdint>
#include <sstream>

namespace toggl {

namespace view {

class TimeEntry {
 public:
    TimeEntry()
        : DurationInSeconds(0)
    , Description("")
    , ProjectAndTaskLabel("")
    , TaskLabel("")
    , ProjectLabel("")
    , ClientLabel("")
    , WID(0)
    , PID(0)
    , TID(0)
    , Duration("")
    , Color("")
    , GUID("")
    , Billable(false)
    , Tags("")
    , Started(0)
    , Ended(0)
    , StartTimeString("")
    , EndTimeString("")
    , UpdatedAt(0)
    , DurOnly(false)
    , DateHeader("")
    , DateDuration("")
    , CanAddProjects(false)
    , CanSeeBillable(false)
    , DefaultWID(0)
    , WorkspaceName("")
    , Unsynced(false)
    , Error(noError)
    , Locked(false)
    , Group(false)
    , GroupOpen(false)
    , GroupName("")
    , GroupDuration("")
    , GroupItemCount(0) {}

    int64_t DurationInSeconds;
    std::string Description;
    std::string ProjectAndTaskLabel;
    std::string TaskLabel;
    std::string ProjectLabel;
    std::string ClientLabel;
    uint64_t WID;
    uint64_t PID;
    uint64_t TID;
    std::string Duration;
    std::string Color;
    std::string GUID;
    bool Billable;
    std::string Tags;
    int64_t Started;
    int64_t Ended;
    std::string StartTimeString;
    std::string EndTimeString;
    int64_t UpdatedAt;
    bool DurOnly;
    // In case it's a header
    std::string DateHeader;
    std::string DateDuration;
    // Additional fields; only when in time entry editor
    bool CanAddProjects;
    bool CanSeeBillable;
    uint64_t DefaultWID;
    std::string WorkspaceName;
    bool Unsynced;
    // If syncing a time entry ended with an error,
    // the error is attached to the time entry
    std::string Error;
    bool Locked;
    bool Group;
    bool GroupOpen;
    // date + description + pid
    std::string GroupName;
    std::string GroupDuration;
    uint64_t GroupItemCount;

    void Fill(toggl::TimeEntry * const model);

    bool operator == (const TimeEntry& other) const;
};

class Autocomplete {
 public:
    Autocomplete()
        : Text("")
    , Description("")
    , ProjectAndTaskLabel("")
    , TaskLabel("")
    , ProjectLabel("")
    , ClientLabel("")
    , ProjectColor("")
    , TaskID(0)
    , ProjectID(0)
    , WorkspaceID(0)
    , Type(0)
    , Billable(false)
    , Tags("")
    , WorkspaceName("")
    , ClientID(0)
    , ProjectGUID("") {}

    bool IsTimeEntry() const {
        return kAutocompleteItemTE == Type;
    }
    bool IsTask() const {
        return kAutocompleteItemTask == Type;
    }
    bool IsProject() const {
        return kAutocompleteItemProject == Type;
    }
    bool IsWorkspace() const {
        return kAutocompleteItemWorkspace == Type;
    }

    std::string String() const {
        std::stringstream ss;
        ss << "AutocompleteItem"
           << " Text=" << Text
           << " Description=" << Description
           << " ProjectAndTaskLabel=" << ProjectAndTaskLabel
           << " TaskLabel=" << TaskLabel
           << " ProjectLabel=" << ProjectLabel
           << " ClientLabel=" << ClientLabel
           << " ProjectColor=" << ProjectColor
           << " TaskID=" << TaskID
           << " ProjectID=" << ProjectID
           << " WorkspaceID=" << WorkspaceID
           << " Type=" << Type
           << " Billable=" << Billable
           << " WorkspaceName=" << WorkspaceName
           << " Tags=" << Tags;
        return ss.str();
    }

    // This is what is displayed to user, includes project and task.
    std::string Text;
    // This is copied to "time_entry.description" field if item is selected
    std::string Description;
    // Project label, if has a project
    std::string ProjectAndTaskLabel;
    std::string TaskLabel;
    std::string ProjectLabel;
    std::string ClientLabel;
    std::string ProjectColor;
    uint64_t TaskID;
    uint64_t ProjectID;
    uint64_t WorkspaceID;
    uint64_t Type;
    // If its a time entry or project, it can be billable
    bool Billable;
    // If its a time entry, it has tags
    std::string Tags;
    std::string WorkspaceName;
    uint64_t ClientID;
    std::string ProjectGUID;

    bool operator == (const Autocomplete& other) const;
};

class Generic {
 public:
    Generic()
        : ID(0)
    , WID(0)
    , GUID("")
    , Name("")
    , WorkspaceName("")
    , Premium(false) {}

    uint64_t ID;
    uint64_t WID;
    std::string GUID;
    std::string Name;
    std::string WorkspaceName;
    bool Premium;

    bool operator == (const Generic& other) const;
};

class Settings {
 public:
    Settings()
        : UseProxy(false)
    , ProxyHost("")
    , ProxyPort(0)
    , ProxyUsername("")
    , ProxyPassword("")
    , UseIdleDetection(false)
    , MenubarTimer(false)
    , MenubarProject(false)
    , DockIcon(false)
    , OnTop(false)
    , Reminder(false)
    , RecordTimeline(false)
    , IdleMinutes(0)
    , FocusOnShortcut(false)
    , ReminderMinutes(0)
    , ManualMode(false)
    , AutodetectProxy(false)
    , RemindMon(false)
    , RemindTue(false)
    , RemindWed(false)
    , RemindThu(false)
    , RemindFri(false)
    , RemindSat(false)
    , RemindSun(false)
    , RemindStarts("")
    , RemindEnds("")
    , Autotrack(false)
    , OpenEditorOnShortcut(false)
    , Pomodoro(false)
    , PomodoroBreak(false)
    , PomodoroMinutes(0)
    , PomodoroBreakMinutes(0)
    , StopEntryOnShutdownSleep(false) {}

    bool UseProxy;
    std::string ProxyHost;
    uint64_t ProxyPort;
    std::string ProxyUsername;
    std::string ProxyPassword;
    bool UseIdleDetection;
    bool MenubarTimer;
    bool MenubarProject;
    bool DockIcon;
    bool OnTop;
    bool Reminder;
    bool RecordTimeline;
    uint64_t IdleMinutes;
    bool FocusOnShortcut;
    uint64_t ReminderMinutes;
    bool ManualMode;
    bool AutodetectProxy;
    bool RemindMon;
    bool RemindTue;
    bool RemindWed;
    bool RemindThu;
    bool RemindFri;
    bool RemindSat;
    bool RemindSun;
    std::string RemindStarts;
    std::string RemindEnds;
    bool Autotrack;
    bool OpenEditorOnShortcut;
    bool Pomodoro;
    bool PomodoroBreak;
    uint64_t PomodoroMinutes;
    uint64_t PomodoroBreakMinutes;
    bool StopEntryOnShutdownSleep;

    bool operator == (const Settings& other) const;
};

class AutotrackerRule {
 public:
    AutotrackerRule()
        : ID(0)
    , Term("")
    , ProjectName("") {}

    int64_t ID;
    std::string Term;
    std::string ProjectName;

    bool operator == (const AutotrackerRule& other) const;
};

class TimelineEvent {
 public:
    TimelineEvent()
        : ID(0)
    , Title("")
    , Filename("")
    , StartTime(0)
    , EndTime(0)
    , Idle(false) {}

    int64_t ID;
    std::string Title;
    std::string Filename;
    uint64_t StartTime;
    uint64_t EndTime;
    bool Idle;

    bool operator == (const TimelineEvent& other) const;
};

class Country {
 public:
    Country()
        : ID(0)
    , Name("")
    , VatApplicable(false)
    , VatRegex("")
    , VatPercentage("")
    , Code("") {}

    uint64_t ID;
    std::string Name;
    bool VatApplicable;
    std::string VatRegex;
    std::string VatPercentage;
    std::string Code;

    bool operator == (const Country& other) const;
};

}  // namespace view

} // namespace toggl
