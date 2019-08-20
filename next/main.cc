
#include "toggl_api.h"

#include "event_queue.h"
#include "https_client.h"

void OnApp(void *context, bool open) {
    (void) context; (void) open;
}

void OnSyncState(void *context, int64_t state) {
    (void) context; (void) state;
}

void OnUnsyncedItems(void *context, int64_t count) {
    (void) context; (void) count;
}

void OnError(void *context, const char *errmsg, bool user_error) {
    (void) context; (void) errmsg; (void) user_error;
}

void OnOverlay(void *context, int64_t type) {
    (void) context; (void) type;
}

void OnOnlineState(void *context, int64_t state) {
    (void) context; (void) state;
}

void OnURL(void *context, const char *url) {
    (void) context; (void) url;
}

void OnLogin(void *context, bool open, uint64_t user_id) {
    (void) context; (void) open; (void) user_id;
}

void OnReminder(void *context, const char *title, const char *informative_text) {
    (void) context; (void) title; (void) informative_text;
}

void OnPomodoro(void *context, const char *title, const char *informative_text) {
    (void) context; (void) title; (void) informative_text;
}

void OnPomodoroBreak(void *context, const char *title, const char *informative_text) {
    (void) context; (void) title; (void) informative_text;
}

void OnAutotrackerNotification(void *context, const char *project_name, uint64_t project_id, uint64_t task_id) {
    (void) context; (void) project_name; (void) project_id; (void) task_id;
}

void OnPromotion(void *context, int64_t promotion_type) {
    (void) context; (void) promotion_type;
}

void OnObmExperiment(void *context, uint64_t nr, bool included, bool seen) {
    (void) context; (void) nr; (void) included; (void) seen;
}

void OnTimeEntryList(void *context, bool open, TogglTimeEntryView *first, bool show_load_more_button) {
    (void) context; (void) open; (void) first; (void) show_load_more_button;
}

void OnAutocomplete(void *context, TogglAutocompleteView *first) {
    (void) context; (void) first;
}

void OnHelpArticles(void *context, TogglHelpArticleView *first) {
    (void) context; (void) first;
}

void OnViewItems(void *context, TogglGenericView *first) {
    (void) context; (void) first;
}

void OnTimeEntryEditor(void *context, bool open, TogglTimeEntryView *te, const char *focused_field_name) {
    (void) context; (void) open; (void) te; (void) focused_field_name;
}

void OnSettings(void *context, bool open, TogglSettingsView *settings) {
    (void) context; (void) open; (void) settings;
}

void OnTimerState(void *context, TogglTimeEntryView *te) {
    (void) context; (void) te;
}

void OnIdleNotification(void *context, const char *guid, const char *since, const char *duration, int64_t started, const char *description) {
    (void) context; (void) guid; (void) since; (void) duration; (void) started; (void) description;
}

void OnUpdate(void *context, const char *url) {
    (void) context; (void) url;
}

void OnUpdateDownloadState(void *context, const char *version, int64_t download_state) {
    (void) context; (void) version; (void) download_state;
}

void OnAutotrackerRules(void *context, TogglAutotrackerRuleView *first, uint64_t title_count, string_list_t title_list) {
    (void) context; (void) first; (void) title_count; (void) title_list;
}

void OnProjectColors(void *context, string_list_t color_list, uint64_t color_count) {
    (void) context; (void) color_list; (void) color_count;
}

void OnCountries(void *context, TogglCountryView *first) {
    (void) context; (void) first;
}


static TogglCallbacks callbacks {
    OnApp,
    OnSyncState,
    OnUnsyncedItems,
    OnError,
    OnOverlay,
    OnOnlineState,
    OnURL,
    OnLogin,
    OnReminder,
    OnPomodoro,
    OnPomodoroBreak,
    OnAutotrackerNotification,
    OnPromotion,
    OnObmExperiment,
    OnTimeEntryList,
    OnAutocomplete,
    OnHelpArticles,
    OnViewItems,
    OnTimeEntryEditor,
    OnSettings,
    OnTimerState,
    OnIdleNotification,
    OnUpdate,
    OnUpdateDownloadState,
    OnAutotrackerRules,
    OnProjectColors,
    OnCountries,
};


int main(void) {
    /*
    void *ctx = toggl_context_init("Test", "7.90.0", false, false);
    toggl_register_callbacks(ctx, callbacks);
    toggl_ui_start(ctx);
    */

    toggl::EventQueue *queue = new toggl::EventQueue(nullptr);

    toggl::Event *e = queue->create<toggl::Event>();
    toggl::Event *h = queue->create<toggl::ServerStatus>();

    queue->schedule(e);
    queue->schedule(e, 2500);
    queue->schedule(e, 10000);
    queue->schedule(e, 15000);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    queue->wakeUp();

    std::this_thread::sleep_for(std::chrono::seconds(15));
}
