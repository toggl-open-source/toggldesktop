//
//  UIEvents.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 24/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Foundation/Foundation.h>

// States
extern NSString *const kUIStateUserLoggedIn;
extern NSString *const kUIStateUserLoggedOut;
extern NSString *const kUIStateTimerRunning;
extern NSString *const kUIStateTimerStopped;
extern NSString *const kUIStateTimeEntrySelected;
extern NSString *const kUIStateTimeEntryDeselected;
extern NSString *const kUIStateError;

// Events
extern NSString *const kUIEventModelChange;

// Commands
extern NSString *const kUICommandNew;
extern NSString *const kUICommandStop;
extern NSString *const kUICommandContinue;
