//
//  onboarding_service.cpp
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 4/3/20.
//  Copyright © 2020 Toggl. All rights reserved.
//

#include "onboarding_service.h"
#include "database.h"

namespace toggl {

void OnboardingService::RegisterEvents(std::function<void (const OnboardingType)> callback) {
    _callback = callback;
}

void OnboardingService::SetDatabase(Database *db) {
    database = db;
}

void OnboardingService::SetUserID(Poco::UInt64 user_id) {
    // Load onboarding state
    state = new OnboardingState();
    database->LoadOnboardingState(user_id, state);
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
