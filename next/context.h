// Copyright 2019 Toggl Desktop developers.

#include "toggl_api.h"

#include "util.h"

#include <string>

namespace toggl {

class UserData;

class Context {
public:
    Context(const std::string &appName, const std::string &appVersion, bool production, bool checkForUpdates);
    ~Context();

    void registerCallbacks(TogglCallbacks callbacks);
    void getCountries();

    void debug(const std::string &text);

    void setCacertPath(const std::string &path);
    bool setDbPath(const std::string &path);
    void setUpdatePath(const std::string &path);
    const std::string &updatePath();
    void setLogPath(const std::string &path);
    void setLogLevel(const std::string &level);
    bool setUpdateChannel(const std::string &updateChannel);
    const std::string &getUpdateChannel();

    void sync();
    void fullsync();
    bool clearCache();

    void setSleep();
    void setWake();
    void setLocked();
    void setUnlocked();
    void osShutdown();
    void setOnline();
    void setIdleSeconds(uint64_t idleSeconds);

    bool uiStart();
    bool login(const std::string &email, const std::string &password);
    bool signup(const std::string &email, const std::string &password, uint64_t countryId);
    bool googleLogin(const std::string &accessToken);
    void passwordForgot();
    bool logout();
    const std::string &getUserFullname();
    const std::string &getUserEmail();

    void tos();
    bool acceptTos();
    void privacyPolicy();
    void openInBrowser();

    void showApp();
    void editPreferences();

    void getSupport(int32_t type);
    bool feedbackSend(const std::string &topic, const std::string &details, const std::string &filename);
    void searchHelpArticles(const std::string &keywords);

    void viewTimeEntryList();
    void loadMore();
    const std::string &start(const std::string &description, const std::string &duration, uint64_t taskId, uint64_t projectId, const std::string &projectGuid, const std::string &tags, bool preventOnApp);
    bool stop(bool preventOnApp);
    bool continueTimeEntry(const std::string &guid);
    bool continueLatestTimeEntry(bool preventOnApp);
    void edit(const std::string &guid, bool editRunningTimeEntry, const std::string &focusedFieldName);
    bool deleteTimeEntry(const std::string &guid);
    bool setTimeEntryDuration(const std::string &guid, const std::string &value);
    bool setTimeEntryProject(const std::string &guid, uint64_t taskId, uint64_t projectId, const std::string &projectGuid);
    bool setTimeEntryDate(const std::string &guid, int64_t unixTimestamp);
    bool setTimeEntryStart(const std::string &guid, const std::string &value);
    bool setTimeEntryEnd(const std::string &guid, const std::string &value);
    bool setTimeEntryTags(const std::string &guid, const std::string &value);
    bool setTimeEntryBillable(const std::string &guid, bool value);
    bool setTimeEntryDescription(const std::string &guid, const std::string &value);
    bool discardTimeAt(const std::string &guid, int64_t at, bool splitIntoNewEntry);
    bool discardTimeAndContinue(const std::string &guid, int64_t at);

    const std::string &createClient(uint64_t workspaceId, const std::string &clientName);

    const std::string &addProject(const std::string &timeEntryGuid, uint64_t workspaceId, uint64_t clientId, const std::string &clientGuid, const std::string &projectName, bool isPrivate, const std::string &projectColor);
    bool setDefaultProject(uint64_t pid, uint64_t tid);
    uint64_t getDefaultProjectId();
    const std::string &getDefaultProjectName();
    void getProjectColors();

    uint64_t getDefaultTaskId();

    bool addObmAction(uint64_t experimentId, const std::string &key, const std::string &value);
    void addObmExperimentNr(uint64_t nr);

    bool setPromotionResponse(int64_t promotionType, int64_t promotionResponse);

    bool timelineToggleRecording(bool recordTimeline);
    bool timelineIsRecordingEnabled();
    int64_t autotrackerAddRule(const std::string &term, uint64_t projectId, uint64_t taskId);
    bool autotrackerDeleteRule(int64_t id);

    bool setSettingsRemindDays(bool remindMon, bool remindTue, bool remindWed, bool remindThu, bool remindFri, bool remindSat, bool remindSun);
    bool setSettingsRemindTimes(const std::string &remindStarts, const std::string &remindEnds);
    bool setSettingsUseIdleDetection(bool useIdleDetection);
    bool setSettingsAutotrack(bool value);
    bool setSettingsOpenEditorOnShortcut(bool value);
    bool setSettingsAutodetectProxy(bool autodetectProxy);
    bool setSettingsMenubarTimer(bool menubarTimer);
    bool setSettingsMenubarProject(bool menubarProject);
    bool setSettingsDockIcon(bool dockIcon);
    bool setSettingsOnTop(bool onTop);
    bool setSettingsReminder(bool reminder);
    bool setSettingsPomodoro(bool pomodoro);
    bool setSettingsPomodoroBreak(bool pomodoroBreak);
    bool setSettingsStopEntryOnShutdownSleep(bool stopEntry);
    bool setSettingsIdleMinutes(uint64_t idleMinutes);
    bool setSettingsFocusOnShortcut(bool focusOnShortcut);
    bool setSettingsReminderMinutes(uint64_t reminderMinutes);
    bool setSettingsPomodoroMinutes(uint64_t pomodoroMinutes);
    bool setSettingsPomodoroBreakMinutes(uint64_t pomodoroBreakMinutes);
    bool setSettingsManualMode(bool manualMode);
    bool setProxySettings(bool useProxy, const std::string &proxyHost, uint64_t proxyPort, const std::string &proxyUsername, const std::string &proxyPassword);
    bool setWindowSettings(int64_t windowX, int64_t windowY, int64_t windowHeight, int64_t windowWidth);
    bool windowSettings(int64_t *windowX, int64_t *windowY, int64_t *windowHeight, int64_t *windowWidth);
    void setWindowMaximized(bool value);
    bool getWindowMaximized();
    void setWindowMinimized(bool value);
    bool getWindowMinimized();
    void setWindowEditSizeHeight(int64_t value);
    int64_t getWindowEditSizeHeight();
    void setWindowEditSizeWidth(int64_t value);
    int64_t getWindowEditSizeWidth();
    void setKeyStart(const std::string &value);
    const std::string &getKeyStart();
    void setKeyShow(const std::string &value);
    const std::string &getKeyShow();
    void setKeyModifierShow(const std::string &value);
    const std::string &getKeyModifierShow();
    void setKeyModifierStart(const std::string &value);
    const std::string &getKeyModifierStart();
    void trackWindowSize(uint64_t width, uint64_t height);
    void trackEditSize(uint64_t width, uint64_t height);
    void setMiniTimerX(int64_t value);
    int64_t getMiniTimerX();
    void setMiniTimerY(int64_t value);
    int64_t getMiniTimerY();
    void setMiniTimerW(int64_t value);
    int64_t getMiniTimerW();

    void setMiniTimerVisible(bool value);
    bool getMiniTimerVisible();
    void setCompactMode(bool value);
    bool getCompactMode();

    void setKeepEndTimeFixed(bool value);
    bool getKeepEndTimeFixed();

    const std::string &formatTrackingTimeDuration(int64_t durationInSeconds);
    const std::string &formatTrackedTimeDuration(int64_t durationInSeconds);
    int64_t parseDurationStringIntoSeconds(const std::string &durationString);

    const std::string &runScript(const std::string &script, int64_t *err);
    void testingSleep(int32_t seconds);
    bool testingSetLoggedInUser(const std::string &json);

    const std::string &checkViewStructSize(int32_t timeEntryViewItemSize, int32_t autocompleteViewItemSize, int32_t viewItemSize, int32_t settingsSize, int32_t autotrackerViewItemSize);

private:
    std::string appName_;
    std::string version_;
    bool production_;
    bool checkForUpdates_;

    CustomPocoErrorHandler error_handler_;

    TogglCallbacks callbacks_;

    UserData *user_;
};

}
