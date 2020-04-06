//
//  onboarding_service.hpp
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 4/3/20.
//  Copyright © 2020 Toggl. All rights reserved.
//

#ifndef onboarding_service_h
#define onboarding_service_h

#include <string>
#include <Poco/Types.h>
#include <stdio.h>

#include "types.h"

namespace toggl {

class Database;

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

class TOGGL_INTERNAL_EXPORT OnboardingState {
public:
    OnboardingState()
        : openTimelineTabCount(0)
    , lastOpenApp(0)
    , editOnTimelineCount(0)
    , isUseTimelineRecord(false)
    , isUseManualMode(false)
    , isPresentNewUser(false)
    , isPresentOldUser(false)
    , isPresentManualMode(false)
    , isPresentTimelineTab(false)
    , isPresentEditTimeEntry(false)
    , isPresentTimelineTimeEntry(false)
    , isPresentTimelineView(false)
    , isPresentTimelineActivity(false)
    , isPresentRecordActivity(false) {}

private:
    Poco::Int64 openTimelineTabCount;
    Poco::Int64 lastOpenApp;
    Poco::Int64 editOnTimelineCount;
    bool isUseTimelineRecord;
    bool isUseManualMode;

    // Onboarding
    bool isPresentNewUser;
    bool isPresentOldUser;
    bool isPresentManualMode;
    bool isPresentTimelineTab;
    bool isPresentEditTimeEntry;
    bool isPresentTimelineTimeEntry;
    bool isPresentTimelineView;
    bool isPresentTimelineActivity;
    bool isPresentRecordActivity;
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
    OnboardingState state;
    Database *database;
    std::function<void (const OnboardingType)> _callback;
};
}


#endif /* onboarding_service_hpp */

