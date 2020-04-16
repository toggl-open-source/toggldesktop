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
#include <Poco/LocalDateTime.h>

#include "types.h"
#include "logger.h"

namespace toggl {

class Database;
class User;
class Timer;

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
        : local_id(0)
    , user_id(0)
    , timeEntryTotal(0)
    , openTimelineTabCount(0)
    , firstTimeEntryCreatedAt()
    , createdAt(std::time(NULL))
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

    Poco::Int64 local_id;
    Poco::Int64 user_id;
    Poco::Int64 timeEntryTotal;
    Poco::Int64 firstTimeEntryCreatedAt;
    Poco::Int64 openTimelineTabCount;
    Poco::Int64 createdAt;
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
    static OnboardingService* getInstance() {
        static OnboardingService instance;
        return &instance;
    }

    // Register action
    void RegisterEvents(std::function<void (const OnboardingType)> callback);
    void SetDatabase(Database *db);
    void LoadOnboardingStateFromCurrentUser(User *user);
    void Reset();
    
    // User actions
    void OpenApp();
    void StopTimeEntry();
    void OpenTimelineTab();
    void TurnOnRecordActivity();
    void EditOrAddTimeEntryDirectlyToTimelineView();
    void SetTimelineDateAt(const Poco::LocalDateTime &value) {
        timeline_date_at_ = value;
    }
    
private:
    OnboardingService(): timeline_date_at_(Poco::LocalDateTime()) {};

    OnboardingState *state;
    Database *database;
    Timer *t;
    Logger logger { "Onboarding" };
    User *user;
    Poco::LocalDateTime timeline_date_at_;
    std::function<void (const OnboardingType)> _callback;

    bool isTrackingTimeEntryForLastThreeDays();
    bool hasAtLeastOneTimelineTimeEntryOnCurrentDay();
    void sync();

    bool handleTimelineViewOnboarding();
    bool handleTimelineTimeEntryOnboarding();
    bool handleTimelineRecordActivityOnboarding();
    bool handleTimelineActivityOnboarding();
    bool handleEditTimeEntryOnboarding();
    bool handleTimelineTabOnboarding();
    bool handleNewUserOnboarding();
    bool handleOldUserOnboarding();
};
}

#endif /* onboarding_service_hpp */

