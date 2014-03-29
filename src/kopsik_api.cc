// Copyright 2014 Toggl Desktop developers.

// No exceptions should be thrown from this library.

#include <cstring>
#include <set>

#include "./kopsik_api.h"
#include "./kopsik_api_private.h"
#include "./database.h"
#include "./user.h"
#include "./https_client.h"
#include "./websocket_client.h"
#include "./version.h"
#include "./timeline_uploader.h"
#include "./window_change_recorder.h"
#include "./CustomErrorHandler.h"
#include "./proxy.h"
#include "./context.h"
#include "./formatter.h"
#include "./feedback.h"

#include "Poco/Bugcheck.h"
#include "Poco/Path.h"
#include "Poco/Logger.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"

inline Poco::Logger &logger() {
    return Poco::Logger::get("kopsik_api");
}

inline Poco::Logger &rootLogger() {
    return Poco::Logger::get("");
}

inline kopsik::Context *app(void *context) {
    poco_assert(context);
    return reinterpret_cast<kopsik::Context *>(context);
}

int kopsik_is_networking_error(
    const char *error) {
    std::string value(error);
    if (value.find("Host not found") != std::string::npos) {
        return 1;
    }
    if (value.find("Cannot upgrade to WebSocket connection")
            != std::string::npos) { // NOLINT
        return 1;
    }
    if (value.find("No message received") != std::string::npos) {
        return 1;
    }
    if (value.find("Connection refused") != std::string::npos) {
        return 1;
    }
    if (value.find("Connection timed out") != std::string::npos) {
        return 1;
    }
    if (value.find("connect timed out") != std::string::npos) {
        return 1;
    }
    if (value.find("SSL connection unexpectedly closed") != std::string::npos) {
        return 1;
    }
    if (value.find("Network is down") != std::string::npos) {
        return 1;
    }
    if (value.find("Network is unreachable") != std::string::npos) {
        return 1;
    }
    if (value.find("Host is down") != std::string::npos) {
        return 1;
    }
    return 0;
}

// Generic view items

void kopsik_view_item_clear(
    KopsikViewItem *item) {
    if (!item) {
        return;
    }
    if (item->Name) {
        free(item->Name);
        item->Name = 0;
    }
    if (item->GUID) {
        free(item->GUID);
        item->GUID = 0;
    }
    if (item->Next) {
        KopsikViewItem *next = reinterpret_cast<KopsikViewItem *>(item->Next);
        kopsik_view_item_clear(next);
    }
    delete item;
    item = 0;
}

// Context API.

KopsikViewItemChangeCallback user_data_change_callback_ = 0;

void export_on_change_callback(
    const kopsik::ModelChange mc) {
    poco_assert(user_data_change_callback_);

    KopsikModelChange *change = model_change_init();
    model_change_to_change_item(mc, change);
    user_data_change_callback_(KOPSIK_API_SUCCESS, 0, change);
    model_change_clear(change);
}

KopsikErrorCallback user_data_error_callback_ = 0;

void export_on_error_callback(
    const kopsik::error err) {
    poco_assert(user_data_error_callback_);

    user_data_error_callback_(err.c_str());
}

KopsikCheckUpdateCallback user_data_check_updates_callback_ = 0;

void export_on_check_update_callback(
    const bool is_update_available,
    const std::string url,
    const std::string version) {
    poco_assert(user_data_check_updates_callback_);

    unsigned int avail = 0;
    if (is_update_available) {
        avail = 1;
    }
    user_data_check_updates_callback_(avail, url.c_str(), version.c_str());
}

void *kopsik_context_init(
    const char *app_name,
    const char *app_version,
    KopsikViewItemChangeCallback change_callback,
    KopsikErrorCallback error_callback,
    KopsikCheckUpdateCallback check_updates_callback,
    KopsikOnOnlineCallback online_callback) {
    poco_assert(app_name);
    poco_assert(app_version);

    kopsik::Context *ctx =
        new kopsik::Context(std::string(app_name), std::string(app_version));

    user_data_change_callback_ = change_callback;
    ctx->SetModelChangeCallback(export_on_change_callback);

    user_data_error_callback_ = error_callback;
    ctx->SetOnErrorCallback(export_on_error_callback);

    user_data_check_updates_callback_ = check_updates_callback;
    ctx->SetCheckUpdateCallback(export_on_check_update_callback);

    ctx->SetOnOnlineCallback(online_callback);

    ctx->SetAPIURL(kAPIURL);
    ctx->SetTimelineUploadURL(kTimelineUploadURL);
    ctx->SetWebSocketClientURL(kWebSocketURL);

    return ctx;
}

void kopsik_context_shutdown(void *context) {
    app(context)->Shutdown();
}

void kopsik_context_clear(void *context) {
    delete app(context);
}

// Configuration API.

kopsik_api_result kopsik_get_settings(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    unsigned int *out_use_idle_detection,
    unsigned int *out_menubar_timer,
    unsigned int *out_dock_icon) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(out_use_idle_detection);
        poco_assert(out_menubar_timer);
        poco_assert(out_dock_icon);

        bool use_idle_detection(false);
        bool menubar_timer(false);
        bool dock_icon(false);

        kopsik::error err = app(context)->LoadSettings(&use_idle_detection,
                            &menubar_timer,
                            &dock_icon);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }

        *out_use_idle_detection = 0;
        if (use_idle_detection) {
            *out_use_idle_detection = 1;
        }

        *out_menubar_timer = 0;
        if (menubar_timer) {
            *out_menubar_timer = 1;
        }

        *out_dock_icon = 0;
        if (dock_icon) {
            *out_dock_icon = 1;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_get_proxy_settings(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    unsigned int *out_use_proxy,
    char **out_proxy_host,
    unsigned int *out_proxy_port,
    char **out_proxy_username,
    char **out_proxy_password) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(out_use_proxy);
        poco_assert(out_proxy_host);
        poco_assert(out_proxy_port);
        poco_assert(out_proxy_username);
        poco_assert(out_proxy_password);

        bool use_proxy(false);
        kopsik::Proxy proxy;
        kopsik::error err = app(context)->LoadProxySettings(
            &use_proxy,
            &proxy);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }

        *out_use_proxy = 0;
        if (use_proxy) {
            *out_use_proxy = 1;
        }

        *out_proxy_host = strdup(proxy.host.c_str());
        *out_proxy_port = proxy.port;
        *out_proxy_username = strdup(proxy.username.c_str());
        *out_proxy_password = strdup(proxy.password.c_str());
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_settings(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const unsigned int use_idle_detection,
    const unsigned int menubar_timer,
    const unsigned int dock_icon) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);

        kopsik::error err = app(context)->SaveSettings(
            use_idle_detection,
            menubar_timer,
            dock_icon);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_proxy_settings(void *context,
        char *errmsg,
        const unsigned int errlen,
        const unsigned int use_proxy,
        const char *proxy_host,
        const unsigned int proxy_port,
        const char *proxy_username,
        const char *proxy_password) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(proxy_host);
        poco_assert(proxy_username);
        poco_assert(proxy_password);

        kopsik::Proxy proxy;
        proxy.host = std::string(proxy_host);
        proxy.port = proxy_port;
        proxy.username = std::string(proxy_username);
        proxy.password = std::string(proxy_password);

        kopsik::error err = app(context)->SaveProxySettings(use_proxy, &proxy);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_configure_proxy(
    void *context,
    char *errmsg,
    const unsigned int errlen) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);

        kopsik::error err = app(context)->ConfigureProxy();
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_db_path(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *path) {
    kopsik::error err = kopsik::noError;
    try {
        poco_assert(path);

        std::stringstream ss;
        ss << "kopsik_set_db_path path=" << path;
        logger().debug(ss.str());

        app(context)->SetDBPath(std::string(path));
    } catch(const Poco::Exception& exc) {
        err = exc.displayText();
    } catch(const std::exception& ex) {
        err = ex.what();
    } catch(const std::string& ex) {
        err = ex;
    }
    if (err != kopsik::noError) {
        strncpy(errmsg, err.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

void kopsik_set_log_path(const char *path) {
    poco_assert(path);

    Poco::AutoPtr<Poco::SimpleFileChannel> simpleFileChannel(
        new Poco::SimpleFileChannel);
    simpleFileChannel->setProperty("path", path);
    simpleFileChannel->setProperty("rotation", "1 M");

    Poco::AutoPtr<Poco::FormattingChannel> formattingChannel(
        new Poco::FormattingChannel(
            new Poco::PatternFormatter(
                "%Y-%m-%d %H:%M:%S.%i [%P %I]:%s:%q:%t")));
    formattingChannel->setChannel(simpleFileChannel);

    rootLogger().setChannel(formattingChannel);
    rootLogger().setLevel(Poco::Message::PRIO_DEBUG);
}

void kopsik_set_log_level(const char *level) {
    poco_assert(level);

    rootLogger().setLevel(level);
}

void kopsik_set_api_url(
    void *context,
    const char *api_url) {
    poco_assert(api_url);

    app(context)->SetAPIURL(std::string(api_url));
}

void kopsik_set_websocket_url(
    void *context,
    const char *websocket_url) {
    poco_assert(websocket_url);

    app(context)->SetWebSocketClientURL(websocket_url);
}

// User API.

KopsikUser *kopsik_user_init() {
    KopsikUser *user = new KopsikUser();
    user->ID = 0;
    user->Fullname = 0;
    return user;
}

void kopsik_user_clear(
    KopsikUser *user) {
    poco_assert(user);
    user->ID = 0;
    if (user->Fullname) {
        free(user->Fullname);
        user->Fullname = 0;
    }
    delete user;
}

kopsik_api_result kopsik_current_user(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    KopsikUser *out_user) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(out_user);

        logger().debug("kopsik_current_user");

        kopsik::User *current_user = 0;
        kopsik::error err = app(context)->CurrentUser(&current_user);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }

        if (!current_user) {
            return KOPSIK_API_SUCCESS;
        }
        if (out_user->Fullname) {
            free(out_user->Fullname);
            out_user->Fullname = 0;
        }
        out_user->Fullname = strdup(current_user->Fullname().c_str());
        out_user->ID = (unsigned int)current_user->ID();
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_api_token(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *api_token) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(api_token);

        std::stringstream ss;
        ss << "kopsik_set_api_token api_token=" << api_token;
        logger().debug(ss.str());

        kopsik::error err = app(context)->SetCurrentAPIToken(api_token);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_get_api_token(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    char *str,
    const unsigned int max_strlen) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(str);
        poco_assert(max_strlen);

        std::string token("");
        kopsik::error err = app(context)->CurrentAPIToken(&token);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
        strncpy(str, token.c_str(), max_strlen);
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_logged_in_user(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *json) {
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(json);

    try {
        logger().debug("kopsik_set_logged_in_user");

        kopsik::error err =
            app(context)->SetLoggedInUserFromJSON(std::string(json));
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_login(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *in_email,
    const char *in_password) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(in_email);
        poco_assert(in_password);

        std::stringstream ss;
        ss << "kopik_login email=" << in_email;
        logger().debug(ss.str());

        std::string email(in_email);
        std::string password(in_password);
        if (email.empty()) {
            strncpy(errmsg, "Empty email", errlen);
            return KOPSIK_API_FAILURE;
        }
        if (password.empty()) {
            strncpy(errmsg, "Empty password", errlen);
            return KOPSIK_API_FAILURE;
        }

        kopsik::error err = app(context)->Login(email, password);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_logout(
    void *context,
    char *errmsg,
    const unsigned int errlen) {
    poco_assert(errmsg);
    poco_assert(errlen);

    logger().debug("kopsik_logout");

    kopsik::error err = app(context)->Logout();
    if (err != kopsik::noError) {
        strncpy(errmsg, err.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }

    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_clear_cache(
    void *context,
    char *errmsg,
    const unsigned int errlen) {
    poco_assert(errmsg);
    poco_assert(errlen);

    logger().debug("kopsik_clear_cache");

    kopsik::error err = app(context)->ClearCache();
    if (err != kopsik::noError) {
        strncpy(errmsg, err.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_user_can_see_billable_flag(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *guid,
    unsigned int *can_see) {

    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(can_see);
        poco_assert(guid);

        *can_see = 0;
        if (app(context)->CanSeeBillable(std::string(guid))) {
            *can_see = 1;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_user_can_add_projects(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const unsigned int workspace_id,
    unsigned int *can_add) {

    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(can_add);

        *can_add = 0;
        if (app(context)->CanAddProjects(workspace_id)) {
            *can_add = 1;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_user_is_logged_in(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    unsigned int *is_logged_in) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(is_logged_in);

        *is_logged_in = 0;
        if (app(context)->UserIsLoggedIn()) {
            *is_logged_in = 1;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_users_default_wid(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    unsigned int *default_wid) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(default_wid);

        *default_wid =
            static_cast<unsigned int>(app(context)->UsersDefaultWID());
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

// Sync

kopsik_api_result kopsik_pushable_models(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    KopsikPushableModelStats *stats) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(stats);

        logger().debug("kopsik_pushable_models");

        std::vector<kopsik::TimeEntry *> pushable;
        app(context)->CollectPushableTimeEntries(&pushable);

        stats->TimeEntries = 0;
        for (std::vector<kopsik::TimeEntry *>::const_iterator it =
            pushable.begin();
                it != pushable.end();
                it++) {
            stats->TimeEntries++;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

void kopsik_sync(void *context) {
    logger().debug("kopsik_sync");
    app(context)->FullSync();
}

void kopsik_autocomplete_item_clear(
    KopsikAutocompleteItem *item) {
    if (!item) {
        return;
    }
    if (item->Text) {
        free(item->Text);
        item->Text = 0;
    }
    if (item->ProjectAndTaskLabel) {
        free(item->ProjectAndTaskLabel);
        item->ProjectAndTaskLabel = 0;
    }
    if (item->Description) {
        free(item->Description);
        item->Description = 0;
    }
    if (item->ProjectColor) {
        free(item->ProjectColor);
        item->ProjectColor = 0;
    }
    if (item->Next) {
        KopsikAutocompleteItem *next =
            reinterpret_cast<KopsikAutocompleteItem *>(item->Next);
        kopsik_autocomplete_item_clear(next);
        item->Next = 0;
    }
    delete item;
}

kopsik_api_result kopsik_autocomplete_items(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    KopsikAutocompleteItem **first,
    const unsigned int include_time_entries,
    const unsigned int include_tasks,
    const unsigned int include_projects) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(first);

        logger().debug("kopsik_autocomplete_items");

        *first = 0;

        std::vector<kopsik::AutocompleteItem> items;
        app(context)->AutocompleteItems(&items,
                                        include_time_entries,
                                        include_tasks,
                                        include_projects);

        KopsikAutocompleteItem *previous = 0;
        for (std::vector<kopsik::AutocompleteItem>::iterator it =
            items.begin();
                it != items.end();
                it++) {
            kopsik::AutocompleteItem &item = *it;

            KopsikAutocompleteItem *autocomplete_item =
                autocomplete_item_init();
            if (!*first) {
                *first = autocomplete_item;
            }
            if (previous) {
                previous->Next = autocomplete_item;
            }

            autocomplete_item->Description = strdup(item.Description.c_str());
            autocomplete_item->Text = strdup(item.Text.c_str());
            autocomplete_item->ProjectAndTaskLabel =
                strdup(item.ProjectAndTaskLabel.c_str());
            autocomplete_item->ProjectColor =
                strdup(item.ProjectColor.c_str());
            autocomplete_item->ProjectID =
                static_cast<unsigned int>(item.ProjectID);
            autocomplete_item->TaskID =
                static_cast<unsigned int>(item.TaskID);
            autocomplete_item->Type =
                static_cast<unsigned int>(item.Type);

            previous = autocomplete_item;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

// Tags

kopsik_api_result kopsik_tags(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    KopsikViewItem **first) {
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(first);
    poco_assert(!*first);

    std::vector<std::string> tags = app(context)->Tags();

    *first = 0;
    for (std::vector<std::string>::const_iterator it = tags.begin();
            it != tags.end();
            it++) {
        std::string name = *it;
        KopsikViewItem *item = tag_to_view_item(name);
        item->Next = *first;
        *first = item;
    }

    return KOPSIK_API_SUCCESS;
}

// Workpaces

kopsik_api_result kopsik_workspaces(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    KopsikViewItem **first) {
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(first);
    poco_assert(!*first);

    std::vector<kopsik::Workspace *> workspaces = app(context)->Workspaces();

    *first = 0;
    for (std::vector<kopsik::Workspace *>::const_iterator it =
        workspaces.begin();
            it != workspaces.end();
            it++) {
        KopsikViewItem *item = workspace_to_view_item(*it);
        item->Next = *first;
        *first = item;
    }

    return KOPSIK_API_SUCCESS;
}

// Clients

kopsik_api_result kopsik_clients(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const unsigned int workspace_id,
    KopsikViewItem **first) {
    poco_assert(errmsg);
    poco_assert(errlen);
    poco_assert(first);
    poco_assert(!*first);

    std::vector<kopsik::Client *> clients = app(context)->Clients(workspace_id);

    *first = 0;
    for (std::vector<kopsik::Client *>::const_iterator it = clients.begin();
            it != clients.end();
            it++) {
        KopsikViewItem *item = client_to_view_item(*it);
        item->Next = *first;
        *first = item;
    }

    return KOPSIK_API_SUCCESS;
}

// Projects

kopsik_api_result kopsik_add_project(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const unsigned int workspace_id,
    const unsigned int client_id,
    const char *project_name,
    KopsikViewItem **resulting_project) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(resulting_project);

        kopsik::Project *p = 0;
        kopsik::error err = app(context)->AddProject(
            workspace_id,
            client_id,
            std::string(project_name),
            &p);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
        poco_assert(p);

        *resulting_project = project_to_view_item(p);
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

// Time entries view API

KopsikTimeEntryViewItem *kopsik_time_entry_view_item_init() {
    KopsikTimeEntryViewItem *item = new KopsikTimeEntryViewItem();
    item->DurationInSeconds = 0;
    item->Description = 0;
    item->ProjectAndTaskLabel = 0;
    item->WID = 0;
    item->PID = 0;
    item->TID = 0;
    item->Duration = 0;
    item->Color = 0;
    item->GUID = 0;
    item->Billable = 0;
    item->Tags = 0;
    item->Started = 0;
    item->Ended = 0;
    item->UpdatedAt = 0;
    item->DateHeader = 0;
    item->DurOnly = 0;
    item->Next = 0;
    return item;
}

void kopsik_time_entry_view_item_clear(
    KopsikTimeEntryViewItem *item) {
    if (!item) {
        return;
    }
    if (item->Description) {
        free(item->Description);
        item->Description = 0;
    }
    if (item->ProjectAndTaskLabel) {
        free(item->ProjectAndTaskLabel);
        item->ProjectAndTaskLabel = 0;
    }
    if (item->Duration) {
        free(item->Duration);
        item->Duration = 0;
    }
    if (item->Color) {
        free(item->Color);
        item->Color = 0;
    }
    if (item->GUID) {
        free(item->GUID);
        item->GUID = 0;
    }
    if (item->Tags) {
        free(item->Tags);
        item->Tags = 0;
    }
    if (item->DateHeader) {
        free(item->DateHeader);
        item->DateHeader = 0;
    }
    if (item->DateDuration) {
        free(item->DateDuration);
        item->DateDuration = 0;
    }
    if (item->Next) {
        KopsikTimeEntryViewItem *next =
            reinterpret_cast<KopsikTimeEntryViewItem *>(item->Next);
        kopsik_time_entry_view_item_clear(next);
        item->Next = 0;
    }
    delete item;
}

void kopsik_format_duration_in_seconds_hhmmss(
    const int duration_in_seconds,
    char *out_str,
    const unsigned int max_strlen) {
    poco_assert(out_str);
    poco_assert(max_strlen);
    std::string formatted =
        kopsik::Formatter::FormatDurationInSecondsHHMMSS(duration_in_seconds);
    strncpy(out_str, formatted.c_str(), max_strlen);
}

void kopsik_format_duration_in_seconds_hhmm(
    const int duration_in_seconds,
    char *out_str,
    const unsigned int max_strlen) {
    poco_assert(out_str);
    poco_assert(max_strlen);
    std::string formatted = kopsik::Formatter::FormatDurationInSecondsHHMM(
        duration_in_seconds);
    strncpy(out_str, formatted.c_str(), max_strlen);
}

kopsik_api_result kopsik_start(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *description,
    const char *duration,
    const unsigned int task_id,
    const unsigned int project_id,
    KopsikTimeEntryViewItem *out_view_item) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(out_view_item);

        logger().debug("kopsik_start");

        std::string desc("");
        if (description) {
            desc = std::string(description);
        }

        std::string dur("");
        if (duration) {
            dur = std::string(duration);
        }

        kopsik::TimeEntry *te = 0;
        kopsik::error err =
            app(context)->Start(desc, dur, task_id, project_id, &te);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), err.length());
            return KOPSIK_API_FAILURE;
        }

        if (te) {
            std::string project_label("");
            std::string color_code("");
            app(context)->ProjectLabelAndColorCode(te,
                                                   &project_label,
                                                   &color_code);
            time_entry_to_view_item(te,
                                    project_label,
                                    color_code,
                                    out_view_item,
                                    "");
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_time_entry_view_item_by_guid(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *guid,
    KopsikTimeEntryViewItem *view_item,
    int *was_found) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(guid);
        poco_assert(view_item);
        poco_assert(was_found);

        std::stringstream ss;
        ss << "kopsik_time_entry_view_item_by_guid guid=" << guid;
        logger().trace(ss.str());

        std::string GUID(guid);
        poco_assert(!GUID.empty());

        kopsik::TimeEntry *te = app(context)->GetTimeEntryByGUID(GUID);
        if (!te) {
            *was_found = 0;
            return KOPSIK_API_SUCCESS;
        }

        *was_found = 1;

        std::string project_label("");
        std::string color_code("");
        app(context)->ProjectLabelAndColorCode(te, &project_label, &color_code);

        time_entry_to_view_item(te, project_label, color_code, view_item, "");
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_continue(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *guid,
    KopsikTimeEntryViewItem *view_item) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(guid);
        poco_assert(view_item);

        std::stringstream ss;
        ss << "kopsik_continue guid=" << guid;
        logger().debug(ss.str());

        std::string GUID(guid);

        if (GUID.empty()) {
            strncpy(errmsg, "Missing GUID", errlen);
            return KOPSIK_API_FAILURE;
        }

        kopsik::TimeEntry *te = 0;
        kopsik::error err = app(context)->Continue(GUID, &te);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), err.length());
            return KOPSIK_API_FAILURE;
        }

        if (te) {
            std::string project_label("");
            std::string color_code("");
            app(context)->ProjectLabelAndColorCode(te,
                                                   &project_label,
                                                   &color_code);
            time_entry_to_view_item(te,
                                    project_label,
                                    color_code,
                                    view_item,
                                    "");
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_continue_latest(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    KopsikTimeEntryViewItem *view_item,
    int *was_found) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(view_item);
        poco_assert(was_found);

        *was_found = 0;

        logger().debug("kopsik_continue_latest");

        kopsik::TimeEntry *te = 0;
        kopsik::error err = app(context)->ContinueLatest(&te);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), err.length());
            return KOPSIK_API_FAILURE;
        }

        if (te) {
            *was_found = 1;
            std::string project_label("");
            std::string color_code("");
            app(context)->ProjectLabelAndColorCode(te,
                                                   &project_label,
                                                   &color_code);
            time_entry_to_view_item(te,
                                    project_label,
                                    color_code,
                                    view_item,
                                    "");
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_delete_time_entry(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *guid) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(guid);

        std::stringstream ss;
        ss << "kopsik_delete_time_entry guid=" << guid;
        logger().debug(ss.str());

        std::string GUID(guid);
        if (GUID.empty()) {
            strncpy(errmsg, "Missing GUID", errlen);
            return KOPSIK_API_FAILURE;
        }

        kopsik::error err = app(context)->DeleteTimeEntryByGUID(GUID);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_duration(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *guid,
    const char *value) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(guid);
        poco_assert(value);

        std::stringstream ss;
        ss  << "kopsik_set_time_entry_duration guid=" << guid
            << ", value=" << value;
        logger().debug(ss.str());

        kopsik::error err = app(context)->SetTimeEntryDuration(
            std::string(guid),
            std::string(value));
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_project(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *guid,
    const unsigned int task_id,
    const unsigned int project_id,
    const char *project_guid) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(guid);
        std::string pguid("");
        if (project_guid) {
            pguid = std::string(project_guid);
        }
        kopsik::error err = app(context)->SetTimeEntryProject(std::string(guid),
                            task_id,
                            project_id,
                            pguid);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_start_iso_8601(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *guid,
    const char *value) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(guid);
        poco_assert(value);

        std::stringstream ss;
        ss  << "kopsik_set_time_entry_start_iso_8601 guid=" << guid
            << ", value=" << value;
        logger().debug(ss.str());

        kopsik::error err =
            app(context)->SetTimeEntryStartISO8601(std::string(guid),
                    std::string(value));
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_end_iso_8601(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *guid,
    const char *value) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(guid);
        poco_assert(value);

        std::stringstream ss;
        ss  << "kopsik_set_time_entry_end_iso_8601 guid=" << guid
            << ", value=" << value;
        logger().debug(ss.str());

        kopsik::error err = app(context)->SetTimeEntryEndISO8601(
            std::string(guid),
            std::string(value));
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_tags(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *guid,
    const char *value) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(guid);
        poco_assert(value);

        std::stringstream ss;
        ss  << "kopsik_set_time_entry_tags guid=" << guid
            << ", value=" << value;
        logger().debug(ss.str());

        kopsik::error err = app(context)->SetTimeEntryTags(std::string(guid),
                            std::string(value));
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_billable(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *guid,
    const int value) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(guid);

        std::stringstream ss;
        ss  << "kopsik_set_time_entry_billable guid=" << guid
            << ", value=" << value;
        logger().debug(ss.str());

        kopsik::error err =
            app(context)->SetTimeEntryBillable(std::string(guid), value);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_time_entry_description(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *guid,
    const char *value) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(guid);
        poco_assert(value);

        std::stringstream ss;
        ss  << "kopsik_set_time_entry_description guid=" << guid
            << ", value=" << value;
        logger().debug(ss.str());

        kopsik::error err =
            app(context)->SetTimeEntryDescription(std::string(guid),
                    std::string(value));
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_stop(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    KopsikTimeEntryViewItem *out_view_item,
    int *was_found) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(out_view_item);
        poco_assert(was_found);

        logger().debug("kopsik_stop");

        *was_found = 0;

        kopsik::TimeEntry *te = 0;
        kopsik::error err = app(context)->Stop(&te);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
        if (te) {
            *was_found = 1;
            std::string project_label("");
            std::string color_code("");
            app(context)->ProjectLabelAndColorCode(te,
                                                   &project_label,
                                                   &color_code);
            time_entry_to_view_item(te,
                                    project_label,
                                    color_code,
                                    out_view_item,
                                    "");
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_stop_running_time_entry_at(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const unsigned int at,
    KopsikTimeEntryViewItem *out_view_item,
    int *was_found) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(out_view_item);
        poco_assert(was_found);
        poco_assert(at);

        logger().debug("kopsik_stop");

        *was_found = 0;
        kopsik::TimeEntry *te = 0;
        kopsik::error err = app(context)->StopAt(at, &te);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
        if (te) {
            *was_found = 1;
            std::string project_label("");
            std::string color_code("");
            app(context)->ProjectLabelAndColorCode(te,
                                                   &project_label,
                                                   &color_code);
            time_entry_to_view_item(te,
                                    project_label,
                                    color_code,
                                    out_view_item,
                                    "");
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_running_time_entry_view_item(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    KopsikTimeEntryViewItem *out_item,
    int *out_is_tracking) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(out_item);
        poco_assert(out_is_tracking);

        logger().debug("kopsik_running_time_entry_view_item");

        *out_is_tracking = 0;
        kopsik::TimeEntry *te = 0;
        kopsik::error err = app(context)->RunningTimeEntry(&te);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
        if (te) {
            *out_is_tracking = true;
            std::string project_label("");
            std::string color_code("");
            app(context)->ProjectLabelAndColorCode(te,
                                                   &project_label,
                                                   &color_code);
            time_entry_to_view_item(te,
                                    project_label,
                                    color_code,
                                    out_item,
                                    "");
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_time_entry_view_items(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    KopsikTimeEntryViewItem **first) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(first);

        logger().debug("kopsik_time_entry_view_items");

        std::map<std::string, Poco::Int64> date_durations;
        std::vector<kopsik::TimeEntry *> visible;

        kopsik::error err = app(context)->TimeEntries(&date_durations,
                            &visible);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }

        if (visible.empty()) {
            return KOPSIK_API_SUCCESS;
        }

        *first = 0;
        KopsikTimeEntryViewItem *previous = 0;
        for (unsigned int i = 0; i < visible.size(); i++) {
            kopsik::TimeEntry *te = visible[i];
            KopsikTimeEntryViewItem *view_item =
                kopsik_time_entry_view_item_init();
            if (previous) {
                previous->Next = view_item;
            }
            if (!*first) {
                *first = view_item;
            }

            Poco::Int64 duration = date_durations[te->DateHeaderString()];
            std::string formatted =
                kopsik::Formatter::FormatDurationInSecondsHHMM(duration);

            std::string project_label("");
            std::string color_code("");
            app(context)->ProjectLabelAndColorCode(te,
                                                   &project_label,
                                                   &color_code);
            time_entry_to_view_item(te,
                                    project_label,
                                    color_code,
                                    view_item,
                                    formatted);
            previous = view_item;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_duration_for_date_header(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *date,
    char *duration,
    const unsigned int duration_len) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(duration);
        poco_assert(duration_len);
        poco_assert(date);

        logger().debug("kopsik_duration_for_date_header");

        int sum(0);
        kopsik::error err =
            app(context)->TrackedPerDateHeader(std::string(date), &sum);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }

        kopsik_format_duration_in_seconds_hhmm(sum, duration, duration_len);
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

// Websocket client

void kopsik_websocket_switch(
    void *context,
    const unsigned int on) {
    std::stringstream ss;
    ss << "kopsik_websocket_switch on=" << on;
    logger().debug(ss.str());

    if (on) {
        app(context)->SwitchWebSocketOn();
        return;
    }
    app(context)->SwitchWebSocketOff();
}

// Timeline

void kopsik_timeline_switch(
    void *context,
    const unsigned int on) {
    std::stringstream ss;
    ss << "kopsik_timeline_switch on=" << on;
    logger().debug(ss.str());

    if (on) {
        app(context)->SwitchTimelineOn();
        return;
    }

    app(context)->SwitchTimelineOff();
}

void kopsik_timeline_toggle_recording(
    void *context) {
    logger().debug("kopsik_timeline_toggle_recording");
    app(context)->ToggleTimelineRecording();
}

int kopsik_timeline_is_recording_enabled(
    void *context) {
    return app(context)->RecordTimeline();
}

// Feedback

kopsik_api_result kopsik_feedback_send(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *topic,
    const char *details,
    const char *filename) {
    std::stringstream ss;
    ss << "kopsik_feedback_send topic=" << topic << " details=" << details;
    logger().debug(ss.str());

    kopsik::Feedback feedback(topic, details, filename);

    kopsik::error err = app(context)->SendFeedback(feedback);
    if (err != kopsik::noError) {
        strncpy(errmsg, err.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

// Updates

void kopsik_check_for_updates(
    void *context) {
    logger().debug("kopsik_check_for_updates");

    app(context)->FetchUpdates();
}

kopsik_api_result kopsik_set_update_channel(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    const char *update_channel) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(update_channel);

        kopsik::error err =
            app(context)->SaveUpdateChannel(std::string(update_channel));
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_get_update_channel(
    void *context,
    char *errmsg,
    const unsigned int errlen,
    char *update_channel,
    const unsigned int update_channel_len) {
    try {
        poco_assert(errmsg);
        poco_assert(errlen);
        poco_assert(update_channel);
        poco_assert(update_channel_len);

        std::string s("");
        kopsik::error err = app(context)->LoadUpdateChannel(&s);
        if (err != kopsik::noError) {
            strncpy(errmsg, err.c_str(), errlen);
            return KOPSIK_API_FAILURE;
        }

        strncpy(update_channel, s.c_str(), update_channel_len);
    } catch(const Poco::Exception& exc) {
        strncpy(errmsg, exc.displayText().c_str(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::exception& ex) {
        strncpy(errmsg, ex.what(), errlen);
        return KOPSIK_API_FAILURE;
    } catch(const std::string& ex) {
        strncpy(errmsg, ex.c_str(), errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

int kopsik_parse_duration_string_into_seconds(const char *duration_string) {
    if (!duration_string) {
        return 0;
    }
    return kopsik::Formatter::ParseDurationString(std::string(duration_string));
}
