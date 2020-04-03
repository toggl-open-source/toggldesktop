//
//  UIEvents.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 24/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>

// Commands, backend <- UI
extern NSString *const kCommandNew;
extern NSString *const kCommandStop;
extern NSString *const kCommandContinue;

// Display events, UI <- backend
extern NSString *const kDisplayApp;
extern NSString *const kDisplayTimeEntryList;
extern NSString *const kDisplayOverlay;
extern NSString *const kDisplayTimeEntryAutocomplete;
extern NSString *const kDisplayMinitimerAutocomplete;
extern NSString *const kDisplayProjectAutocomplete;
extern NSString *const kDisplayClientSelect;
extern NSString *const kDisplayWorkspaceSelect;
extern NSString *const kDisplayTags;
extern NSString *const kDisplayTimeEntryEditor;
extern NSString *const kDisplayLogin;
extern NSString *const kDisplayError;
extern NSString *const kDisplayUpdate;
extern NSString *const kDisplayIdleNotification;
extern NSString *const kDisplayOnlineState;
extern NSString *const kDisplaySyncState;
extern NSString *const kDisplaySettings;
extern NSString *const kDisplayProxySettings;
extern NSString *const kDisplayTimerState;
extern NSString *const kDisplayUnsyncedItems;
extern NSString *const kDisplayAutotrackerRules;
extern NSString *const kDisplayPromotion;
extern NSString *const kDisplayTimeline;
extern NSString *const kStartTimer;

extern NSString *const kHideDisplayError;
extern NSString *const kForceCloseEditPopover;
extern NSString *const kResizeEditForm;
extern NSString *const kResizeEditFormWidth;
extern NSString *const kResetEditPopover;
extern NSString *const kResetEditPopoverSize;
extern NSString *const kFocusListing;
extern NSString *const kFocusTimer;
extern NSString *const kEscapeListing;
extern NSString *const kToggleManualMode;
extern NSString *const kToggleTimerMode;
extern NSString *const kSetProjectColors;
extern NSString *const kToggleGroup;
extern NSString *const kDisplayCountries;
extern NSString *const kUpdateIconTooltip;
extern NSString *const kUserHasBeenSignup;
extern NSString *const kDeselectAllTimeEntryList;
extern NSString *const kDidAdddManualTimeNotification;
extern NSString *const kTouchBarSettingChanged;
extern NSString *const kStartDisplayInAppMessage;
extern NSString *const kStarTimeEntryWithStartTime;

const char *kFocusedFieldNameDuration;
const char *kFocusedFieldNameDescription;
const char *kFocusedFieldNameProject;
const char *kFocusedFieldNameTag;

extern NSString *const kStartButtonStateChange;
extern NSString *const kInvalidAppleUserCrendential;
extern NSString *const kStartDisplayOnboarding;
