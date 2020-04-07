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

namespace toggl {

void OnboardingService::RegisterEvents(std::function<void (const OnboardingType)> callback) {
    _callback = callback;
}

void OnboardingService::SetDatabase(Database *db) {
    database = db;
}

void OnboardingService::LoadOnboardingStateFromCurrentUser(User *user) {
    state = new OnboardingState();

    // Load onboarding state from database
    database->LoadOnboardingState(user->ID(), state);

    // Load total TE because we don't store it
    state->timeEntryTotal = user->related.TimeEntries.size();
    logger.debug("Onboarding state ", state);
}

// User actions
void OnboardingService::OpenApp() {

}

void OnboardingService::StopTimeEntry() {

}

void OnboardingService::OpenTimelineTab() {

}

void OnboardingService::TurnOnRecordActivity() {

}

}
