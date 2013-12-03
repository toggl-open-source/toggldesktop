//
//  UIEvents.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 24/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "UIEvents.h"

// States
NSString *const kUIStateUserLoggedIn = @"UserLoggedIn";
NSString *const kUIStateUserLoggedOut = @"UserLoggedOut";
NSString *const kUIStateTimerRunning = @"TimerRunning";
NSString *const kUIStateTimerStopped = @"TimerStopped";
NSString *const kUIStateTimeEntrySelected = @"TimeEntrySelected";
NSString *const kUIStateTimeEntryDeselected = @"TimeEntryDeselected";
NSString *const kUIStateError = @"Error";
NSString *const kUIStateUpdateAvailable = @"UpdateAvailable";
NSString *const kUIStateUpToDate = @"UpToDate";

// Events
NSString *const kUIEventModelChange = @"ModelChange";
NSString *const kUIEventIdleFinished = @"IdleFinished";
NSString *const kUIEventWebSocketConnection = @"WebsocketConnected";
NSString *const kUIEventTimelineRecording = @"TimelineRecording";
NSString *const kUIEventSettingsChanged = @"SettingsChanged";

// Commands
NSString *const kUICommandNew = @"New";
NSString *const kUICommandStop = @"Stop";
NSString *const kUICommandContinue = @"Continue";
NSString *const kUICommandShowPreferences = @"Show Preferences";
NSString *const kUICommandStopAt = @"Stop At";
NSString *const kUICommandSplitAt = @"Split At";
NSString *const kUICommandEditRunningTimeEntry = @"Edit Running Time Entry";