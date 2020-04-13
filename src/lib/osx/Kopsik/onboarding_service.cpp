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

namespace toggl {

void OnboardingService::RegisterEvents(std::function<void (const OnboardingType)> callback) {
    _callback = callback;
}

void OnboardingService::SetDatabase(Database *db) {
    database = db;
}

void OnboardingService::LoadOnboardingStateFromCurrentUser(User *user) {
    if (user == nullptr) {
        return;
    }
    userID = user->ID();

    // Initialize Onboarding state if need
    bool isAtLaunchTime = false;
    if (state == nullptr) {
        isAtLaunchTime = true;
        state = new OnboardingState();
        database->LoadOnboardingState(userID, state);
    }

    // Load total TE because we don't store it
    state->timeEntryTotal = user->related.TimeEntries.size();
    logger.debug("Onboarding state ", state);

    // If it's the call from the launch
    // Check condition for onboarding after 2 seconds, after the app is loading successfully
    if (isAtLaunchTime) {
        t = new Timer();
        t->SetTimeout([&]() {
            OpenApp();
            t->Stop();
        }, 2000); // Two seconds
    }
}

void OnboardingService::Reset() {
    if (state) {
        delete state;
        state = nullptr;
    }
    userID = 0;
}

void OnboardingService::sync() {
    database->SetOnboardingState(userID, state);
}

// User actions
void OnboardingService::OpenApp() {
    if (state == nullptr) {
        return;
    }

    // Present Onboarding on Timeline Tab
    if (!state->isPresentTimelineTab && state->timeEntryTotal >= 3 && state->openTimelineTabCount == 0) {
        state->isPresentTimelineTab = true;

        // UI
        _callback(TimelineTab);
        sync();
        return;
    }
}

void OnboardingService::StopTimeEntry() {
    if (state == nullptr) {
        return;
    }

    // Stop the first TE
    if (state->timeEntryTotal == 0 && !state->isPresentEditTimeEntry) {
        state->isPresentEditTimeEntry = true;
        state->timeEntryTotal += 1;

        // UI
        _callback(EditTimeEntry);
        sync();
    }
}

void OnboardingService::OpenTimelineTab() {
    state->openTimelineTabCount += 1;
    sync();
}

void OnboardingService::TurnOnRecordActivity() {

}

}
