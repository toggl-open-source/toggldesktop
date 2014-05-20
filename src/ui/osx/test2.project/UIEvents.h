//
//  UIEvents.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 24/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>

// Commands, backend <- UI
extern NSString *const kCommandNew;
extern NSString *const kCommandStop;
extern NSString *const kCommandContinue;
extern NSString *const kCommandStopAt;

// Display events, UI <- backend
extern NSString *const kDisplayTimeEntryList;
extern NSString *const kDisplayAutocomplete;
extern NSString *const kDisplayClientSelect;
extern NSString *const kDisplayWorkspaceSelect;
extern NSString *const kDisplayTags;
extern NSString *const kDisplayTimeEntryEditor;
extern NSString *const kDisplayLogin;
extern NSString *const kDisplayError;
extern NSString *const kDisplayUpdate;
extern NSString *const kDisplayIdleNotification;
extern NSString *const kDisplayOnlineState;
extern NSString *const kDisplaySettings;
extern NSString *const kDisplayProxySettings;
extern NSString *const kDisplayTimerState;

extern NSString *const kHideDisplayError;

const char *kFocusedFieldNameDuration;
const char *kFocusedFieldNameDescription;
const char *kFocusedFieldNameProject;
