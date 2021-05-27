//
//  onboarding_service.cpp
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 4/3/20.
//  Copyright © 2020 Toggl. All rights reserved.
//

#include "onboarding_service.h"
#include "database/database.h"
#include "model/user.h"
#include "model/time_entry.h"
#include "related_data.h"
#include <time.h>

#if defined(__APPLE__)
const bool is_onboarding_enable = true;
#else
const bool is_onboarding_enable = false;
#endif

using namespace std::chrono;

namespace toggl {

void OnboardingService::RegisterEvents(std::function<void (const OnboardingType)> callback) {
    if (!is_onboarding_enable) {
        return;
    }
    _callback = callback;
}

void OnboardingService::SetDatabase(Database *db) {
    if (!is_onboarding_enable) {
        return;
    }
    database = db;
}

void OnboardingService::LoadOnboardingStateFromCurrentUser(User *_user) {
    if (!is_onboarding_enable) {
        return;
    }

    Poco::Mutex::ScopedLock lock(onboarding_m_);
    if (_user == nullptr) {
        return;
    }
    user = _user;

    // Initialize Onboarding state if need
    bool isAtLaunchTime = false;
    if (state == nullptr) {
        isAtLaunchTime = true;
        state = new OnboardingState();
        database->LoadOnboardingState(user->ID(), state);
    }

    // Load some states because we don't store it from db
    state->timeEntryTotal = user->related.TimeEntries.size();
    getFirstTimeEntryCreatedAtFromUser(user);

    logger.debug("Onboarding state ", state);

    // If it's the call from the launch
    // Check condition for onboarding after 1 seconds, after the app is loading successfully
    if (isAtLaunchTime) {
        timer.add(seconds(1), [&](CppTime::timer_id id) {
            OpenApp();
            timer.remove(id);
        });
    }
}

void OnboardingService::Reset() {
    if (!is_onboarding_enable) {
        return;
    }

    Poco::Mutex::ScopedLock lock(onboarding_m_);
    if (state) {
        delete state;
        state = nullptr;
    }
}

void OnboardingService::sync() {
    if (!is_onboarding_enable) {
        return;
    }

    Poco::Mutex::ScopedLock lock(onboarding_m_);
    if (user == nullptr || state == nullptr) {
        return;
    }
    database->SetOnboardingState(user->ID(), state);
}

// User actions
void OnboardingService::OpenApp() {
    if (!is_onboarding_enable) {
        return;
    }

    Poco::Mutex::ScopedLock lock(onboarding_m_);
    if (state == nullptr) {
        return;
    }

    if (handleNewUserOnboarding()) {
        return;
    }

    if (handleOldUserOnboarding()) {
        return;
    }

    if (handleTimelineTabOnboarding()) {
        return;
    }

    if (handleTextShortcutsOnboarding()) {
        return;
    }
}

void OnboardingService::StopTimeEntry() {
    if (!is_onboarding_enable) {
        return;
    }

    Poco::Mutex::ScopedLock lock(onboarding_m_);
    if (state == nullptr) {
        return;
    }

    if (handleEditTimeEntryOnboarding()) {
        return;
    }

    // Normal cases
    state->timeEntryTotal += 1;
    sync();
}

void OnboardingService::OpenTimelineTab() {
    if (!is_onboarding_enable) {
        return;
    }

    Poco::Mutex::ScopedLock lock(onboarding_m_);
    state->openTimelineTabCount += 1;

    // Onboading on Timeline View
    if (handleTimelineViewOnboarding()) {
        return;
    }

    // Onboarding on Timeline Time Entry
    if (handleTimelineTimeEntryOnboarding()) {
        return;
    }

    // Recording Activity
    if (handleTimelineRecordActivityOnboarding()) {
        return;
    }

    // Normal case
    // Don't store unnecessary counter since all onboardings only consider 3 time at most
    if (state->openTimelineTabCount <= 5) {
        sync();
    }
}

void OnboardingService::TurnOnRecordActivity() {
    if (!is_onboarding_enable) {
        return;
    }

    Poco::Mutex::ScopedLock lock(onboarding_m_);
    if (handleTimelineActivityOnboarding()) {
        return;
    }
    state->isUseTimelineRecord = true;
    sync();
}

void OnboardingService::EditOrAddTimeEntryDirectlyToTimelineView() {
    if (!is_onboarding_enable) {
        return;
    }

    Poco::Mutex::ScopedLock lock(onboarding_m_);
    state->editOnTimelineCount += 1;
    sync();
}

bool OnboardingService::isTrackingTimeEntryForLastThreeDays() {
    if (state == nullptr) {
        return false;
    }
    // the app might not quit, so the firstTimeEntryCreatedAt could be out of date
    // Get new data from user Time Entries
    getFirstTimeEntryCreatedAtFromUser(user);
    if (state->firstTimeEntryCreatedAt == 0) {
        return false;
    }
    time_t now = std::time(NULL);
    return (now - state->firstTimeEntryCreatedAt >= 259200); // 3 days
}

bool OnboardingService::hasAtLeastOneTimelineTimeEntryOnCurrentDay() {
    if (user == nullptr) {
        return false;
    }

    // Get current start/end day
    time_t tzd = timeline_date_at_.tzd();
    time_t start_day = timeline_date_at_.timestamp().epochTime() - tzd;

    // Since we don't store the TimeEntry for each particular day
    // so we have to iterate and find it
    for (std::vector<TimeEntry *>::const_iterator it = user->related.TimeEntries.begin();
         it != user->related.TimeEntries.end(); ++it) {
        TimeEntry *item = *it;
        time_t start_time_entry = Poco::Timestamp::fromEpochTime(item->StartTime()).epochTime();

        // If we have at least one Time Entry in current Timeline day
        if (start_time_entry >= start_day) {
            return true;
        }
    }
    return false;
}

bool OnboardingService::handleTimelineViewOnboarding() {
    /*
     Displayed when entering Timeline tab for the firt time
     */
    if (!state->isPresentTimelineView &&
        state->openTimelineTabCount == 1) {
        state->isPresentTimelineView = true;
        _callback(OnboardingTypeTimelineView);
        sync();
        return true;
    }
    return false;
}

bool OnboardingService::handleTimelineTimeEntryOnboarding() {
    /*
     Should be displayed if:

     > User visits Timeline tab for the 3rd time

     and

     > Hasn’t been editing or adding entries directly on the Timeline yet

     and

     > There is at least one TE to display on chosen day

     If there is no TEs to display, hint should be shown on next possible occassion when all the conditions are true
     */
    if (!state->isPresentTimelineTimeEntry &&
        state->openTimelineTabCount == 3 &&
        state->editOnTimelineCount == 0 &&
        hasAtLeastOneTimelineTimeEntryOnCurrentDay()) {
        state->isPresentTimelineTimeEntry = true;
        _callback(OnboardingTypeTimelineTimeEntry);
        sync();
        return true;
    }
    return false;
}

bool OnboardingService::handleTimelineRecordActivityOnboarding() {
    /*
    Displayed if:

    > User has never been using recording functionality

    and

    > They already tracked TEs for 3 days
    (so they’ve learned some areas of the app already)

    and

    > User is on Timeline tab for at least 3th time
    */
    if (!state->isPresentRecordActivity &&
        !state->isUseTimelineRecord &&
        state->openTimelineTabCount >= 3 &&
        isTrackingTimeEntryForLastThreeDays() ) {
        state->isPresentRecordActivity = true;
        _callback(OnboardingTypeRecordActivity);
        sync();
        return true;
    }
    return false;
}

bool OnboardingService::handleTimelineActivityOnboarding() {
    /*
     Displayed when user turns on ‘Record activity’ for the fist time
     */
    if (state->isUseTimelineRecord == false &&
        !state->isPresentTimelineActivity) {
        state->isPresentTimelineActivity = true;
        _callback(OnboardingTypeTimelineActivity);
        sync();
        return true;
    }
    return false;
}

bool OnboardingService::handleEditTimeEntryOnboarding() {
    if (state->timeEntryTotal == 0 &&
        !state->isPresentEditTimeEntry) {
        state->isPresentEditTimeEntry = true;
        state->timeEntryTotal += 1;
        _callback(OnboardingTypeEditTimeEntry);
        sync();
        return true;
    }
    return false;
}

bool OnboardingService::handleTimelineTabOnboarding() {
    // Present Onboarding on Timeline Tab
    /*
     > For all users who tracked at least 3 TEs and hasn’t opened Timeline tab yet
     (If user opens Timeline tab before that, standard empty state should be displayed)
     */
    if (!state->isPresentTimelineTab && state->timeEntryTotal >= 3 && state->openTimelineTabCount == 0) {
        state->isPresentTimelineTab = true;
        _callback(OnboardingTypeTimelineTab);
        sync();
        return true;
    }
    return false;
}

bool OnboardingService::handleNewUserOnboarding() {
    /*
     New Toogl User
     > When user has never tracked any TEs on Desktop
     > 5 sec after they open the app

     > Hint disappears when user clicks on the X or outside the blue box (applies to all hints)
     > It reappears twice:
         1. when user installs the app
         2. when users comes back to the app after more than 24 hours and still hasn’t tracked any time
     */
    if (user->IsNewUser() && state->timeEntryTotal == 0) {
        bool isTrigger = false;

        // Present twice
        time_t elapsed = std::time(NULL) - state->createdAt;
        if (!state->isPresentNewUser) { // first time
            isTrigger = true;
            state->isPresentNewUser = true;
        } else if (!state->isPresentNewUserSecondTime && elapsed > 86400) { // second time
            isTrigger = true;
            state->isPresentNewUserSecondTime = true;
        }

        if (isTrigger) {
            // 5 sec after they open the app
            timer.add(seconds(5), [&](CppTime::timer_id id) {
                _callback(OnboardingTypeNewUser);
                timer.remove(id);
            });
            sync();
            return true;
        }
    }
    return false;
}

bool OnboardingService::handleOldUserOnboarding() {
    /*
     New Desktop App user and Existing Toggl user

     > When user has never tracked any TEs on Desktop
     > 5 sec after they open the app

     > Hint disappears when user clicks on the X or outside the blue box (applies to all hints)
     > It reappears twice:
         1. when user installs the app
         2. when users comes back to the app after more than 24 hours and still hasn’t tracked any time
     */
    if (!user->IsNewUser()) {
        bool isTrigger = false;
        time_t elapsed = std::time(NULL) - state->createdAt;

        if (!state->isPresentOldUser) { // First time
            isTrigger = true;
            state->isPresentOldUser = true;
        } else if (!state->isPresentOldUserSecondTime && elapsed > 86400 && state->timeEntryTotal == 0) { // Second time
            isTrigger = true;
            state->isPresentOldUserSecondTime = true;
        }

        if (isTrigger) {
            // 5 sec after they open the app
            timer.add(seconds(5), [&](CppTime::timer_id id) {
                _callback(OnboardingTypeOldUser);
                timer.remove(id);
            });
            sync();
            return true;
        }
    }
    return false;
}

bool OnboardingService::handleTextShortcutsOnboarding() {
    /*
     Present Onboarding for # and @ shortcuts on Timer

     > For all users who tracked at least 3 TEs and haven't yet saw shortcuts onboarding
     */
    if (!state->isPresentTextShortcuts && state->timeEntryTotal >= 3) {
        state->isPresentTextShortcuts = true;
        _callback(OnboardingTypeTextShortcuts);
        sync();
        return true;
    }
    return false;
}

void OnboardingService::getFirstTimeEntryCreatedAtFromUser(User *user) {
    if (user == nullptr) {
        return;
    }
    if (!user->related.TimeEntries.empty()) {
        TimeEntry *firstTimeEntry = user->related.TimeEntries.front();
        if (firstTimeEntry != nullptr) {
            state->firstTimeEntryCreatedAt = firstTimeEntry->StartTime();
        }
    }
}
}
