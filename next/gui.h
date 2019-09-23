// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_GUI_H_
#define SRC_GUI_H_

#include <set>
#include <string>
#include <vector>

#include "https_client.h"
#include "toggl_api.h"
#include "user_data.h"

namespace Poco {
class Logger;
}

namespace toggl {

class Context;
class Client;
class RelatedData;
class User;
class TimeEntry;
class Workspace;

class GUI : public SyncStateMonitor {
 public:
    GUI(Context *context, TogglCallbacks callbacks)
    : context_(context)
    , callbacks_(callbacks)
    , lastSyncState(-1)
    , lastUnsyncedItemsCount(-1)
    , lastDisplayLoginOpen(false)
    , lastDisplayLoginUserID(0)
    , lastOnlineState(-1)
    , lastErr(noError)
    , isFirstLaunch(true) {}

    ~GUI() {}

    void SetCallbacks(TogglCallbacks callbacks);

    void DisplayApp();

    error DisplayError(const error);

    // Overlay screen triggers
    error DisplayWSError();
    error DisplayTosAccept();

    void DisplayHelpArticles(locked<const std::vector<HelpArticle*>> &articles);

    void DisplaySyncState(const Poco::Int64 state);

    void DisplayOnlineState(const Poco::Int64 state);

    void DisplayUnsyncedItems(const Poco::Int64 count);

    void DisplayReminder();

    void DisplayPomodoro(const Poco::Int64 minutes);

    void DisplayPomodoroBreak(const Poco::Int64 minutes);

    void DisplayAutotrackerNotification(locked<const Project> &p,
        locked<const Task> &t);

    void DisplayMinitimerAutocomplete(std::vector<toggl::view::Autocomplete> *);

    void DisplayTimeEntryAutocomplete(std::vector<toggl::view::Autocomplete> *);

    void DisplayProjectAutocomplete(std::vector<toggl::view::Autocomplete> *);

    void DisplayTimeEntryList(
        const bool open,
        const std::vector<view::TimeEntry> list,
        const bool show_load_more_button);

    void DisplayProjectColors();

    void DisplayCountries(
        std::vector<TogglCountryView> *items);

    void DisplayWorkspaceSelect(
        const std::vector<view::Generic> list);

    void DisplayClientSelect(
        const std::vector<view::Generic> list);

    void DisplayTags(
        const std::vector<view::Generic> list);

    void DisplayAutotrackerRules(
        const std::vector<view::AutotrackerRule> &autotracker_rules,
        const std::vector<std::string> &titles);

    void DisplayTimeEntryEditor(
        const bool open,
        const view::TimeEntry te,
        const std::string focused_field_name);

    void DisplayURL(const std::string);

    void DisplayLogin(const bool open, const uint64_t user_id);

    void DisplaySettings(const bool open,
        const bool record_timeline,
        const toggl::Settings &settings,
        const bool use_proxy,
        const Proxy proxy);

    void DisplayTimerState(
        const view::TimeEntry &te);

    void DisplayEmptyTimerState();

    void DisplayIdleNotification(const std::string guid,
                                 const std::string since,
                                 const std::string duration,
                                 const int64_t started,
                                 const std::string description);

    void DisplayUpdate(const std::string URL);

    void DisplayUpdateDownloadState(
        const std::string version,
        const Poco::Int64 download_state);

    error VerifyCallbacks();

    void DisplayPromotion(const int64_t promotion_type);

    void DisplayObmExperiment(
        const uint64_t nr,
        const bool included,
        const bool seen);

    void resetFirstLaunch();

 private:
    error findMissingCallbacks();

    Context *context_;
    TogglCallbacks callbacks_;

    // Cached views
    Poco::Int64 lastSyncState;
    Poco::Int64 lastUnsyncedItemsCount;
    bool lastDisplayLoginOpen;
    uint64_t lastDisplayLoginUserID;
    Poco::Int64 lastOnlineState;
    error lastErr;
    bool isFirstLaunch;
    Poco::Logger &logger() const;
};

}  // namespace toggl

#endif  // SRC_GUI_H_
