#ifndef VIEWS_IMPLEMENTATION

# define VIEW(_parent, _view) typedef void* _view;
# define VIEW_GET(_parent, _view, _type, _property) \
     _type _view ## _ ## _property(const _view *v);
# define VIEW_DEDUCE(_parent, _view, _type, _property, ...) \
     _type _view ## _ ## _property(const _view *v);

#else // VIEWS_IMPLEMENTATION

# undef VIEW
# undef VIEW_GET
# undef VIEW_DEDUCE

# define VIEW(_parent, _view) \
    template<> _view const* convert(toggl::_parent* const& p) { return reinterpret_cast<_view const*>(p); } \
    //template <> const _view* convert(const toggl::_parent *&p) { return nullptr; }  \
    //typedef toggl::_parent _view;
# define VIEW_GET(_parent, _view, _type, _property) \
 _type _view ## _ ## _property(const _view *v) { \
     return convert<_type>(reinterpret_cast<const toggl::_parent*>(v)->_property); \
 }
# define VIEW_DEDUCE(_parent, _view, _type, _property, ...) \
 _type _view ## _ ## _property(const _view *v) { \
     (void) v; \
     __VA_ARGS__ \
 }

#endif // VIEWS_IMPLEMENTATION

VIEW(view::TimeEntry, TogglTimeEntryView)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, int64_t, DurationInSeconds)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, Description)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, ProjectAndTaskLabel)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, TaskLabel)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, ProjectLabel)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, ClientLabel)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, uint64_t, WID)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, uint64_t, PID)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, uint64_t, TID)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, Duration)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, Color)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, GUID)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, bool_t, Billable)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, Tags)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, int64_t, Started)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, int64_t, Ended)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, StartTimeString)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, EndTimeString)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, int64_t, UpdatedAt)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, bool_t, DurOnly)
// In case it's a header
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, DateHeader)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, DateDuration)
// TODO
VIEW_DEDUCE(TimeEntry, TogglTimeEntryView, bool_t, IsHeader,
    return false;
)
// Additional fields only when in time entry editor
VIEW_GET(view::TimeEntry, TogglTimeEntryView, bool_t, CanAddProjects)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, bool_t, CanSeeBillable)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, uint64_t, DefaultWID)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, WorkspaceName)
// If syncing a time entry ended with an error
// the error is attached to the time entry
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, Error)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, bool_t, Locked)
// Indicates if time entry is not synced to server
VIEW_GET(view::TimeEntry, TogglTimeEntryView, bool_t, Unsynced)
// Group attributes
VIEW_GET(view::TimeEntry, TogglTimeEntryView, bool_t, Group)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, bool_t, GroupOpen)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, GroupName)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const char_t*, GroupDuration)
VIEW_GET(view::TimeEntry, TogglTimeEntryView, uint64_t, GroupItemCount)
// Next in list
VIEW_GET(view::TimeEntry, TogglTimeEntryView, const TogglTimeEntryView *, Next)

VIEW(view::Autocomplete, TogglAutocompleteView)
// This is what is displayed to user, includes project and task.
VIEW_GET(view::Autocomplete, TogglAutocompleteView, const char_t*, Text)
// This is copied to "time_entry.description" field if item is selected
VIEW_GET(view::Autocomplete, TogglAutocompleteView, const char_t*, Description)
// Project label, if has a project
VIEW_GET(view::Autocomplete, TogglAutocompleteView, const char_t*, ProjectAndTaskLabel)
VIEW_GET(view::Autocomplete, TogglAutocompleteView, const char_t*, TaskLabel)
VIEW_GET(view::Autocomplete, TogglAutocompleteView, const char_t*, ProjectLabel)
VIEW_GET(view::Autocomplete, TogglAutocompleteView, const char_t*, ClientLabel)
VIEW_GET(view::Autocomplete, TogglAutocompleteView, const char_t*, ProjectColor)
VIEW_GET(view::Autocomplete, TogglAutocompleteView, const char_t*, ProjectGUID)
VIEW_GET(view::Autocomplete, TogglAutocompleteView, uint64_t, TaskID)
VIEW_GET(view::Autocomplete, TogglAutocompleteView, uint64_t, ProjectID)
VIEW_GET(view::Autocomplete, TogglAutocompleteView, uint64_t, WorkspaceID)
VIEW_GET(view::Autocomplete, TogglAutocompleteView, uint64_t, Type)
// If its a time entry or project, it can be billable
VIEW_GET(view::Autocomplete, TogglAutocompleteView, bool_t, Billable)
// If its a time entry, it has tags
VIEW_GET(view::Autocomplete, TogglAutocompleteView, const char_t*, Tags)
VIEW_GET(view::Autocomplete, TogglAutocompleteView, const char_t*, WorkspaceName)
VIEW_GET(view::Autocomplete, TogglAutocompleteView, uint64_t, ClientID)
VIEW_GET(view::Autocomplete, TogglAutocompleteView, const TogglAutocompleteView*, Next)

VIEW(view::Generic, TogglGenericView)
VIEW_GET(view::Generic, TogglGenericView, uint64_t, ID)
VIEW_GET(view::Generic, TogglGenericView, uint64_t, WID)
VIEW_GET(view::Generic, TogglGenericView, const char_t*, GUID)
VIEW_GET(view::Generic, TogglGenericView, const char_t*, Name)
VIEW_GET(view::Generic, TogglGenericView, const char_t*, WorkspaceName)
VIEW_GET(view::Generic, TogglGenericView, bool_t, Premium)
VIEW_GET(view::Generic, TogglGenericView, const TogglGenericView*, Next)

VIEW(HelpArticle, TogglHelpArticleView)
//VIEW_GET(HelpArticle, TogglHelpArticleView, const char_t*, Category)
VIEW_GET(HelpArticle, TogglHelpArticleView, const char_t*, Name)
VIEW_GET(HelpArticle, TogglHelpArticleView, const char_t*, URL)
//VIEW_GET(view::HelpArticle, TogglHelpArticleView, TogglHelpArticleView*, Next)

VIEW(view::Settings, TogglSettingsView)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, UseProxy)
VIEW_GET(view::Settings, TogglSettingsView, const char_t*, ProxyHost)
VIEW_GET(view::Settings, TogglSettingsView, uint64_t, ProxyPort)
VIEW_GET(view::Settings, TogglSettingsView, const char_t*, ProxyUsername)
VIEW_GET(view::Settings, TogglSettingsView, const char_t*, ProxyPassword)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, UseIdleDetection)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, MenubarTimer)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, MenubarProject)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, DockIcon)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, OnTop)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, Reminder)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, RecordTimeline)
VIEW_GET(view::Settings, TogglSettingsView, int64_t, IdleMinutes)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, FocusOnShortcut)
VIEW_GET(view::Settings, TogglSettingsView, int64_t, ReminderMinutes)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, ManualMode)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, AutodetectProxy)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, RemindMon)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, RemindTue)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, RemindWed)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, RemindThu)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, RemindFri)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, RemindSat)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, RemindSun)
VIEW_GET(view::Settings, TogglSettingsView, const char_t*, RemindStarts)
VIEW_GET(view::Settings, TogglSettingsView, const char_t*, RemindEnds)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, Autotrack)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, OpenEditorOnShortcut)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, Pomodoro)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, PomodoroBreak)
VIEW_GET(view::Settings, TogglSettingsView, int64_t, PomodoroMinutes)
VIEW_GET(view::Settings, TogglSettingsView, int64_t, PomodoroBreakMinutes)
VIEW_GET(view::Settings, TogglSettingsView, bool_t, StopEntryOnShutdownSleep)

VIEW(view::AutotrackerRule, TogglAutotrackerRuleView)
VIEW_GET(view::AutotrackerRule, TogglAutotrackerRuleView, uint64_t, ID)
VIEW_GET(view::AutotrackerRule, TogglAutotrackerRuleView, const char_t*, Term)
//VIEW_GET(view::AutotrackerRule, TogglAutotrackerRuleView, const char_t*, ProjectAndTaskLabel)
VIEW_GET(view::AutotrackerRule, TogglAutotrackerRuleView, const TogglAutotrackerRuleView*, Next)

VIEW(view::TimelineEvent, TogglTimelineEventView)
VIEW_GET(view::TimelineEvent, TogglTimelineEventView, uint64_t, ID)
VIEW_GET(view::TimelineEvent, TogglTimelineEventView, const char_t*, Title)
VIEW_GET(view::TimelineEvent, TogglTimelineEventView, const char_t*, Filename)
VIEW_GET(view::TimelineEvent, TogglTimelineEventView, int64_t, StartTime)
VIEW_GET(view::TimelineEvent, TogglTimelineEventView, int64_t, EndTime)
VIEW_GET(view::TimelineEvent, TogglTimelineEventView, bool_t, Idle)
VIEW_GET(view::TimelineEvent, TogglTimelineEventView, const TogglTimelineEventView*, Next)

VIEW(view::Country, TogglCountryView)
VIEW_GET(view::Country, TogglCountryView, uint64_t, ID)
VIEW_GET(view::Country, TogglCountryView, const char_t*, Name)
VIEW_GET(view::Country, TogglCountryView, bool_t, VatApplicable)
VIEW_GET(view::Country, TogglCountryView, const char_t*, VatRegex)
VIEW_GET(view::Country, TogglCountryView, const char_t*, VatPercentage)
VIEW_GET(view::Country, TogglCountryView, const char_t*, Code)
VIEW_GET(view::Country, TogglCountryView, const TogglCountryView*, Next)
