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
NSString *const kDisplayTimeEntryAutocomplete = @"kDisplayTimeEntryAutocomplete";
NSString *const kDisplayProjectAutocomplete = @"kDisplayProjectAutocomplete";
NSString *const kDisplayClientSelect = @"kDisplayClientSelect";
NSString *const kDisplayWorkspaceSelect = @"kDisplayWorkspaceSelect";
NSString *const kDisplayTags = @"kDisplayTags";
NSString *const kDisplayTimeEntryEditor = @"kDisplayTimeEntryEditor";
NSString *const kDisplayLogin = @"kDisplayLogin";
NSString *const kDisplayError = @"kDisplayError";
NSString *const kDisplayIdleNotification = @"kDisplayIdleNotification";
NSString *const kDisplayOnlineState = @"kDisplayOnlineState";
NSString *const kDisplaySettings = @"kDisplaySettings";
NSString *const kDisplayProxySettings = @"kDisplayProxySettings";
NSString *const kDisplayTimerState = @"kDisplayTimerState";

NSString *const kHideDisplayError = @"kHideDisplayError";
NSString *const kForceCloseEditPopover = @"kForceCloseEditPopover";
NSString *const kResizeEditForm = @"kResizeEditForm";
NSString *const kResizeEditFormWidth = @"kResizeEditFormWidth";
NSString *const kResetEditPopover = @"kResetEditPopover";

const char *kFocusedFieldNameDuration = "duration";
const char *kFocusedFieldNameDescription = "description";
const char *kFocusedFieldNameProject = "project";