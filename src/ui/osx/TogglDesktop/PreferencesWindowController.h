//
//  PreferencesWindowController.h
//
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DisplayCommand.h"
#import "AutocompleteItem.h"
#import "AutocompleteDataSource.h"
#import "NSCustomComboBox.h"
#import <MASShortcut/Shortcut.h>

extern NSString *const kPreferenceGlobalShortcutShowHide;
extern NSString *const kPreferenceGlobalShortcutStartStop;

@interface PreferencesWindowController : NSWindowController <NSTextFieldDelegate, NSTableViewDataSource, NSComboBoxDataSource, NSComboBoxDelegate>

@property DisplayCommand *originalCmd;
@property uint64_t user_id;
@end
