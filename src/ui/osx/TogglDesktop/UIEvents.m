//
//  UIEvents.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 24/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "UIEvents.h"

NSString *const kCommandNew = @"New";
NSString *const kCommandStop = @"Stop";
NSString *const kCommandContinue = @"Continue";

NSString *const kDisplayApp = @"kDisplayApp";
NSString *const kDisplayTimeEntryList = @"kDisplayTimeEntryList";
NSString *const kDisplayOverlay = @"kDisplayOverlay";
NSString *const kDisplayTimeEntryAutocomplete = @"kDisplayTimeEntryAutocomplete";
NSString *const kDisplayMinitimerAutocomplete = @"kDisplayMinitimerAutocomplete";
NSString *const kDisplayProjectAutocomplete = @"kDisplayProjectAutocomplete";
NSString *const kDisplayClientSelect = @"kDisplayClientSelect";
NSString *const kDisplayWorkspaceSelect = @"kDisplayWorkspaceSelect";
NSString *const kDisplayTags = @"kDisplayTags";
NSString *const kDisplayTimeEntryEditor = @"kDisplayTimeEntryEditor";
NSString *const kDisplayLogin = @"kDisplayLogin";
NSString *const kDisplayError = @"kDisplayError";
NSString *const kDisplayIdleNotification = @"kDisplayIdleNotification";
NSString *const kDisplayOnlineState = @"kDisplayOnlineState";
NSString *const kDisplaySyncState = @"kDisplaySyncState";
NSString *const kDisplaySettings = @"kDisplaySettings";
NSString *const kDisplayProxySettings = @"kDisplayProxySettings";
NSString *const kDisplayTimerState = @"kDisplayTimerState";
NSString *const kDisplayUnsyncedItems = @"kDisplayUnsyncedItems";
NSString *const kDisplayAutotrackerRules = @"kDisplayAutotrackerRules";
NSString *const kDisplayPromotion = @"kDisplayPromotion";
NSString *const kDisplayTimeline = @"kDisplayTimeline";
NSString *const kStartTimer = @"kStartTimer";

NSString *const kHideDisplayError = @"kHideDisplayError";
NSString *const kForceCloseEditPopover = @"kForceCloseEditPopover";
NSString *const kResizeEditForm = @"kResizeEditForm";
NSString *const kResizeEditFormWidth = @"kResizeEditFormWidth";
NSString *const kResetEditPopover = @"kResetEditPopover";
NSString *const kResetEditPopoverSize = @"kResetEditPopoverSize";
NSString *const kFocusListing = @"kFocusListing";
NSString *const kFocusTimer = @"kFocusTimer";
NSString *const kEscapeListing = @"kEscapeListing";
NSString *const kToggleManualMode = @"kToggleManualMode";
NSString *const kToggleTimerMode = @"kToggleTimerMode";
NSString *const kSetProjectColors = @"kSetProjectColors";
NSString *const kToggleGroup = @"ToggleGroup";
NSString *const kDisplayCountries = @"kDisplayCountries";
NSString *const kUpdateIconTooltip = @"kUpdateIconTooltip";
NSString *const kUserHasBeenSignup = @"kUserHasBeenSignup";
NSString *const kTouchBarSettingChanged = @"kTouchBarSettingChanged";
NSString *const kStartDisplayInAppMessage = @"kStartDisplayInAppMessage";

NSString *const kDeselectAllTimeEntryList = @"kDeselectAllTimeEntryList";
NSString *const kDidAdddManualTimeNotification = @"kDidAdddManualTimeNotification";
NSString *const kStarTimeEntryWithStartTime = @"kStarTimeEntryWithStartTime";

const char *kFocusedFieldNameDuration = "duration";
const char *kFocusedFieldNameDescription = "description";
const char *kFocusedFieldNameProject = "project";
const char *kFocusedFieldNameTag = "tag";

NSString *const kStartButtonStateChange = @"kStartButtonStateChange";
NSString *const kInvalidAppleUserCrendential = @"kInvalidAppleUserCrendential";

NSString *const kStartDisplayOnboarding = @"kStartDisplayOnboarding";
