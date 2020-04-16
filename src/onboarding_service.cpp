//
//  onboarding_service.cpp
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 4/3/20.
//  Copyright © 2020 Toggl. All rights reserved.
//

#include "onboarding_service.h"
#include "database.h"
#include "user.h"
#include "timer.h"
#include "time_entry.h"
#include "related_data.h"

#include <time.h>

namespace toggl {

void OnboardingService::RegisterEventQueue(EventQueue &_queue) {
    queue = &_queue;
}

void OnboardingService::RegisterEvents(std::function<void (const OnboardingType)> callback) {
    _callback = callback;
}

void OnboardingService::SetDatabase(Database *db) {
    database = db;
}

void OnboardingService::LoadOnboardingStateFromCurrentUser(User *_user) {
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
    TimeEntry *firstTimeEntry = user->related.TimeEntries.back();
    if (firstTimeEntry != nullptr) {
        state->firstTimeEntryCreatedAt = firstTimeEntry->Start();
    }
    logger.debug("Onboarding state ", state);

    // If it's the call from the launch
    // Check condition for onboarding after 2 seconds, after the app is loading successfully
    if (isAtLaunchTime) {
        std::function<void ()> event = [&]() {
            OpenApp();
        };
        queue->schedule(std::chrono::seconds(2), event);
    }
}

void OnboardingService::Reset() {
    if (state) {
        delete state;
        state = nullptr;
    }
}

void OnboardingService::sync() {
    if (user == nullptr || state == nullptr) {
        return;
    }
    database->SetOnboardingState(user->ID(), state);
}

// User actions
void OnboardingService::OpenApp() {
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
}

void OnboardingService::StopTimeEntry() {
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
    sync();
}

void OnboardingService::TurnOnRecordActivity() {
    if (handleTimelineActivityOnboarding()) {
        return;
    }
    state->isUseTimelineRecord = true;
    sync();
}

void OnboardingService::EditOrAddTimeEntryDirectlyToTimelineView() {
    state->editOnTimelineCount += 1;
    sync();
}

bool OnboardingService::isTrackingTimeEntryForLastThreeDays() {
    if (state == nullptr) {
        return false;
    }
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
    int tzd = timeline_date_at_.tzd();
    time_t start_day = timeline_date_at_.timestamp().epochTime() - tzd;

    // Since we don't store the TimeEntry for each particular day
    // so we have to iterate and find it
    for (std::vector<TimeEntry *>::const_iterator it = user->related.TimeEntries.begin();
         it != user->related.TimeEntries.end(); ++it) {
        TimeEntry *item = *it;
        time_t start_time_entry = Poco::Timestamp::fromEpochTime(item->Start()).epochTime();

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
        _callback(TimelineView);
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
        _callback(TimelineTimeEntry);
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
        _callback(RecordActivity);
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
        _callback(TimelineActivity);
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
        _callback(EditTimeEntry);
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
        _callback(TimelineTab);
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
    if (user->IsNewUser && state->timeEntryTotal == 0) {
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
            std::function<void ()> event = [&]() {
                _callback(NewUser);
            };
            queue->schedule(std::chrono::seconds(5), event);
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
    if (!user->IsNewUser) {
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
            std::function<void ()> event = [&]() {
                _callback(OldUser);
            };
            queue->schedule(std::chrono::seconds(5), event);
            sync();
            return true;
        }
    }
    return false;
}
}
