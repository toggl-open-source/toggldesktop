
#include "context.h"

#include "gui.h"
#include "urls.h"

#include <Poco/Environment.h>
#include <Poco/Logger.h>
#include <Poco/URI.h>
#include <Poco/URIStreamOpener.h>
#include <Poco/Net/HTTPStreamFactory.h>
#include <Poco/Net/HTTPSStreamFactory.h>
#include <Poco/Crypto/OpenSSLInitializer.h>

using namespace toggl;

Context::Context(const std::string &appName, const std::string &appVersion, bool production, bool checkForUpdates)
    : appName_(appName)
    , version_(appVersion)
    , production_(production)
    , checkForUpdates_(checkForUpdates)
    , events_(new EventQueue(this))
    , gui_(new GUI(this, callbacks_))
{
    if (!Poco::URIStreamOpener::defaultOpener().supportsScheme("http")) {
        Poco::Net::HTTPStreamFactory::registerFactory();
    }
    if (!Poco::URIStreamOpener::defaultOpener().supportsScheme("https")) {
        Poco::Net::HTTPSStreamFactory::registerFactory();
    }
    Poco::ErrorHandler::set(&error_handler_);
    Poco::Net::initializeSSL();

    Poco::Crypto::OpenSSLInitializer::initialize();

    if (!production) {
        urls::SetUseStagingAsBackend(true);
    }

}

Context::~Context() {
    Poco::Net::uninitializeSSL();
}

void Context::setCacertPath(const std::string &path) {
// TODO
}

bool Context::setDbPath(const std::string &path) {
// TODO
}

void Context::setUpdatePath(const std::string &path) {
// TODO
}

const std::string &Context::updatePath() {
// TODO
}

void Context::setLogPath(const std::string &path) {
// TODO
}

void Context::setLogLevel(const std::string &level) {
// TODO
}

void Context::showApp() {
// TODO
}

void Context::registerCallbacks(TogglCallbacks callbacks) {
    callbacks_ = callbacks;
    gui_->SetCallbacks(callbacks);
}

bool Context::uiStart() {
// TODO
}

bool Context::login(const std::string &email, const std::string &password) {
// TODO
}

bool Context::signup(const std::string &email, const std::string &password, uint64_t countryId) {
// TODO
}

bool Context::googleLogin(const std::string &accessToken) {
// TODO
}

void Context::passwordForgot() {
// TODO
}

void Context::tos() {
// TODO
}

void Context::privacyPolicy() {
// TODO
}

void Context::openInBrowser() {
// TODO
}

bool Context::acceptTos() {
// TODO
}

void Context::getSupport(int32_t type) {
// TODO
}

bool Context::feedbackSend(const std::string &topic, const std::string &details, const std::string &filename) {
// TODO
}

void Context::searchHelpArticles(const std::string &keywords) {
// TODO
}

void Context::viewTimeEntryList() {
// TODO
}

void Context::edit(const std::string &guid, bool editRunningTimeEntry, const std::string &focusedFieldName) {
// TODO
}

void Context::editPreferences() {
// TODO
}

bool Context::continueTimeEntry(const std::string &guid) {
// TODO
}

bool Context::continueLatestTimeEntry(bool preventOnApp) {
// TODO
}

bool Context::deleteTimeEntry(const std::string &guid) {
// TODO
}

bool Context::setTimeEntryDuration(const std::string &guid, const std::string &value) {
// TODO
}

bool Context::setTimeEntryProject(const std::string &guid, uint64_t taskId, uint64_t projectId, const std::string &projectGuid) {
// TODO
}

bool Context::setTimeEntryDate(const std::string &guid, int64_t unixTimestamp) {
// TODO
}

bool Context::setTimeEntryStart(const std::string &guid, const std::string &value) {
// TODO
}

bool Context::setTimeEntryEnd(const std::string &guid, const std::string &value) {
// TODO
}

bool Context::setTimeEntryTags(const std::string &guid, const std::string &value) {
// TODO
}

bool Context::setTimeEntryBillable(const std::string &guid, bool value) {
// TODO
}

bool Context::setTimeEntryDescription(const std::string &guid, const std::string &value) {
// TODO
}

bool Context::stop(bool preventOnApp) {
// TODO
}

bool Context::discardTimeAt(const std::string &guid, int64_t at, bool splitIntoNewEntry) {
// TODO
}

bool Context::discardTimeAndContinue(const std::string &guid, int64_t at) {
    // TODO
}

void Context::toggleEntriesGroup(const std::string &name) {
    // TODO
}

bool Context::setSettingsRemindDays(bool remindMon, bool remindTue, bool remindWed, bool remindThu, bool remindFri, bool remindSat, bool remindSun) {
// TODO
}

bool Context::setSettingsRemindTimes(const std::string &remindStarts, const std::string &remindEnds) {
// TODO
}

bool Context::setSettingsUseIdleDetection(bool useIdleDetection) {
// TODO
}

bool Context::setSettingsAutotrack(bool value) {
// TODO
}

bool Context::setSettingsOpenEditorOnShortcut(bool value) {
// TODO
}

bool Context::setSettingsAutodetectProxy(bool autodetectProxy) {
// TODO
}

bool Context::setSettingsMenubarTimer(bool menubarTimer) {
// TODO
}

bool Context::setSettingsMenubarProject(bool menubarProject) {
// TODO
}

bool Context::setSettingsDockIcon(bool dockIcon) {
// TODO
}

bool Context::setSettingsOnTop(bool onTop) {
// TODO
}

bool Context::setSettingsReminder(bool reminder) {
// TODO
}

bool Context::setSettingsPomodoro(bool pomodoro) {
// TODO
}

bool Context::setSettingsPomodoroBreak(bool pomodoroBreak) {
// TODO
}

bool Context::setSettingsStopEntryOnShutdownSleep(bool stopEntry) {
// TODO
}

bool Context::setSettingsIdleMinutes(uint64_t idleMinutes) {
// TODO
}

bool Context::setSettingsFocusOnShortcut(bool focusOnShortcut) {
// TODO
}

bool Context::setSettingsReminderMinutes(uint64_t reminderMinutes) {
// TODO
}

bool Context::setSettingsPomodoroMinutes(uint64_t pomodoroMinutes) {
// TODO
}

bool Context::setSettingsPomodoroBreakMinutes(uint64_t pomodoroBreakMinutes) {
// TODO
}

bool Context::setSettingsManualMode(bool manualMode) {
// TODO
}

bool Context::setProxySettings(bool useProxy, const std::string &proxyHost, uint64_t proxyPort, const std::string &proxyUsername, const std::string &proxyPassword) {
// TODO
}

bool Context::setWindowSettings(int64_t windowX, int64_t windowY, int64_t windowHeight, int64_t windowWidth) {
// TODO
}

bool Context::windowSettings(int64_t *windowX, int64_t *windowY, int64_t *windowHeight, int64_t *windowWidth) {
// TODO
}

void Context::setWindowMaximized(bool value) {
// TODO
}

bool Context::getWindowMaximized() {
// TODO
}

void Context::setWindowMinimized(bool value) {
// TODO
}

bool Context::getWindowMinimized() {
// TODO
}

void Context::setWindowEditSizeHeight(int64_t value) {
// TODO
}

int64_t Context::getWindowEditSizeHeight() {
// TODO
}

void Context::setWindowEditSizeWidth(int64_t value) {
// TODO
}

int64_t Context::getWindowEditSizeWidth() {
// TODO
}

void Context::setKeyStart(const std::string &value) {
// TODO
}

const std::string &Context::getKeyStart() {
// TODO
}

void Context::setKeyShow(const std::string &value) {
// TODO
}

const std::string &Context::getKeyShow() {
// TODO
}

void Context::setKeyModifierShow(const std::string &value) {
// TODO
}

const std::string &Context::getKeyModifierShow() {
// TODO
}

void Context::setKeyModifierStart(const std::string &value) {
// TODO
}

const std::string &Context::getKeyModifierStart() {
// TODO
}

bool Context::logout() {
// TODO
}

bool Context::clearCache() {
// TODO
}

const std::string &Context::start(const std::string &description, const std::string &duration, uint64_t taskId, uint64_t projectId, const std::string &projectGuid, const std::string &tags, bool preventOnApp) {
// TODO
}

const std::string &Context::addProject(const std::string &timeEntryGuid, uint64_t workspaceId, uint64_t clientId, const std::string &clientGuid, const std::string &projectName, bool isPrivate, const std::string &projectColor) {
// TODO
}

const std::string &Context::createClient(uint64_t workspaceId, const std::string &clientName) {
// TODO
}

bool Context::addObmAction(uint64_t experimentId, const std::string &key, const std::string &value) {
// TODO
}

void Context::addObmExperimentNr(uint64_t nr) {
// TODO
}

bool Context::setDefaultProject(uint64_t pid, uint64_t tid) {
// TODO
}

void Context::getProjectColors() {
// TODO
}

void Context::getCountries() {
// TODO
}

const std::string &Context::getDefaultProjectName() {
// TODO
}

uint64_t Context::getDefaultProjectId() {
// TODO
}

uint64_t Context::getDefaultTaskId() {
// TODO
}

bool Context::setUpdateChannel(const std::string &updateChannel) {
// TODO
}

const std::string &Context::getUpdateChannel() {
// TODO
}

const std::string &Context::getUserFullname() {
// TODO
}

const std::string &Context::getUserEmail() {
// TODO
}

void Context::sync() {
// TODO
}

void Context::fullsync() {
// TODO
}

bool Context::timelineToggleRecording(bool recordTimeline) {
// TODO
}

bool Context::timelineIsRecordingEnabled() {
// TODO
}

void Context::setSleep() {
// TODO
}

void Context::setWake() {
// TODO
}

void Context::setLocked() {
// TODO
}

void Context::setUnlocked() {
// TODO
}

void Context::osShutdown() {
// TODO
}

void Context::setOnline() {
// TODO
}

void Context::setIdleSeconds(uint64_t idleSeconds) {
// TODO
}

bool Context::setPromotionResponse(int64_t promotionType, int64_t promotionResponse) {
// TODO
}

const std::string &Context::formatTrackingTimeDuration(int64_t durationInSeconds) {
// TODO
}

const std::string &Context::formatTrackedTimeDuration(int64_t durationInSeconds) {
// TODO
}

int64_t Context::parseDurationStringIntoSeconds(const std::string &durationString) {
// TODO
}

void Context::debug(const std::string &text) {
// TODO
}

const std::string &Context::checkViewStructSize(int32_t timeEntryViewItemSize, int32_t autocompleteViewItemSize, int32_t viewItemSize, int32_t settingsSize, int32_t autotrackerViewItemSize) {
// TODO
}

const std::string &Context::runScript(const std::string &script, int64_t *err) {
// TODO
}

int64_t Context::autotrackerAddRule(const std::string &term, uint64_t projectId, uint64_t taskId) {
// TODO
}

bool Context::autotrackerDeleteRule(int64_t id) {
// TODO
}

void Context::testingSleep(int32_t seconds) {
// TODO
}

bool Context::testingSetLoggedInUser(const std::string &json) {
// TODO
}

void Context::setCompactMode(bool value) {
// TODO
}

bool Context::getCompactMode() {
// TODO
}

void Context::setKeepEndTimeFixed(bool value) {
// TODO
}

bool Context::getKeepEndTimeFixed() {
// TODO
}

void Context::setMiniTimerX(int64_t value) {
// TODO
}

int64_t Context::getMiniTimerX() {
// TODO
}

void Context::setMiniTimerY(int64_t value) {
// TODO
}

int64_t Context::getMiniTimerY() {
// TODO
}

void Context::setMiniTimerW(int64_t value) {
// TODO
}

int64_t Context::getMiniTimerW() {
// TODO
}

void Context::setMiniTimerVisible(bool value) {
// TODO
}

bool Context::getMiniTimerVisible() {
// TODO
}

void Context::loadMore() {
// TODO
}

void Context::trackWindowSize(uint64_t width, uint64_t height) {
// TODO
}

void Context::trackEditSize(uint64_t width, uint64_t height) {
// TODO
}

