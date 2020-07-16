//
//  PreferencesWindowController.h
//
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class DisplayCommand;

extern NSString *const kPreferenceGlobalShortcutShowHide;
extern NSString *const kPreferenceGlobalShortcutStartStop;

@interface PreferencesWindowController : NSWindowController
@property (nonatomic, strong) DisplayCommand *originalCmd;
@property (nonatomic, assign) NSInteger user_id;
@end
