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

    // User actions
    void openApp();
    void stopTimeEntry();
    void openTimelineTab();
    void turnOnRecordActivity();


};
}


#endif /* onboarding_service_hpp */

