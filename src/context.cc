
// Copyright 2014 Toggl Desktop developers

// No exceptions should be thrown from this class.
// If pointers to models are returned from this
// class, the ownership does not change and you
// must not delete the pointers you got.

// All public methods should start with an uppercase name.
// All public methods should catch their exceptions.

#include "../src/context.h"

#include <iostream>  // NOLINT

#include "./autotracker.h"
#include "./client.h"
#include "./const.h"
#include "./database.h"
#include "./error.h"
#include "./formatter.h"
#include "./https_client.h"
#include "./obm_action.h"
#include "./project.h"
#include "./settings.h"
#include "./task.h"
#include "./time_entry.h"
#include "./timeline_uploader.h"
#include "./urls.h"
#include "./window_change_recorder.h"
#include "./workspace.h"

#include "Poco/Crypto/OpenSSLInitializer.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Environment.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/FormattingChannel.h"
#include "Poco/Logger.h"
#include "Poco/Net/FilePartSource.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/Net/HTTPSStreamFactory.h"
#include "Poco/Net/NetSSL.h"
#include "Poco/Net/StringPartSource.h"
#include "Poco/Path.h"
#include "Poco/PatternFormatter.h"
#include "Poco/Random.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/Stopwatch.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/UTF8String.h"
#include "Poco/URIStreamOpener.h"
#include "Poco/Util/TimerTask.h"
#include "Poco/Util/TimerTaskAdapter.h"
#include <mutex> // NOLINT
#include <thread>

namespace toggl {

std::string Context::log_path_ = "";

Context::Context(const std::string app_name, const std::string app_version)
    : db_(nullptr)
, user_(nullptr)
, timeline_uploader_(nullptr)
, window_change_recorder_(nullptr)
, next_sync_at_(0)
, next_push_changes_at_(0)
, next_fetch_updates_at_(0)
, next_update_timeline_settings_at_(0)
, next_wake_at_(0)
, time_entry_editor_guid_("")
, environment_(APP_ENVIRONMENT)
, idle_(&ui_)
, last_sync_started_(0)
, sync_interval_seconds_(0)
, update_check_disabled_(UPDATE_CHECK_DISABLED)
, trigger_sync_(false)
, trigger_push_(false)
, quit_(false)
, ui_updater_(this, &Context::uiUpdaterActivity)
, reminder_(this, &Context::reminderActivity)
, syncer_(this, &Context::syncerActivity)
, update_path_("")
, overlay_visible_(false) {

    HTTPSClient::Config.AppName = app_name;
    HTTPSClient::Config.AppVersion = app_version;

    startPeriodicUpdateCheck();

    startPeriodicSync();

    if (!ui_updater_.isRunning()) {
        ui_updater_.start();
    }

    if (!reminder_.isRunning()) {
        reminder_.start();
    }

    resetLastTrackingReminderTime();

    pomodoro_break_entry_ = nullptr;
}

Context::~Context() {
    SetQuit();

    stopActivities();

    {
        Poco::Mutex::ScopedLock lock(window_change_recorder_m_);
        if (window_change_recorder_) {
            delete window_change_recorder_;
            window_change_recorder_ = nullptr;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            delete timeline_uploader_;
            timeline_uploader_ = nullptr;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(db_m_);
        if (db_) {
            delete db_;
            db_ = nullptr;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (user_) {
            delete user_;
            user_ = nullptr;
        }
    }

}

void Context::stopActivities() {
    try {
        {
            Poco::Mutex::ScopedLock lock(reminder_m_);
            if (reminder_.isRunning()) {
                reminder_.stop();
                reminder_.wait(2000);
            }
        }

        {
            Poco::Mutex::ScopedLock lock(ui_updater_m_);
            if (ui_updater_.isRunning()) {
                ui_updater_.stop();
                ui_updater_.wait(2000);
            }
        }


        {
            Poco::Mutex::ScopedLock lock(syncer_m_);
            if (syncer_.isRunning()) {
                syncer_.stop();
                syncer_.wait(2000);
            }
        }
    } catch(const Poco::Exception& exc) {
        logger().debug(exc.displayText());
    } catch(const std::exception& ex) {
        logger().debug(ex.what());
    } catch(const std::string& ex) {
        logger().debug(ex);
    }

    {
        Poco::Mutex::ScopedLock lock(window_change_recorder_m_);
        if (window_change_recorder_) {
            window_change_recorder_->Shutdown();
        }
    }

    {
        Poco::Mutex::ScopedLock lock(ws_client_m_);
        ws_client_.Shutdown();
    }

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            timeline_uploader_->Shutdown();
        }
    }

    TogglClient::TogglStatus.DisableStatusCheck();
}

void Context::Shutdown() {
    stopActivities();

    // cancel tasks but allow them finish
    {
        Poco::Mutex::ScopedLock lock(timer_m_);
        timer_.cancel(false);
    }

    // Stops all running threads and waits
    // for their completion (maximum 10 seconds).
    Poco::ThreadPool::defaultPool().stopAll();
}

Poco::Timestamp Context::postpone(
    const Poco::Timestamp::TimeDiff throttleMicros) const {
    return Poco::Timestamp() + throttleMicros;
}

bool Context::isPostponed(
    const Poco::Timestamp value,
    const Poco::Timestamp::TimeDiff throttleMicros) const {
    Poco::Timestamp now;
    if (now > value) {
        return false;
    }
    Poco::Timestamp::TimeDiff diff = value - now;
    if (diff > 2*throttleMicros) {
        logger().warning(
            "Cannot postpone task, its foo far in the future");
        return false;
    }
    return true;
}

int Context::nextSyncIntervalSeconds() const {
    Poco::Random random;
    random.seed();
    int n = static_cast<int>(random.next(kSyncIntervalRangeSeconds)) + kSyncIntervalRangeSeconds;
    std::stringstream ss;
    ss << "Next autosync in " << n << " seconds";
    logger().trace(ss.str());
    return n;
}

void Context::scheduleSync() {
    Poco::Int64 elapsed_seconds = Poco::Int64(time(nullptr)) - last_sync_started_;

    {
        std::stringstream ss;
        ss << "scheduleSync elapsed_seconds=" << elapsed_seconds;
        logger().debug(ss.str());
    }

    if (elapsed_seconds < sync_interval_seconds_) {
        std::stringstream ss;
        ss << "Last sync attempt less than " << sync_interval_seconds_
           << " seconds ago, chill";
        logger().trace(ss.str());
        return;
    }

    Sync();
}

void Context::onTimeEntryAutocompletes(Poco::Util::TimerTask&) {  // NOLINT
    std::vector<view::Autocomplete> time_entry_autocompletes;
    if (user_) {
        user_->related.TimeEntryAutocompleteItems(&time_entry_autocompletes);
    }
    UI()->DisplayTimeEntryAutocomplete(&time_entry_autocompletes);
}

void Context::onMiniTimerAutocompletes(Poco::Util::TimerTask&) {  // NOLINT
    std::vector<view::Autocomplete> minitimer_autocompletes;
    if (user_) {
        user_->related.MinitimerAutocompleteItems(&minitimer_autocompletes);
    }
    UI()->DisplayMinitimerAutocomplete(&minitimer_autocompletes);
}

void Context::onProjectAutocompletes(Poco::Util::TimerTask&) {  // NOLINT
    std::vector<view::Autocomplete> project_autocompletes;
    if (user_) {
        user_->related.ProjectAutocompleteItems(&project_autocompletes);
    }
    UI()->DisplayProjectAutocomplete(&project_autocompletes);
}

void Context::setOnline(const std::string reason) {
    std::stringstream ss;
    ss << "setOnline, reason:" << reason;
    logger().debug(ss.str());

    if (quit_) {
        return;
    }

    UI()->DisplayOnlineState(kOnlineStateOnline);

    scheduleSync();
}

void Context::switchWebSocketOff() {
    logger().debug("switchWebSocketOff");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchWebSocketOff);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchWebSocketOff(Poco::Util::TimerTask&) {  // NOLINT
    logger().debug("onSwitchWebSocketOff");

    Poco::Mutex::ScopedLock lock(ws_client_m_);
    ws_client_.Shutdown();
}

error Context::LoadUpdateFromJSONString(const std::string json) {
    std::stringstream ss;
    ss << "LoadUpdateFromJSONString json=" << json;
    logger().debug(ss.str());

    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger().warning("User is logged out, cannot update");
        return noError;
    }

    TimeEntry *running_entry = user_->RunningTimeEntry();

    error err = user_->LoadUserUpdateFromJSONString(json);
    if (err != noError) {
        return displayError(err);
    }

    TimeEntry *new_running_entry = user_->RunningTimeEntry();

    // Reset reminder time when entry stopped by websocket
    if (running_entry && !new_running_entry) {
        resetLastTrackingReminderTime();
    }

    return displayError(save(false));
}

void Context::switchWebSocketOn() {
    logger().debug("switchWebSocketOn");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchWebSocketOn);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchWebSocketOn(Poco::Util::TimerTask&) {  // NOLINT
    logger().debug("onSwitchWebSocketOn");

    std::string apitoken("");

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (user_) {
            apitoken = user_->APIToken();
        }
    }

    if (apitoken.empty()) {
        logger().error("No API token, cannot switch Websocket on");
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(ws_client_m_);
        ws_client_.Start(this, apitoken, on_websocket_message);
    }
}

// Start/stop timeline recording on local machine
void Context::switchTimelineOff() {
    logger().debug("switchTimelineOff");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchTimelineOff);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchTimelineOff(Poco::Util::TimerTask&) {  // NOLINT
    logger().debug("onSwitchTimelineOff");

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            delete timeline_uploader_;
            timeline_uploader_ = nullptr;
        }
    }
}

void Context::switchTimelineOn() {
    logger().debug("switchTimelineOn");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSwitchTimelineOn);

    if (quit_) {
        return;
    }

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, Poco::Timestamp());
}

void Context::onSwitchTimelineOn(Poco::Util::TimerTask&) {  // NOLINT
    logger().debug("onSwitchTimelineOn");

    if (quit_) {
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_ || !user_->RecordTimeline()) {
            return;
        }
    }

    {
        Poco::Mutex::ScopedLock lock(timeline_uploader_m_);
        if (timeline_uploader_) {
            delete timeline_uploader_;
            timeline_uploader_ = nullptr;
        }
        timeline_uploader_ = new TimelineUploader(this);
    }
}

void Context::fetchUpdates() {
    logger().debug("fetchUpdates");

    next_fetch_updates_at_ =
        postpone(kRequestThrottleSeconds * kOneSecondInMicros);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onFetchUpdates);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_fetch_updates_at_);

    std::stringstream ss;
    ss << "Next update fetch at "
       << Formatter::Format8601(next_fetch_updates_at_);
    logger().debug(ss.str());
}

void Context::onFetchUpdates(Poco::Util::TimerTask&) {  // NOLINT
    if (isPostponed(next_fetch_updates_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger().debug("onFetchUpdates postponed");
        return;
    }

    executeUpdateCheck();
}

void Context::startPeriodicSync() {
    logger().trace("startPeriodicSync");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>
    (*this, &Context::onPeriodicSync);

    sync_interval_seconds_ = nextSyncIntervalSeconds();

    Poco::Timestamp next_periodic_sync_at_ =
        Poco::Timestamp() + (sync_interval_seconds_ * kOneSecondInMicros);
    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_periodic_sync_at_);

    std::stringstream ss;
    ss << "Next periodic sync at "
       << Formatter::Format8601(next_periodic_sync_at_);
    logger().debug(ss.str());
}

void Context::onPeriodicSync(Poco::Util::TimerTask&) {  // NOLINT
    logger().debug("onPeriodicSync");

    scheduleSync();

    startPeriodicSync();
}

void Context::startPeriodicUpdateCheck() {
    logger().debug("startPeriodicUpdateCheck");

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>
    (*this, &Context::onPeriodicUpdateCheck);

    Poco::Int64 micros = kCheckUpdateIntervalSeconds *
                          Poco::Int64(kOneSecondInMicros);
    Poco::Timestamp next_periodic_check_at = Poco::Timestamp() + micros;
    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_periodic_check_at);

    std::stringstream ss;
    ss << "Next periodic update check at "
       << Formatter::Format8601(next_periodic_check_at);
    logger().debug(ss.str());
}

void Context::onPeriodicUpdateCheck(Poco::Util::TimerTask&) {  // NOLINT
    logger().debug("onPeriodicUpdateCheck");

    executeUpdateCheck();

    startPeriodicUpdateCheck();
}

error Context::UpdateChannel(
    std::string *update_channel) {
    poco_check_ptr(update_channel);

    error err = db()->LoadUpdateChannel(update_channel);

    if (err.find(kDatabaseDiskMalformed) != std::string::npos) {
        err = noError;
    }
    return displayError(err);
}

std::string Context::UserFullName() {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        return "";
    }
    return user_->Fullname();
}

std::string Context::UserEmail() {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        return "";
    }
    return user_->Email();
}

void Context::executeUpdateCheck() {
    logger().debug("executeUpdateCheck");

    displayError(downloadUpdate());
}

error Context::downloadUpdate() {
    try {
        if (update_check_disabled_) {
            return noError;
        }

        // To test updater in development, comment this block out:
        if ("production" != environment_) {
            logger().debug("Not in production, will not download updates");
            return noError;
        }

        // Load current update channel
        std::string update_channel("");
        error err = db()->LoadUpdateChannel(&update_channel);
        if (err != noError) {
            return err;
        }

        if (HTTPSClient::Config.AppVersion.empty()) {
            return error("This version cannot check for updates. This has been probably already fixed. Please check https://toggl.com/toggl-desktop/ for a newer version.");
        }

        // Ask Toggl server if we have updates
        std::string url("");
        std::string version_number("");
        {
            HTTPSRequest req;
            req.host = "https://toggl.github.io";
            req.relative_url = "/toggldesktop/assets/releases/updates.json";

            TogglClient client;
            HTTPSResponse resp = client.Get(req);
            if (resp.err != noError) {
                return resp.err;
            }

            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(resp.body, root)) {
                return error("Error parsing update check response body");
            }
            auto latestVersion = root[shortOSName()][update_channel];
            url = latestVersion[installerPlatform()].asString();
            auto versionNumberJsonToken = latestVersion["version"];
            if (versionNumberJsonToken.empty()) {
                return error("No versions found for OS " + shortOSName() + ", platform " + installerPlatform() + ", channel " + update_channel);
            }
            version_number = versionNumberJsonToken.asString();

            if (lessThanVersion(HTTPSClient::Config.AppVersion, version_number)) {
                std::stringstream ss;
                ss << "Found update " << version_number
                   << " (" << url << ")";
                logger().debug(ss.str());
            } else {
                logger().debug("The app is up to date");
                if (UI()->CanDisplayUpdate()) {
                    UI()->DisplayUpdate("");
                }
                return noError;
            }
        }

        // linux has non-silent updates, just pass on the URL
        // linux users will download the update themselves
        if (UI()->CanDisplayUpdate()) {
            UI()->DisplayUpdate(url);
            return noError;
        }

        // we need a path to download to, when going this way
        if (update_path_.empty()) {
            return error("update path is empty, cannot download update");
        }

        // Ignore update if not compatible with this client version
        // only windows .exe installers are supported atm
        if (url.find(".exe") == std::string::npos) {
            logger().debug("Update is not compatible with this client,"
                           " will ignore");
            return noError;
        }

        // Download update if it's not downloaded yet.
        {
            Poco::URI uri(url);

            std::vector<std::string> path_segments;
            uri.getPathSegments(path_segments);

            Poco::Path save_location(update_path_);
            save_location.append(path_segments.back());
            std::string file = save_location.toString();

            Poco::File f(file);
            if (f.exists()) {
                logger().debug("File already exists: " + file);
                return noError;
            }

            Poco::File(update_path_).createDirectory();

            if (UI()->CanDisplayUpdateDownloadState()) {
                UI()->DisplayUpdateDownloadState(
                    version_number,
                    kDownloadStatusStarted);
            }

            std::auto_ptr<std::istream> stream(
                Poco::URIStreamOpener::defaultOpener().open(uri));
            Poco::FileOutputStream fos(file, std::ios::binary);
            Poco::StreamCopier::copyStream(*stream.get(), fos);
            fos.flush();
            fos.close();

            if (UI()->CanDisplayUpdateDownloadState()) {
                UI()->DisplayUpdateDownloadState(
                    version_number,
                    kDownloadStatusDone);
            }
        }
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

const std::string Context::linuxPlatformName() {
    if (kDebianPackage) {
        return "deb64";
    }
    return std::string("linux");
}

const std::string Context::windowsPlatformName() {
#if defined(_WIN64)
    return "windows64";
#elif defined(_WIN32)
    BOOL f64 = FALSE;
    if (IsWow64Process(GetCurrentProcess(), &f64) && f64)
        return "windows64";
    return "windows";
#else
    return "windows";
#endif
}

const std::string Context::installerPlatform() {
    std::stringstream ss;
    if (POCO_OS_LINUX == POCO_OS) {
        ss <<  linuxPlatformName();
    } else if (POCO_OS_WINDOWS_NT == POCO_OS) {
        ss << windowsPlatformName();
    } else {
        ss << "macos";
    }
    if (kEnterpriseInstall) {
        ss << "_enterprise";
    }
    return ss.str();
}

const std::string Context::shortOSName() {
    if (POCO_OS_LINUX == POCO_OS) {
        return "linux";
    } else if (POCO_OS_WINDOWS_NT == POCO_OS) {
        return "win";
    } else if (POCO_OS_MAC_OS_X == POCO_OS) {
        return "mac";
    }
    return "unknown";
}

void Context::parseVersion(int result[4], const std::string& input) {
    std::istringstream parser(input);
    parser >> result[0];
    for (auto idx = 1; idx < 4; idx++) {
        parser.get(); //Skip period
        parser >> result[idx];
    }
}

bool Context::lessThanVersion(const std::string& version1, const std::string& version2) {
    int parsed1[4] {}, parsed2[4] {};
    parseVersion(parsed1, version1);
    parseVersion(parsed2, version2);
    return std::lexicographical_compare(parsed1, &parsed1[4], parsed2, &parsed2[4]);
}

void Context::TimelineUpdateServerSettings() {
    logger().debug("TimelineUpdateServerSettings");

    next_update_timeline_settings_at_ =
        postpone(kRequestThrottleSeconds * kOneSecondInMicros);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this,
                &Context::onTimelineUpdateServerSettings);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_update_timeline_settings_at_);

    std::stringstream ss;
    ss << "Next timeline settings update at "
       << Formatter::Format8601(next_update_timeline_settings_at_);
    logger().debug(ss.str());
}

const std::string kRecordTimelineEnabledJSON = "{\"record_timeline\": true}";
const std::string kRecordTimelineDisabledJSON = "{\"record_timeline\": false}";

void Context::onTimelineUpdateServerSettings(Poco::Util::TimerTask&) {  // NOLINT
    if (isPostponed(next_update_timeline_settings_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger().debug("onTimelineUpdateServerSettings postponed");
        return;
    }

    logger().debug("onTimelineUpdateServerSettings executing");

    std::string apitoken("");
    std::string json(kRecordTimelineDisabledJSON);

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return;
        }
        if (user_->RecordTimeline()) {
            json = kRecordTimelineEnabledJSON;
        }
        apitoken = user_->APIToken();
    }

    // Not implemented in v9 as of 12.05.2017
    HTTPSRequest req;
    req.host = urls::TimelineUpload();
    req.relative_url = "/api/v8/timeline_settings";
    req.payload = json;
    req.basic_auth_username = apitoken;
    req.basic_auth_password = "api_token";

    TogglClient client(UI());
    HTTPSResponse resp = client.Post(req);
    if (resp.err != noError) {
        displayError(resp.err);
        logger().error(resp.body);
        logger().error(resp.err);
    }
}

error Context::SendFeedback(Feedback fb) {
    if (!user_) {
        logger().warning("Cannot send feedback, user logged out");
        return noError;
    }

    error err = fb.Validate();
    if (err != noError) {
        return displayError(err);
    }

    feedback_ = fb;

    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(
            *this, &Context::onSendFeedback);

    {
        Poco::Mutex::ScopedLock lock(timer_m_);
        timer_.schedule(ptask, Poco::Timestamp());
    }

    return noError;
}

void Context::onSendFeedback(Poco::Util::TimerTask&) {  // NOLINT
    logger().debug("onSendFeedback");

    std::string api_token_value("");
    std::string api_token_name("");

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (user_) {
            api_token_value = user_->APIToken();
            api_token_name = "api_token";
        }
    }

    std::string update_channel("");
    UpdateChannel(&update_channel);

    Poco::Net::HTMLForm form;
    Json::Value settings_json;

    form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);

    form.set("desktop", "true");
    form.set("toggl_version", HTTPSClient::Config.AppVersion);
    form.set("details", Formatter::EscapeJSONString(feedback_.Details()));
    form.set("subject", Formatter::EscapeJSONString(feedback_.Subject()));
    form.set("date", Formatter::Format8601(time(nullptr)));
    form.set("update_channel", Formatter::EscapeJSONString(update_channel));

    if (!feedback_.AttachmentPath().empty()) {
        form.addPart("files",
                     new Poco::Net::FilePartSource(feedback_.AttachmentPath()));
    }

    // Add all log files to feedback
    int count = 0;
    bool exists = true;

    while (exists) {
        std::stringstream ss;
        ss << log_path_ << "." << std::to_string(count);
        Poco::File file(ss.str());
        exists = file.exists();
        if (exists) {
            form.addPart("files",
                         new Poco::Net::FilePartSource(ss.str()));
        }
        count++;
    }

    form.addPart("files",
                 new Poco::Net::FilePartSource(log_path_));

    settings_json = settings_.SaveToJSON();
    if (user_) {
        settings_json["record_timeline"] = user_->RecordTimeline();
    }

    form.addPart("files",
                 new Poco::Net::StringPartSource(
                     Json::StyledWriter().write(settings_json),
                     "application/json",
                     "settings.json"));

    // Not implemented in v9 as of 12.05.2017
    HTTPSRequest req;
    req.host = urls::API();
    req.relative_url ="/api/v8/feedback/web";
    req.basic_auth_username = api_token_value;
    req.basic_auth_password = api_token_name;
    req.form = &form;

    TogglClient client(UI());
    HTTPSResponse resp = client.Post(req);
    logger().debug("Feedback result: " + resp.err);
    if (resp.err != noError) {
        displayError(resp.err);
        return;
    }
}

error Context::SetSettingsRemindTimes(
    const std::string remind_starts,
    const std::string remind_ends) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsRemindTimes(remind_starts, remind_ends));
}

error Context::SetSettingsRemindDays(
    const bool remind_mon,
    const bool remind_tue,
    const bool remind_wed,
    const bool remind_thu,
    const bool remind_fri,
    const bool remind_sat,
    const bool remind_sun) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsRemindDays(
            remind_mon,
            remind_tue,
            remind_wed,
            remind_thu,
            remind_fri,
            remind_sat,
            remind_sun));
}

error Context::SetSettingsAutodetectProxy(const bool autodetect_proxy) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsAutodetectProxy(autodetect_proxy));
}

error Context::SetSettingsUseIdleDetection(const bool use_idle_detection) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsUseIdleDetection(use_idle_detection));
}

error Context::applySettingsSaveResultToUI(const error err) {
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    return noError;
}

error Context::SetSettingsAutotrack(const bool value) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsAutotrack(value));
}

error Context::SetSettingsOpenEditorOnShortcut(const bool value) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsOpenEditorOnShortcut(value));
}

error Context::SetSettingsMenubarTimer(const bool menubar_timer) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsMenubarTimer(menubar_timer));
}

error Context::SetSettingsMenubarProject(const bool menubar_project) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsMenubarProject(menubar_project));
}

error Context::SetSettingsDockIcon(const bool dock_icon) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsDockIcon(dock_icon));
}

error Context::SetSettingsOnTop(const bool on_top) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsOnTop(on_top));
}

error Context::SetSettingsReminder(const bool reminder) {
    error err = applySettingsSaveResultToUI(
        db()->SetSettingsReminder(reminder));
    if (err == noError) {
        resetLastTrackingReminderTime();
    }
    return err;
}

error Context::SetSettingsPomodoro(const bool pomodoro) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsPomodoro(pomodoro));
}

error Context::SetSettingsPomodoroBreak(const bool pomodoro_break) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsPomodoroBreak(pomodoro_break));
}

error Context::SetSettingsStopEntryOnShutdownSleep(const bool stop_entry) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsStopEntryOnShutdownSleep(stop_entry));
}

error Context::SetSettingsIdleMinutes(const Poco::UInt64 idle_minutes) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsIdleMinutes(idle_minutes));
}

error Context::SetSettingsFocusOnShortcut(const bool focus_on_shortcut) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsFocusOnShortcut(focus_on_shortcut));
}

error Context::SetSettingsManualMode(const bool manual_mode) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsManualMode(manual_mode));
}

error Context::SetSettingsReminderMinutes(const Poco::UInt64 reminder_minutes) {
    const error err = applySettingsSaveResultToUI(
        db()->SetSettingsReminderMinutes(reminder_minutes));
    if (err == noError) {
        resetLastTrackingReminderTime();
    }
    return err;
}

error Context::SetSettingsPomodoroMinutes(const Poco::UInt64 pomodoro_minutes) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsPomodoroMinutes(pomodoro_minutes));
}

error Context::SetSettingsPomodoroBreakMinutes(
    const Poco::UInt64 pomodoro_break_minutes) {
    return applySettingsSaveResultToUI(
        db()->SetSettingsPomodoroBreakMinutes(pomodoro_break_minutes));
}

error Context::LoadWindowSettings(
    int64_t *window_x,
    int64_t *window_y,
    int64_t *window_height,
    int64_t *window_width) {

    Poco::Int64 x(0), y(0), h(0), w(0);

    error err = db()->LoadWindowSettings(&x, &y, &h, &w);
    if (noError == err) {
        *window_x = x;
        *window_y = y;
        *window_height = h;
        *window_width = w;
    }
    return displayError(err);
}

error Context::SaveWindowSettings(
    const int64_t window_x,
    const int64_t window_y,
    const int64_t window_height,
    const int64_t window_width) {

    error err = db()->SaveWindowSettings(
        window_x,
        window_y,
        window_height,
        window_width);
    return displayError(err);
}

Poco::Int64 Context::GetMiniTimerX() {
    Poco::Int64 value(0);
    displayError(db()->GetMiniTimerX(&value));
    return value;
}

void Context::SetMiniTimerX(const int64_t x) {
    displayError(db()->SetMiniTimerX(x));
}

Poco::Int64 Context::GetMiniTimerY() {
    Poco::Int64 value(0);
    displayError(db()->GetMiniTimerY(&value));
    return value;
}

void Context::SetMiniTimerY(const int64_t y) {
    displayError(db()->SetMiniTimerY(y));
}

Poco::Int64 Context::GetMiniTimerW() {
    Poco::Int64 value(0);
    displayError(db()->GetMiniTimerW(&value));
    return value;
}

void Context::SetMiniTimerW(const int64_t y) {
    displayError(db()->SetMiniTimerW(y));
}

void Context::SetCompactMode(
    const bool value) {
    displayError(db()->SetCompactMode(value));
}

bool Context::GetCompactMode() {
    bool value(false);
    displayError(db()->GetCompactMode(&value));
    return value;
}

void Context::SetMiniTimerVisible(
    const bool value) {
    displayError(db()->SetMiniTimerVisible(value));
}

bool Context::GetMiniTimerVisible() {
    bool value(false);
    displayError(db()->GetMiniTimerVisible(&value));
    return value;
}

void Context::SetKeepEndTimeFixed
(const bool value) {
    displayError(db()->SetKeepEndTimeFixed(value));
}

bool Context::GetKeepEndTimeFixed() {
    bool value(false);
    displayError(db()->GetKeepEndTimeFixed(&value));
    return value;
}

void Context::SetWindowMaximized(
    const bool value) {
    displayError(db()->SetWindowMaximized(value));
}

bool Context::GetWindowMaximized() {
    bool value(false);
    displayError(db()->GetWindowMaximized(&value));
    return value;
}

void Context::SetWindowMinimized(
    const bool_t value) {
    displayError(db()->SetWindowMinimized(value));
}

bool Context::GetWindowMinimized() {
    bool value(false);
    displayError(db()->GetWindowMinimized(&value));
    return value;
}

void Context::SetWindowEditSizeHeight(
    const int64_t value) {
    displayError(db()->SetWindowEditSizeHeight(value));
}

int64_t Context::GetWindowEditSizeHeight() {
    Poco::Int64 value(0);
    displayError(db()->GetWindowEditSizeHeight(&value));
    return value;
}

void Context::SetWindowEditSizeWidth(
    const int64_t value) {
    displayError(db()->SetWindowEditSizeWidth(value));
}

int64_t Context::GetWindowEditSizeWidth() {
    Poco::Int64 value(0);
    displayError(db()->GetWindowEditSizeWidth(&value));
    return value;
}

void Context::SetKeyStart(
    const std::string value) {
    displayError(db()->SetKeyStart(value));
}

std::string Context::GetKeyStart() {
    std::string value("");
    displayError(db()->GetKeyStart(&value));
    return value;
}

void Context::SetKeyShow(
    const std::string value) {
    displayError(db()->SetKeyShow(value));
}

std::string Context::GetKeyShow() {
    std::string value("");
    displayError(db()->GetKeyShow(&value));
    return value;
}

void Context::SetKeyModifierShow(
    const std::string value) {
    displayError(db()->SetKeyModifierShow(value));
}

std::string Context::GetKeyModifierShow() {
    std::string value("");
    displayError(db()->GetKeyModifierShow(&value));
    return value;
}

void Context::SetKeyModifierStart(
    const std::string value) {
    displayError(db()->SetKeyModifierStart(value));
}

std::string Context::GetKeyModifierStart() {
    std::string value("");
    displayError(db()->GetKeyModifierStart(&value));
    return value;
}

error Context::SetProxySettings(
    const bool use_proxy,
    const Proxy proxy) {

    bool was_using_proxy(false);
    Proxy previous_proxy_settings;
    error err = db()->LoadProxySettings(&was_using_proxy,
                                        &previous_proxy_settings);
    if (err != noError) {
        return displayError(err);
    }

    err = db()->SaveProxySettings(use_proxy, proxy);
    if (err != noError) {
        return displayError(err);
    }

    UIElements render;
    render.display_settings = true;
    updateUI(render);

    if (use_proxy != was_using_proxy
            || proxy.Host() != previous_proxy_settings.Host()
            || proxy.Port() != previous_proxy_settings.Port()
            || proxy.Username() != previous_proxy_settings.Username()
            || proxy.Password() != previous_proxy_settings.Password()) {
        Sync();
        switchWebSocketOn();
    }

    return noError;
}

void Context::OpenSettings() {
    logger().debug("OpenSettings");

    UIElements render;
    render.display_settings = true;
    render.open_settings = true;
    updateUI(render);
}

error Context::SetDBPath(
    const std::string path) {
    try {
        std::stringstream ss;
        ss << "SetDBPath " << path;
        logger().debug(ss.str());

        Poco::Mutex::ScopedLock lock(db_m_);
        if (db_) {
            logger().debug("delete db_ from SetDBPath()");
            delete db_;
            db_ = nullptr;
        }
        db_ = new Database(path);
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

void Context::SetEnvironment(const std::string value) {
    if (!("production" == value ||
            "development" == value ||
            "test" == value)) {
        std::stringstream ss;
        ss << "Invalid environment '" << value << "'!";
        logger().error(ss.str());
        return;
    }
    logger().debug("SetEnvironment " + value);
    environment_ = value;

    HTTPSClient::Config.IgnoreCert = ("development" == environment_);
    urls::SetRequestsAllowed("test" != environment_);
}

Database *Context::db() const {
    poco_check_ptr(db_);
    return db_;
}

error Context::attemptOfflineLogin(const std::string email,
                                   const std::string password) {
    if (email.empty()) {
        return error("cannot login offline without an e-mail");
    }

    if (password.empty()) {
        return error("cannot login offline without a password");
    }

    User *user = new User();

    error err = db()->LoadUserByEmail(email, user);
    if (err != noError) {
        delete user;
        return err;
    }

    if (!user->ID()) {
        delete user;
        logger().debug("User data not found in local database for " + email);
        return error(kEmailNotFoundCannotLogInOffline);
    }

    if (user->OfflineData().empty()) {
        delete user;
        logger().debug("Offline data not found in local database for "
                       + email);
        return error(kEmailNotFoundCannotLogInOffline);
    }

    err = user->SetAPITokenFromOfflineData(password);
    if ("I/O error" == err || err.find("bad decrypt") != std::string::npos) {
        delete user;
        return error(kInvalidPassword);
    }
    if (err != noError) {
        delete user;
        return err;
    }

    err = db()->SetCurrentAPIToken(user->APIToken(), user->ID());
    if (err != noError) {
        delete user;
        return err;
    }

    setUser(user, true);

    updateUI(UIElements::Reset());

    return save(false);
}

void Context::setUser(User *value, const bool logged_in) {
    {
        std::stringstream ss;
        ss << "setUser user_logged_in=" << logged_in;
        logger().debug(ss.str());
    }

    Poco::UInt64 user_id(0);

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (user_) {
            delete user_;
        }
        user_ = value;
        if (user_) {
            user_id = user_->ID();
        }
    }

    if (quit_) {
        return;
    }

    if (!user_id) {
        UI()->DisplayLogin(true, 0);

        {
            Poco::Mutex::ScopedLock l(window_change_recorder_m_);
            if (window_change_recorder_) {
                delete window_change_recorder_;
                window_change_recorder_ = nullptr;
            }
        }

        switchTimelineOff();

        switchWebSocketOff();

        // Reset autotracker view
        // Autotracker rules has a project autocomplete, too
        autotracker_titles_.clear();
        UIElements render;
        render.display_autotracker_rules = true;
        render.display_project_autocomplete = true;
        updateUI(render);

        return;
    }

    UI()->DisplayLogin(false, user_id);

    {
        Poco::Mutex::ScopedLock l(window_change_recorder_m_);
        if (window_change_recorder_) {
            delete window_change_recorder_;
            window_change_recorder_ = nullptr;
        }
        window_change_recorder_ = new WindowChangeRecorder(this);
    }

    switchTimelineOn();

    switchWebSocketOn();

    if (!logged_in) {
        Sync();
    }

    fetchUpdates();

    if (!ui_updater_.isRunning()) {
        ui_updater_.start();
    }

    if (!reminder_.isRunning()) {
        reminder_.start();
    }

    // Offer beta channel, if not offered yet
    bool did_offer_beta_channel(false);
    error err = offerBetaChannel(&did_offer_beta_channel);
    if (err != noError) {
        displayError(err);
    }
}

error Context::SetLoggedInUserFromJSON(
    const std::string json) {

    if (json.empty()) {
        return displayError("empty JSON");
    }

    Poco::UInt64 userID(0);
    error err = User::UserID(json, &userID);
    if (err != noError) {
        return displayError(err);
    }

    if (!userID) {
        return displayError("missing user ID in JSON");
    }

    User *user = new User();

    err = db()->LoadUserByID(userID, user);
    if (err != noError) {
        delete user;
        return displayError(err);
    }

    err = user->LoadUserAndRelatedDataFromJSONString(json, true);
    if (err != noError) {
        delete user;
        return displayError(err);
    }

    err = db()->SetCurrentAPIToken(user->APIToken(), user->ID());
    if (err != noError) {
        delete user;
        return displayError(err);
    }

    setUser(user, true);

    updateUI(UIElements::Reset());

    err = save(false);
    if (err != noError) {
        return displayError(err);
    }

    // Fetch OBM experiments..
    err = pullObmExperiments();
    if (err != noError) {
        logger().error("Error pulling OBM experiments: " + err);
    }

    // ..and run the OBM experiments
    err = runObmExperiments();
    if (err != noError) {
        logger().error("Error running OBM experiments: " + err);
    }

    return noError;
}

error Context::Logout() {
    try {
        if (!user_) {
            logger().warning("User is logged out, cannot logout again");
            return noError;
        }

        error err = db()->ClearCurrentAPIToken();
        if (err != noError) {
            return displayError(err);
        }

        logger().debug("setUser from Logout");
        overlay_visible_ = false;
        setUser(nullptr);

        UI()->resetFirstLaunch();
        UI()->DisplayApp();

        Shutdown();
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::ClearCache() {
    try {
        error err = noError;

        err = db()->ResetWindow();

        if (err != noError) {
            return displayError(err);
        }

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("User is logged out, cannot clear cache");
                return noError;
            }
            err = db()->DeleteUser(user_, true);
        }

        if (err != noError) {
            return displayError(err);
        }

        return Logout();
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
}

TimeEntry *Context::Start(
    const std::string description,
    const std::string duration,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string project_guid,
    const std::string tags,
    const bool prevent_on_app) {

    // Do not even allow to add new time entries,
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        displayError(kUnsupportedAppError);
        return nullptr;
    }

    // Discard Start if WS missing error is present
    if (overlay_visible_) {
        return nullptr;
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot start tracking, user logged out");
            return nullptr;
        }

        Poco::UInt64 tid(task_id);

        // Check if there's a default PID set
        Poco::UInt64 pid(project_id);
        if (!pid && project_guid.empty()) {
            pid = user_->DefaultPID();
            // Check if there's a default TID set
            tid = user_->DefaultTID();
        }

        te = user_->Start(description,
                          duration,
                          tid,
                          pid,
                          project_guid,
                          tags);
    }

    error err = save(true);
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    if ("production" == environment_) {
        analytics_.TrackAutocompleteUsage(db_->AnalyticsClientID(),
                                          task_id || project_id);
    }

    OpenTimeEntryList();

    if (!prevent_on_app && settings_.focus_on_shortcut) {
        // Show app
        UI()->DisplayApp();
    }

    if (te && settings_.open_editor_on_shortcut) {
        if (!prevent_on_app && !settings_.focus_on_shortcut) {
            // Show app
            UI()->DisplayApp();
        }
        // Open time entry in editor
        OpenTimeEntryEditor(te->GUID(), true, "");
    }

    return te;
}

void Context::OpenTimeEntryEditor(
    const std::string GUID,
    const bool edit_running_entry,
    const std::string focused_field_name) {
    if (!edit_running_entry && GUID.empty()) {
        logger().error("Cannot edit time entry without a GUID");
        return;
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot edit time entry, user logged out");
            return;
        }

        if (edit_running_entry) {
            te = user_->RunningTimeEntry();
        } else {
            te = user_->related.TimeEntryByGUID(GUID);
        }
    }

    if (!te) {
        logger().warning("Time entry not found for edit " + GUID);
        return;
    }

    UIElements render;
    render.open_time_entry_editor = true;
    render.display_time_entry_editor = true;
    render.time_entry_editor_guid = te->GUID();
    render.time_entry_editor_field = focused_field_name;

    // If user is already editing the time entry, toggle the editor
    // instead of doing nothing
    if (time_entry_editor_guid_ == te->GUID()) {
        render.open_time_entry_editor = false;
        render.display_time_entry_editor = false;
        render.time_entry_editor_guid = "";
        render.time_entry_editor_field = "";

        render.open_time_entry_list = true;
        render.display_time_entries = true;
    }

    updateUI(render);
}

TimeEntry *Context::ContinueLatest(const bool prevent_on_app) {
    // Do not even allow to continue entries,
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        displayError(kUnsupportedAppError);
        return nullptr;
    }

    // Discard Start if WS missing error is present
    if (overlay_visible_) {
        return nullptr;
    }

    TimeEntry *result = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot continue tracking, user logged out");
            return nullptr;
        }

        TimeEntry *latest = user_->related.LatestTimeEntry();

        if (!latest) {
            return nullptr;
        }

        result = user_->Continue(
            latest->GUID(),
            settings_.manual_mode);
    }



    error err = save(true);
    if (noError != err) {
        displayError(err);
        return nullptr;
    }

    if (settings_.manual_mode && result) {
        UIElements render;
        render.open_time_entry_editor = true;
        render.display_time_entry_editor = true;
        render.time_entry_editor_guid = result->GUID();
        updateUI(render);
    }

    if (!prevent_on_app && settings_.focus_on_shortcut) {
        // Show app
        UI()->DisplayApp();
    }

    if (result && settings_.open_editor_on_shortcut) {
        if (!prevent_on_app && !settings_.focus_on_shortcut) {
            // Show app
            UI()->DisplayApp();
        }
        // Open time entry in editor
        OpenTimeEntryEditor(result->GUID(), true, "");
    }

    return result;
}

TimeEntry *Context::Continue(
    const std::string GUID) {

    // Do not even allow to continue entries,
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        displayError(kUnsupportedAppError);
        return nullptr;
    }

    // Discard Start if WS missing error is present
    if (overlay_visible_) {
        return nullptr;
    }

    if (GUID.empty()) {
        displayError(std::string(__FUNCTION__) + ": Missing GUID");
        return nullptr;
    }

    TimeEntry *result = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot continue time entry, user logged out");
            return nullptr;
        }

        result = user_->Continue(
            GUID,
            settings_.manual_mode);
    }

    error err = save(true);
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    if (settings_.manual_mode && result) {
        UIElements render;
        render.open_time_entry_editor = true;
        render.display_time_entry_editor = true;
        render.time_entry_editor_guid = result->GUID();
        updateUI(render);
    } else {
        OpenTimeEntryList();
    }

    return result;
}

error Context::DeleteTimeEntryByGUID(const std::string GUID) {
    // Do not even allow to delete time entries,
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        return displayError(kUnsupportedAppError);
    }

    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot delete time entry, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (te->DeletedAt()) {
            return displayError(kCannotDeleteDeletedTimeEntry);
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    if (te->IsTracking()) {
        error err = Stop(false);
        if (err != noError) {
            return displayError(err);
        }
    }
    te->ClearValidationError();
    te->Delete();
    return displayError(save(true));
}

error Context::SetTimeEntryDuration(
    const std::string GUID,
    const std::string duration) {
    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger().warning("Cannot set duration, user logged out");
        return noError;
    }
    TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
    if (!te) {
        logger().warning("Time entry not found: " + GUID);
        return noError;
    }

    if (isTimeEntryLocked(te)) {
        return logAndDisplayUserTriedEditingLockedEntry();
    }

    // validate the value
    int seconds = Formatter::ParseDurationString(duration);
    if (seconds >= kMaxDurationSeconds) {
        return displayError(error(kOverMaxDurationError));
    }

    te->SetDurationUserInput(duration);
    return displayError(save(true));
}

error Context::SetTimeEntryProject(
    const std::string GUID,
    const Poco::UInt64 task_id,
    const Poco::UInt64 project_id,
    const std::string project_guid) {
    try {
        if (GUID.empty()) {
            return displayError(std::string(__FUNCTION__) + ": Missing GUID");
        }

        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot set project, user logged out");
            return noError;
        }

        TimeEntry *te = user_->related.TimeEntryByGUID(GUID);
        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }

        Project *p = nullptr;
        if (project_id) {
            p = user_->related.ProjectByID(project_id);
        }
        if (p == nullptr && !project_guid.empty()) {
            p = user_->related.ProjectByGUID(project_guid);
        }

        if (p && !canChangeProjectTo(te, p)) {
            return displayError(error(
                "Cannot change project: would end up with locked time entry"));
        }

        if (p) {
            // If user re-assigns project, don't mess with the billable
            // flag any more. (User selected billable project, unchecked
            // billable, // then selected the same project again).
            if (p->ID() != te->PID()
                    || (!project_guid.empty() && p->GUID().compare(te->ProjectGUID()) != 0)) {
                te->SetBillable(p->Billable());
            }
            te->SetWID(p->WID());
        }
        te->SetTID(task_id);
        te->SetPID(project_id);
        te->SetProjectGUID(project_guid);

        if (te->Dirty()) {
            te->ClearValidationError();
            te->SetUIModified();
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return displayError(save(true));
}

error Context::SetTimeEntryDate(
    const std::string GUID,
    const Poco::Int64 unix_timestamp) {

    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    TimeEntry *te;
    Poco::LocalDateTime dt;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot change date, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return displayError(error("Cannot change locked time entry."));
        }

        Poco::LocalDateTime date_part(
            Poco::Timestamp::fromEpochTime(unix_timestamp));

        Poco::LocalDateTime time_part(
            Poco::Timestamp::fromEpochTime(te->Start()));

        // Validate date input
        if (date_part.year() < kMinimumAllowedYear || date_part.year() > kMaximumAllowedYear) {
            return displayError(error(kInvalidDateError));
        }

        dt = Poco::LocalDateTime(
            date_part.year(), date_part.month(), date_part.day(),
            time_part.hour(), time_part.minute(), time_part.second());

        if (!canChangeStartTimeTo(te, dt.timestamp().epochTime())) {
            return displayError(
                error(
                    "Failed to change time entry date: workspace is locked."));
        }
    }

    std::string s = Poco::DateTimeFormatter::format(
        dt,
        Poco::DateTimeFormat::ISO8601_FORMAT);

    te->SetStartUserInput(s, false);

    return displayError(save(true));
}


error Context::SetTimeEntryStart(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }
    Poco::LocalDateTime now;

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot change start time, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    Poco::LocalDateTime local(Poco::Timestamp::fromEpochTime(te->Start()));

    // Validate time input
    if (local.year() < kMinimumAllowedYear || local.year() > kMaximumAllowedYear) {
        return displayError(error(kInvalidStartTimeError));
    }

    int hours(0), minutes(0);
    if (!toggl::Formatter::ParseTimeInput(value, &hours, &minutes)) {
        return error("invalid time format");
    }

    Poco::LocalDateTime dt(
        local.year(), local.month(), local.day(),
        hours, minutes, local.second());

    // check if time is in future and subtrack 1 day if needed
    if (dt.utcTime() > now.utcTime()) {
        Poco::LocalDateTime new_date =
            dt - Poco::Timespan(1 * Poco::Timespan::DAYS);
        dt = new_date;
    }

    std::string s = Poco::DateTimeFormatter::format(
        dt, Poco::DateTimeFormat::ISO8601_FORMAT);

    te->SetStartUserInput(s, GetKeepEndTimeFixed());

    return displayError(save(true));
}

error Context::SetTimeEntryStop(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot change stop time, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    Poco::LocalDateTime stop(
        Poco::Timestamp::fromEpochTime(te->Stop()));

    int hours(0), minutes(0);
    if (!toggl::Formatter::ParseTimeInput(value, &hours, &minutes)) {
        return error("invalid time format");
    }

// By default, keep end date, only change hour && minute
    Poco::LocalDateTime new_stop(
        stop.tzd(),
        stop.year(), stop.month(), stop.day(),
        hours, minutes, stop.second(), 0, 0);

// If end time is on same date as start,
// but is not less than start by hour & minute, then
// assume that the end must be on same date as start.
    Poco::LocalDateTime start(
        Poco::Timestamp::fromEpochTime(te->Start()));
    if (new_stop.day() != start.day()) {
        if (new_stop.hour() >= start.hour()) {
            new_stop = Poco::LocalDateTime(
                start.tzd(),
                start.year(), start.month(), start.day(),
                hours, minutes, stop.second(), 0, 0);
        }
    }

    std::string s = Poco::DateTimeFormatter::format(
        new_stop, Poco::DateTimeFormat::ISO8601_FORMAT);

    te->SetStopUserInput(s);

    return displayError(save(true));
}

error Context::SetTimeEntryTags(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot set tags, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    te->SetTags(value);

    if (te->Dirty()) {
        te->ClearValidationError();
        te->SetUIModified();
    }

    return displayError(save(true));
}

error Context::SetTimeEntryBillable(
    const std::string GUID,
    const bool value) {
    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot set billable, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    te->SetBillable(value);

    if (te->Dirty()) {
        te->ClearValidationError();
        te->SetUIModified();
    }

    return displayError(save(true));
}

error Context::SetTimeEntryDescription(
    const std::string GUID,
    const std::string value) {
    if (GUID.empty()) {
        return displayError(std::string(__FUNCTION__) + ": Missing GUID");
    }

    TimeEntry *te = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot set description, user logged out");
            return noError;
        }
        te = user_->related.TimeEntryByGUID(GUID);

        if (!te) {
            logger().warning("Time entry not found: " + GUID);
            return noError;
        }

        if (isTimeEntryLocked(te)) {
            return logAndDisplayUserTriedEditingLockedEntry();
        }
    }

    // Validate description length
    if (value.length() > kMaximumDescriptionLength) {
        return displayError(error(kMaximumDescriptionLengthError));
    }

    te->SetDescription(value);

    if (te->Dirty()) {
        te->ClearValidationError();
        te->SetUIModified();
    }

    return displayError(save(true));
}

error Context::Stop(const bool prevent_on_app) {
    std::vector<TimeEntry *> stopped;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot stop tracking, user logged out");
            return noError;
        }
        user_->Stop(&stopped);

        resetLastTrackingReminderTime();
    }

    if (stopped.empty()) {
        logger().warning("No time entry was found to stop");
        return noError;
    }

    if (!prevent_on_app && settings_.focus_on_shortcut) {
        UI()->DisplayApp();
    }

    error err = save(true);
    if (err != noError) {
        return displayError(err);
    }

    OpenTimeEntryList();

    return noError;
}

error Context::DiscardTimeAt(
    const std::string guid,
    const Poco::Int64 at,
    const bool split_into_new_entry) {

    // Reset reminder count when doing idle actions
    resetLastTrackingReminderTime();

    // Tracking action
    if ("production" == environment_) {
        std::stringstream ss;
        if (split_into_new_entry) {
            ss << "idle-as-new-entry";
        } else {
            ss << "discard-and-stop";
        }

        analytics_.TrackIdleDetectionClick(db_->AnalyticsClientID(),
                                           ss.str());
    }

    TimeEntry *split = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot stop time entry, user logged out");
            return noError;
        }

        split = user_->DiscardTimeAt(guid, at, split_into_new_entry);
    }

    error err = save(true);
    if (err != noError) {
        return displayError(err);
    }

    if (split_into_new_entry && split) {
        UIElements render;
        render.open_time_entry_editor = true;
        render.display_time_entry_editor = true;
        render.time_entry_editor_guid = split->GUID();
        render.time_entry_editor_field = "";
        updateUI(render);
    }

    return noError;
}

TimeEntry *Context::DiscardTimeAndContinue(
    const std::string guid,
    const Poco::Int64 at) {

    // Reset reminder count when doing idle actions
    last_tracking_reminder_time_ = time(nullptr);

    // Tracking action
    if ("production" == environment_) {
        analytics_.TrackIdleDetectionClick(db_->AnalyticsClientID(),
                                           "discard-and-continue");
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot stop time entry, user logged out");
            return nullptr;
        }
        user_->DiscardTimeAt(guid, at, false);
    }

    error err = save(true);
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    return Continue(guid);
}

TimeEntry *Context::RunningTimeEntry() {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger().warning("Cannot fetch time entry, user logged out");
        return nullptr;
    }
    return user_->RunningTimeEntry();
}

error Context::ToggleTimelineRecording(const bool record_timeline) {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        logger().warning("Cannot toggle timeline, user logged out");
        return noError;
    }
    try {
        user_->SetRecordTimeline(record_timeline);

        error err = save(false);
        if (err != noError) {
            return displayError(err);
        }

        UIElements render;
        render.display_settings = true;
        updateUI(render);

        TimelineUpdateServerSettings();

        if (user_->RecordTimeline()) {
            switchTimelineOn();
        } else {
            switchTimelineOff();
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::SetUpdateChannel(const std::string channel) {
    error err = db()->SaveUpdateChannel(channel);
    if (err != noError) {
        return displayError(err);
    }
    fetchUpdates();
    return noError;
}

void Context::SearchHelpArticles(
    const std::string keywords) {
    UI()->DisplayHelpArticles(help_database_.GetArticles(keywords));
}

error Context::SetDefaultProject(
    const Poco::UInt64 pid,
    const Poco::UInt64 tid) {
    try {
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("Cannot set default PID, user logged out");
                return noError;
            }

            Task *t = nullptr;
            if (tid) {
                t = user_->related.TaskByID(tid);
            }
            if (tid && !t) {
                return displayError("task not found");
            }

            Project *p = nullptr;
            if (pid) {
                p = user_->related.ProjectByID(pid);
            }
            if (pid && !p) {
                return displayError("project not found");
            }
            if (!p && t && t->PID()) {
                p = user_->related.ProjectByID(t->PID());
            }

            if (p && t && p->ID() != t->PID()) {
                return displayError("task does not belong to project");
            }

            if (p) {
                user_->SetDefaultPID(p->ID());
            } else {
                user_->SetDefaultPID(0);
            }

            if (t) {
                user_->SetDefaultTID(t->ID());
            } else {
                user_->SetDefaultTID(0);
            }
        }
        return displayError(save(false));
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
}

error Context::DefaultProjectName(std::string *name) {
    try {
        poco_check_ptr(name);
        Project *p = nullptr;
        Task *t = nullptr;
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("Cannot get default PID, user logged out");
                return noError;
            }
            if (user_->DefaultPID()) {
                p = user_->related.ProjectByID(user_->DefaultPID());
            }
            if (user_->DefaultTID()) {
                t = user_->related.TaskByID(user_->DefaultTID());
            }
        }
        *name = Formatter::JoinTaskName(t, p);
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::DefaultPID(Poco::UInt64 *result) {
    try {
        poco_check_ptr(result);
        *result = 0;
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("Cannot get default PID, user logged out");
                return noError;
            }
            *result = user_->DefaultPID();
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::DefaultTID(Poco::UInt64 *result) {
    try {
        poco_check_ptr(result);
        *result = 0;
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("Cannot get default PID, user logged out");
                return noError;
            }
            *result = user_->DefaultTID();
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::AddAutotrackerRule(
    const std::string term,
    const Poco::UInt64 pid,
    const Poco::UInt64 tid,
    Poco::Int64 *rule_id) {

    poco_check_ptr(rule_id);
    *rule_id = 0;

    if (term.empty()) {
        return displayError("missing term");
    }
    if (!pid && !tid) {
        return displayError("missing project and task");
    }

    std::string lowercase = Poco::UTF8::toLower(term);

    AutotrackerRule *rule = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("cannot add autotracker rule, user logged out");
            return noError;
        }
        if (user_->related.HasMatchingAutotrackerRule(lowercase)) {
            // avoid duplicates
            return displayError(kErrorRuleAlreadyExists);
        }

        Task *t = nullptr;
        if (tid) {
            t = user_->related.TaskByID(tid);
        }
        if (tid && !t) {
            return displayError("task not found");
        }

        Project *p = nullptr;
        if (pid) {
            p = user_->related.ProjectByID(pid);
        }
        if (pid && !p) {
            return displayError("project not found");
        }
        if (t && t->PID() && !p) {
            p = user_->related.ProjectByID(t->PID());
        }

        if (p && t && p->ID() != t->PID()) {
            return displayError("task does not belong to project");
        }

        rule = new AutotrackerRule();
        rule->SetTerm(lowercase);
        if (t) {
            rule->SetTID(t->ID());
        }
        if (p) {
            rule->SetPID(p->ID());
        }
        rule->SetUID(user_->ID());
        user_->related.AutotrackerRules.push_back(rule);
    }

    error err = save(false);
    if (noError != err) {
        return displayError(err);
    }

    if (rule) {
        *rule_id = rule->LocalID();
    }

    return noError;
}

error Context::DeleteAutotrackerRule(
    const Poco::Int64 id) {

    if (!id) {
        return displayError("cannot delete rule without an ID");
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("cannot delete rule, user is logged out");
            return noError;
        }

        error err = user_->related.DeleteAutotrackerRule(id);
        if (err != noError) {
            return displayError(err);
        }
    }

    return displayError(save(false));
}

Project *Context::CreateProject(
    const Poco::UInt64 workspace_id,
    const Poco::UInt64 client_id,
    const std::string client_guid,
    const std::string project_name,
    const bool is_private,
    const std::string project_color) {

    if (!workspace_id) {
        displayError(kPleaseSelectAWorkspace);
        return nullptr;
    }

    std::string trimmed_project_name("");
    error err = db_->Trim(project_name, &trimmed_project_name);
    if (err != noError) {
        displayError(err);
        return nullptr;
    }
    if (trimmed_project_name.empty()) {
        displayError(kProjectNameMustNotBeEmpty);
        return nullptr;
    }

    Project *result = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot add project, user logged out");
            return nullptr;
        }
        for (std::vector<Project *>::iterator it =
            user_->related.Projects.begin();
                it != user_->related.Projects.end(); it++) {
            Project *p = *it;

            bool clientIsSame = false;
            if (!client_guid.empty()) {
                if (!p->ClientGUID().empty()) {
                    clientIsSame = client_guid == p->ClientGUID();
                }
            } else {
                clientIsSame = client_id == p->CID();
            }

            if (clientIsSame && p->Name() == trimmed_project_name) {
                displayError(kProjectNameAlreadyExists);
                return nullptr;
            }
        }
        // Check if projects are billable by default
        Workspace *ws = nullptr;
        bool billable = false;

        ws = user_->related.WorkspaceByID(workspace_id);
        if (ws) {
            billable = ws->ProjectsBillableByDefault();
        }

        std::string client_name("");
        Poco::UInt64 cid(0);
        Client *c = nullptr;

        // Search by client ID
        if (client_id != 0) {
            c = user_->related.ClientByID(client_id);
        } else {
            // Search by Client GUID (when Client is not synced to server yet)
            c = user_->related.ClientByGUID(client_guid);
        }

        if (c) {
            client_name = c->Name();
            cid = c->ID();
        }

        result = user_->CreateProject(
            workspace_id,
            cid,
            client_guid,
            client_name,
            trimmed_project_name,
            is_private,
            project_color,
            billable);
    }

    err = save(false);
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    return result;
}

error Context::AddObmAction(
    const Poco::UInt64 experiment_id,
    const std::string key,
    const std::string value) {
    // Check input
    if (!experiment_id) {
        return error("missing experiment_id");
    }
    std::string trimmed_key("");
    error err = db_->Trim(key, &trimmed_key);
    if (err != noError) {
        return displayError(err);
    }
    if (trimmed_key.empty()) {
        return error("missing key");
    }
    std::string trimmed_value("");
    err = db_->Trim(value, &trimmed_value);
    if (err != noError) {
        return displayError(err);
    }
    if (trimmed_value.empty()) {
        return error("missing value");
    }
    // Add OBM action and save
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot create a OBM action, user logged out");
            return noError;
        }
        ObmAction *action = new ObmAction();
        action->SetExperimentID(experiment_id);
        action->SetUID(user_->ID());
        action->SetKey(trimmed_key);
        action->SetValue(trimmed_value);
        user_->related.ObmActions.push_back(action);
    }
    return displayError(save(false));
}

Client *Context::CreateClient(
    const Poco::UInt64 workspace_id,
    const std::string client_name) {

    if (!workspace_id) {
        displayError(kPleaseSelectAWorkspace);
        return nullptr;
    }

    std::string trimmed_client_name("");
    error err = db_->Trim(client_name, &trimmed_client_name);
    if (err != noError) {
        displayError(err);
        return nullptr;
    }
    if (trimmed_client_name.empty()) {
        displayError(kClientNameMustNotBeEmpty);
        return nullptr;
    }

    Client *result = nullptr;

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("Cannot create a client, user logged out");
            return nullptr;
        }
        for (std::vector<Client *>::iterator it =
            user_->related.Clients.begin();
                it != user_->related.Clients.end(); it++) {
            Client *c = *it;
            if (c->WID() == workspace_id && c->Name() == trimmed_client_name) {
                displayError(kClientNameAlreadyExists);
                return nullptr;
            }
        }
        result = user_->CreateClient(workspace_id, trimmed_client_name);
    }

    err = save(false);
    if (err != noError) {
        displayError(err);
        return nullptr;
    }

    return result;
}

void Context::SetSleep() {

    // Stop running entry if need
    const bool isHandled = handleStopRunningEntry();

    // Set Sleep as usual
    if (!isHandled) {
        logger().debug("SetSleep");
        idle_.SetSleep();
        window_change_recorder_->SetIsSleeping(true);
    }
}

error Context::AsyncOpenReportsInBrowser() {
    std::thread backgroundThread([&]() {
        return this->OpenReportsInBrowser();
    });
    backgroundThread.detach();
    return noError;
}

error Context::OpenReportsInBrowser() {
    // Do not even allow to open reports
    // else they will linger around in the app
    // and the user can continue using the unsupported app.
    if (urls::ImATeapot()) {
        return displayError(kUnsupportedAppError);
    }

    std::string apitoken("");

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return displayError("You must log in to view reports");
        }
        apitoken = user_->APIToken();
    }

    // Not implemented in v9 as of 12.05.2017
    HTTPSRequest req;
    req.host = urls::API();
    req.relative_url = "/api/v8/desktop_login_tokens";
    req.payload = "{}";
    req.basic_auth_username = apitoken;
    req.basic_auth_password = "api_token";

    TogglClient client(UI());
    HTTPSResponse resp = client.Post(req);
    if (resp.err != noError) {
        return displayError(resp.err);
    }
    if (resp.body.empty()) {
        return displayError("Unexpected empty response from API");
    }

    std::string login_token("");
    error err = User::LoginToken(resp.body, &login_token);
    if (err != noError) {
        return displayError(err);
    }

    if (login_token.empty()) {
        return displayError("Could not extract login token from JSON");
    }

    // Not implemented in v9 as of 12.05.2017
    std::stringstream ss;
    ss  << urls::API() << "/api/v8/desktop_login"
        << "?login_token=" << login_token
        << "&goto=reports";
    UI()->DisplayURL(ss.str());

    return noError;
}

error Context::offerBetaChannel(bool *did_offer) {
    try {
        poco_check_ptr(did_offer);

        *did_offer = false;

        if (update_check_disabled_) {
            // if update check is disabled, then
            // the channel selection won't be ever
            // used anyway
            return noError;
        }

        if (settings_.has_seen_beta_offering) {
            return noError;
        }

        if (!UI()->CanDisplayPromotion()) {
            return noError;
        }

        std::string update_channel("");
        error err = db()->LoadUpdateChannel(&update_channel);
        if (err != noError) {
            return err;
        }

        if ("beta" == update_channel) {
            return noError;
        }

        UI()->DisplayPromotion(kPromotionJoinBetaChannel);

        err = db()->SetSettingsHasSeenBetaOffering(true);
        if (err != noError) {
            return err;
        }

        UIElements render;
        render.display_settings = true;
        updateUI(render);

        *did_offer = true;
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::runObmExperiments() {
    try {
        // Collect OBM experiments
        std::map<Poco::UInt64, ObmExperiment> experiments;
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                logger().warning("User logged out, cannot OBM experiment");
                return noError;
            }
            for (std::vector<ObmExperiment *>::const_iterator it =
                user_->related.ObmExperiments.begin();
                    it != user_->related.ObmExperiments.end();
                    it++) {
                ObmExperiment *model = *it;
                if (!model->DeletedAt()) {
                    experiments[model->Nr()] = *model;
                    model->SetHasSeen(true);
                }
            }
        }
        // Save the (seen/unseen) state
        error err = save(false);
        if (err != noError) {
            return err;
        }
        // Now pass the experiments on to UI
        for (std::map<Poco::UInt64, ObmExperiment>::const_iterator
                it = experiments.begin();
                it != experiments.end();
                it++) {
            ObmExperiment experiment = it->second;
            UI()->DisplayObmExperiment(
                experiment.Nr(),
                experiment.Included(),
                experiment.HasSeen());
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

void Context::SetWake() {
    logger().debug("SetWake");

    Poco::Timestamp::TimeDiff delay = 0;
    if (next_wake_at_ > 0) {
        delay = kRequestThrottleSeconds * kOneSecondInMicros;
    }

    next_wake_at_ = postpone(delay);
    Poco::Util::TimerTask::Ptr ptask =
        new Poco::Util::TimerTaskAdapter<Context>(*this, &Context::onWake);

    Poco::Mutex::ScopedLock lock(timer_m_);
    timer_.schedule(ptask, next_wake_at_);

    std::stringstream ss;
    ss << "Next wake at "
       << Formatter::Format8601(next_wake_at_);
    logger().debug(ss.str());
}

void Context::onWake(Poco::Util::TimerTask&) {  // NOLINT
    if (isPostponed(next_wake_at_,
                    kRequestThrottleSeconds * kOneSecondInMicros)) {
        logger().debug("onWake postponed");
        return;
    }
    logger().debug("onWake executing");

    try {
        Poco::LocalDateTime now;
        if (now.year() != last_time_entry_list_render_at_.year()
                || now.month() != last_time_entry_list_render_at_.month()
                || now.day() != last_time_entry_list_render_at_.day()) {
            UIElements render;
            render.display_time_entries = true;
            updateUI(render);
        }

        idle_.SetWake(user_);
        window_change_recorder_->SetIsSleeping(false);

        Sync();
    }
    catch (const Poco::Exception& exc) {
        logger().error(exc.displayText());
    }
    catch (const std::exception& ex) {
        logger().error(ex.what());
    }
    catch (const std::string& ex) {
        logger().error(ex);
    }
}

void Context::SetLocked() {
    logger().debug("SetLocked");
    window_change_recorder_->SetIsLocked(true);
}

void Context::SetUnlocked() {
    logger().debug("SetUnlocked");
    window_change_recorder_->SetIsLocked(false);
}

void Context::SetOnline() {
    logger().debug("SetOnline");
    Sync();
}

void Context::osShutdown() {
    handleStopRunningEntry();
}

const bool Context::handleStopRunningEntry() {

    // Skip if this feature is not enable
    if (!settings_.stop_entry_on_shutdown_sleep) {
        return false;
    }

    // Stop running entry
    return Stop(false) == noError;
}

void Context::displayReminder() {
    if (!settings_.reminder) {
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return;
        }

        if (user_ && user_->RunningTimeEntry()) {
            return;
        }

        if (time(nullptr) - last_tracking_reminder_time_
                < settings_.reminder_minutes * 60) {
            return;
        }
    }

    // Check if allowed to display reminder on this weekday
    Poco::LocalDateTime now;
    int wday = now.dayOfWeek();
    if (
        (Poco::DateTime::MONDAY == wday && !settings_.remind_mon) ||
        (Poco::DateTime::TUESDAY == wday && !settings_.remind_tue) ||
        (Poco::DateTime::WEDNESDAY == wday && !settings_.remind_wed) ||
        (Poco::DateTime::THURSDAY == wday && !settings_.remind_thu) ||
        (Poco::DateTime::FRIDAY == wday && !settings_.remind_fri) ||
        (Poco::DateTime::SATURDAY == wday && !settings_.remind_sat) ||
        (Poco::DateTime::SUNDAY == wday && !settings_.remind_sun)) {
        logger().debug("reminder is not enabled on this weekday");
        return;
    }

    // Check if allowed to display reminder at this time
    if (!settings_.remind_starts.empty()) {
        int h(0), m(0);
        if (toggl::Formatter::ParseTimeInput(settings_.remind_starts, &h, &m)) {
            Poco::LocalDateTime start(
                now.year(), now.month(), now.day(), h, m, now.second());
            if (now < start) {
                std::stringstream ss;
                ss << "Reminder - its too early for reminders"
                   << " [" << now.hour() << ":" << now.minute() << "]"
                   << " (allowed from " << h << ":" << m << ")";
                logger().debug(ss.str());
                return;
            }
        }
    }
    if (!settings_.remind_ends.empty()) {
        int h(0), m(0);
        if (toggl::Formatter::ParseTimeInput(settings_.remind_ends, &h, &m)) {
            Poco::LocalDateTime end(
                now.year(), now.month(), now.day(), h, m, now.second());
            if (now > end) {
                std::stringstream ss;
                ss << "Reminder - its too late for reminders"
                   << " [" << now.hour() << ":" << now.minute() << "]"
                   << " (allowed until " << h << ":" << m << ")";
                logger().debug(ss.str());
                return;
            }
        }
    }

    resetLastTrackingReminderTime();

    UI()->DisplayReminder();
}

void Context::resetLastTrackingReminderTime() {
    last_tracking_reminder_time_ = time(nullptr);
}

void Context::displayPomodoro() {
    if (!settings_.pomodoro) {
        return;
    }

    Poco::UInt64 wid(0);

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return;
        }

        TimeEntry *current_te = user_->RunningTimeEntry();
        if (!current_te) {
            return;
        }
        if (pomodoro_break_entry_ != nullptr
                && !current_te->GUID().empty()
                && current_te->GUID().compare(
                    pomodoro_break_entry_->GUID()) == 0) {
            return;
        }

        if (current_te->DurOnly() && current_te->LastStartAt() != 0) {
            if (time(nullptr) - current_te->LastStartAt()
                    < settings_.pomodoro_minutes * 60) {
                return;
            }
        } else {
            if (time(nullptr) - current_te->Start()
                    < settings_.pomodoro_minutes * 60) {
                return;
            }
        }
        const Poco::Int64 pomodoroDuration = settings_.pomodoro_minutes * 60;
        wid = current_te->WID();
        Stop(true);
        current_te->SetDurationInSeconds(pomodoroDuration);
        current_te->SetStop(current_te->Start() + pomodoroDuration);
    }
    UI()->DisplayPomodoro(settings_.pomodoro_minutes);

    if (settings_.pomodoro_break) {
        //  Start a new task with the tag "pomodoro-break"
        pomodoro_break_entry_ = user_->Start("Pomodoro Break",  // description
                                             "",  // duration
                                             0,  // task_id
                                             0,  // project_id
                                             "",  // project_guid
                                             "pomodoro-break");  // tags

        // Set workspace id to same as the previous entry
        pomodoro_break_entry_->SetWID(wid);
    }
}

void Context::displayPomodoroBreak() {
    if (!settings_.pomodoro_break) {
        return;
    }

    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return;
        }

        TimeEntry *current_te = user_->RunningTimeEntry();

        if (!current_te) {
            return;
        }

        if (pomodoro_break_entry_ == nullptr
                || current_te->GUID().compare(
                    pomodoro_break_entry_->GUID()) != 0) {
            return;
        }

        if (time(nullptr) - current_te->Start()
                < settings_.pomodoro_break_minutes * 60) {
            return;
        }
        const Poco::Int64 pomodoroDuration = settings_.pomodoro_break_minutes * 60;
        Stop(true);
        current_te->SetDurationInSeconds(pomodoroDuration);
        current_te->SetStop(current_te->Start() + pomodoroDuration);
    }
    pomodoro_break_entry_ = nullptr;

    UI()->DisplayPomodoroBreak(settings_.pomodoro_break_minutes);
}

error Context::StartAutotrackerEvent(const TimelineEvent event) {
    Poco::Mutex::ScopedLock lock(user_m_);
    if (!user_) {
        return noError;
    }

    if (!settings_.autotrack) {
        return noError;
    }

    // Notify user to track using autotracker rules:
    if (user_ && user_->RunningTimeEntry()) {
        return noError;
    }
    AutotrackerRule *rule = user_->related.FindAutotrackerRule(event);
    if (!rule) {
        return noError;
    }

    Project *p = nullptr;
    if (rule->PID()) {
        p = user_->related.ProjectByID(rule->PID());
    }
    if (rule->PID() && !p) {
        return error("autotracker project not found");
    }

    Task *t = nullptr;
    if (rule->TID()) {
        t = user_->related.TaskByID(rule->TID());
    }
    if (rule->TID() && !t) {
        return error("autotracker task not found");
    }

    if (!p && !t) {
        return error("no project or task specified in autotracker rule");
    }

    UI()->DisplayAutotrackerNotification(p, t);

    return noError;
}

error Context::CreateCompressedTimelineBatchForUpload(TimelineBatch *batch) {
    try {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("cannot create timeline batch, user logged out");
            return noError;
        }

        poco_check_ptr(batch);

        if (quit_) {
            return noError;
        }

        user_->CompressTimeline();
        error err = save(false);
        if (err != noError) {
            return displayError(err);
        }

        batch->SetEvents(user_->CompressedTimeline());
        batch->SetUserID(user_->ID());
        batch->SetAPIToken(user_->APIToken());
        batch->SetDesktopID(db_->DesktopID());
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::StartTimelineEvent(TimelineEvent *event) {
    try {
        poco_check_ptr(event);

        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            return noError;
        }

        if (user_ && user_->RecordTimeline()) {
            event->SetUID(static_cast<unsigned int>(user_->ID()));
            user_->related.TimelineEvents.push_back(event);
            return displayError(save(false));
        }
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
    return noError;
}

error Context::MarkTimelineBatchAsUploaded(
    const std::vector<TimelineEvent> &events) {
    try {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_) {
            logger().warning("cannot mark timeline events as uploaded, "
                             "user is already logged out");
            return noError;
        }
        user_->MarkTimelineBatchAsUploaded(events);
        return displayError(save(false));
    } catch(const Poco::Exception& exc) {
        return displayError(exc.displayText());
    } catch(const std::exception& ex) {
        return displayError(ex.what());
    } catch(const std::string& ex) {
        return displayError(ex);
    }
}

error Context::SetPromotionResponse(
    const int64_t promotion_type,
    const int64_t promotion_response) {

    if (kPromotionJoinBetaChannel == promotion_type && promotion_response) {
        return SetUpdateChannel("beta");
    }

    return noError;
}

void Context::uiUpdaterActivity() {
    std::string running_time("");
    while (!ui_updater_.isStopped()) {
        // Sleep in increments for faster shutdown.
        for (unsigned int i = 0; i < 4*10; i++) {
            if (ui_updater_.isStopped()) {
                return;
            }
            Poco::Thread::sleep(250);
        }

        TimeEntry *te = nullptr;
        Poco::Int64 duration(0);
        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_) {
                continue;
            }
            te = user_->RunningTimeEntry();
            if (!te) {
                continue;
            }
            duration = user_->related.TotalDurationForDate(te);
        }

        std::string date_duration =
            Formatter::FormatDurationForDateHeader(duration);

        if (running_time != date_duration) {
            UIElements render;
            render.display_time_entries = true;
            updateUI(render);
        }

        running_time = date_duration;
    }
}

void Context::checkReminders() {
    displayReminder();
    displayPomodoro();
    displayPomodoroBreak();
}

void Context::reminderActivity() {
    while (true) {
        // Sleep in increments for faster shutdown.
        for (int i = 0; i < 4; i++) {
            if (reminder_.isStopped()) {
                return;
            }
            Poco::Thread::sleep(250);
        }

        checkReminders();
    }
}

void Context::syncerActivity() {
    while (true) {
        // Sleep in increments for faster shutdown.
        for (int i = 0; i < 4; i++) {
            if (syncer_.isStopped()) {
                return;
            }
            Poco::Thread::sleep(250);
        }

        {
            Poco::Mutex::ScopedLock lock(syncer_m_);

            if (trigger_sync_) {
                TogglClient client(UI());

                error err = pullAllUserData(&client);
                if (err != noError) {
                    displayError(err);
                }

                setOnline("Data pulled");

                err = pushChanges(&client, &trigger_sync_);
                trigger_push_ = false;
                if (err != noError) {
                    user_->ConfirmLoadedMore();
                    displayError(err);
                    return;
                } else {
                    setOnline("Data pushed");
                }
                trigger_sync_ = false;

                // Push cached OBM action
                err = pushObmAction();
                if (err != noError) {
                    std::cout << "SYNC: sync-pushObm ERROR\n";
                    logger().error("Error pushing OBM action: " + err);
                }

                displayError(save(false));
            }

        }

        {
            Poco::Mutex::ScopedLock lock(syncer_m_);

            if (trigger_push_) {
                TogglClient client(UI());

                error err = pushChanges(&client, &trigger_sync_);
                if (err != noError) {
                    user_->ConfirmLoadedMore();
                    displayError(err);
                } else {
                    setOnline("Data pushed");
                }
                trigger_push_ = false;

                // Push cached OBM action
                err = pushObmAction();
                if (err != noError) {
                    std::cout << "SYNC: pushObm ERROR\n";
                    logger().error("Error pushing OBM action: " + err);
                }

                displayError(save(false));
            }
        }
    }
}

void Context::LoadMore() {
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_ || user_->HasLoadedMore()) {
            return;
        }
    }
    {
        Poco::Util::TimerTask::Ptr task =
            new Poco::Util::TimerTaskAdapter<Context>(*this,
                    &Context::onLoadMore);
        Poco::Mutex::ScopedLock lock(timer_m_);
        timer_.schedule(task, postpone(0));
    }
}

void Context::onLoadMore(Poco::Util::TimerTask&) {
    bool needs_render = !user_->HasLoadedMore();
    std::string api_token;
    {
        Poco::Mutex::ScopedLock lock(user_m_);
        if (!user_ || user_->HasLoadedMore()) {
            return;
        }
        api_token = user_->APIToken();
    }

    if (api_token.empty()) {
        return logger().warning(
            "cannot load more time entries without API token");
    }

    try {
        std::stringstream ss;
        ss << "/api/v9/me/time_entries?since="
           << (Poco::Timestamp() - Poco::Timespan(60, 0, 0, 0, 0)).epochTime();

        std::stringstream l;
        l << "loading more: " << ss.str();
        logger().debug(l.str());

        TogglClient client(UI());
        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = api_token;
        req.basic_auth_password = "api_token";

        HTTPSResponse resp = client.Get(req);
        if (resp.err != noError) {
            logger().warning(resp.err);
            return;
        }

        std::string json = resp.body;

        {
            Poco::Mutex::ScopedLock lock(user_m_);
            if (!user_)
                return;
            error err = user_->LoadTimeEntriesFromJSONString(json);

            if (err != noError) {
                logger().error(err);
                return;
            }

            user_->ConfirmLoadedMore();

            // Removes load more button if nothing is to be loaded
            if (needs_render) {
                UIElements render;
                render.display_time_entries = true;
                updateUI(render);
            }
        }

        displayError(save(false));
    }
    catch (const Poco::Exception& exc) {
        logger().warning(exc.displayText());
    }
    catch (const std::exception& ex) {
        logger().warning(ex.what());
    }
    catch (const std::string& ex) {
        logger().warning(ex);
    }
}



void Context::SetLogPath(const std::string path) {
    Poco::AutoPtr<Poco::SimpleFileChannel> simpleFileChannel(
        new Poco::SimpleFileChannel);
    simpleFileChannel->setProperty(
        Poco::SimpleFileChannel::PROP_PATH, path);
    simpleFileChannel->setProperty(
        Poco::SimpleFileChannel::PROP_ROTATION, "1 M");
    simpleFileChannel->setProperty(
        Poco::SimpleFileChannel::PROP_FLUSH, "false");

    Poco::AutoPtr<Poco::FormattingChannel> formattingChannel(
        new Poco::FormattingChannel(
            new Poco::PatternFormatter(
                "%Y-%m-%d %H:%M:%S.%i [%P %I]:%s:%q:%t")));
    formattingChannel->setChannel(simpleFileChannel);

    Poco::Logger::get("").setChannel(formattingChannel);

    log_path_ = path;
}

Poco::Logger &Context::logger() const {
    return Poco::Logger::get("context");
}


error Context::me(
    TogglClient *toggl_client,
    const std::string email,
    const std::string password,
    std::string *user_data_json,
    const Poco::Int64 since) {

    if (email.empty()) {
        return "Empty email or API token";
    }

    if (password.empty()) {
        return "Empty password";
    }

    try {
        poco_check_ptr(user_data_json);
        poco_check_ptr(toggl_client);

        std::stringstream ss;
        ss << "/api/v8/me"
           << "?app_name=" << TogglClient::Config.AppName
           << "&with_related_data=true";
        if (since) {
            ss << "&since=" << since;
        }

        HTTPSRequest req;
        req.host = urls::API();
        req.relative_url = ss.str();
        req.basic_auth_username = email;
        req.basic_auth_password = password;

        HTTPSResponse resp = toggl_client->Get(req);
        if (resp.err != noError) {
            return resp.err;
        }

        *user_data_json = resp.body;
    } catch(const Poco::Exception& exc) {
        return exc.displayText();
    } catch(const std::exception& ex) {
        return ex.what();
    } catch(const std::string& ex) {
        return ex;
    }
    return noError;
}

error Context::logAndDisplayUserTriedEditingLockedEntry() {
    logger().warning("User tried editing locked time entry");
    return displayError(error("Cannot change locked time entry"));
}


error Context::ToggleEntriesGroup(std::string name) {
    entry_groups[name] = !entry_groups[name];
    OpenTimeEntryList();
    return noError;
}

void on_websocket_message(
    void *context,
    std::string json) {

    poco_check_ptr(context);

    if (json.empty()) {
        return;
    }

    Context *ctx = reinterpret_cast<Context *>(context);
    ctx->LoadUpdateFromJSONString(json);
}

void Context::TrackWindowSize(const Poco::UInt64 width,
                              const Poco::UInt64 height) {
    if ("production" == environment_) {
        analytics_.TrackWindowSize(db_->AnalyticsClientID(),
                                   shortOSName(),
                                   toggl::Rectangle(width, height));
    }
}

void Context::TrackEditSize(const Poco::Int64 width,
                            const Poco::Int64 height) {
    if ("production" == environment_) {
        analytics_.TrackEditSize(db_->AnalyticsClientID(),
                                 shortOSName(),
                                 toggl::Rectangle(width, height));
    }
}

}  // namespace toggl
