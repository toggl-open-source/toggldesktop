//
//  onboarding_service.hpp
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 4/3/20.
//  Copyright © 2020 Toggl. All rights reserved.
//

#ifndef onboarding_service_h
#define onboarding_service_h

#include "types.h"
#include <stdio.h>

class Database;

namespace toggl {
enum OnboardingType {
    NewUser,
    OldUser,
    ManualyMode,
    TimelineTab,
    EditTimeEntry,
    TimelineTimeEntry,
    TimelineView,
    TimelineActivity,
    RecordActivity
};

class TOGGL_INTERNAL_EXPORT OnboardingService {
public:

    // Register action
    void RegisterEvents(std::function<void (const OnboardingType)> callback);
    void SetDatabase(Database *db);

    // User actions
    void OpenApp();
    void StopTimeEntry();
    void OpenTimelineTab();
    void TurnOnRecordActivity();

private:
    Database *database;
    std::function<void (const OnboardingType)> _callback;
};
}


#endif /* onboarding_service_hpp */

