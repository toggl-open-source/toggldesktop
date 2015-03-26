//
//  PreferencesWindowController.h
//
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MASShortcutView+UserDefaults.h"
#import "DisplayCommand.h"

extern NSString *const kPreferenceGlobalShortcutShowHide;
extern NSString *const kPreferenceGlobalShortcutStartStop;

@interface PreferencesWindowController : NSWindowController <NSTextFieldDelegate>
@property IBOutlet NSTextField *hostTextField;
@property IBOutlet NSTextField *portTextField;
@property IBOutlet NSTextField *usernameTextField;
@property IBOutlet NSTextField *passwordTextField;
@property IBOutlet NSButton *useProxyButton;
@property IBOutlet NSButton *useIdleDetectionButton;
@property IBOutlet NSButton *recordTimelineCheckbox;
@property IBOutlet NSButton *menubarTimerCheckbox;
@property IBOutlet NSButton *menubarProjectCheckbox;
@property IBOutlet NSButton *dockIconCheckbox;
@property IBOutlet NSButton *ontopCheckbox;
@property IBOutlet NSButton *reminderCheckbox;
@property IBOutlet MASShortcutView *showHideShortcutView;
@property IBOutlet MASShortcutView *startStopShortcutView;
@property IBOutlet NSTextField *idleMinutesTextField;
@property IBOutlet NSButton *focusOnShortcutCheckbox;
@property IBOutlet NSTextField *reminderMinutesTextField;
@property IBOutlet NSButton *autodetectProxyCheckbox;
- (IBAction)idleMinutesChange:(id)sender;
- (IBAction)useProxyButtonChanged:(id)sender;
- (IBAction)hostTextFieldChanged:(id)sender;
- (IBAction)portTextFieldChanged:(id)sender;
- (IBAction)usernameTextFieldChanged:(id)sender;
- (IBAction)passwordTextFieldChanged:(id)sender;
- (IBAction)useIdleDetectionButtonChanged:(id)sender;
- (IBAction)recordTimelineCheckboxChanged:(id)sender;
- (IBAction)menubarTimerCheckboxChanged:(id)sender;
- (IBAction)menubarProjectCheckboxChanged:(id)sender;
- (IBAction)dockIconCheckboxChanged:(id)sender;
- (IBAction)ontopCheckboxChanged:(id)sender;
- (IBAction)reminderCheckboxChanged:(id)sender;
- (IBAction)focusOnShortcutCheckboxChanged:(id)sender;
- (IBAction)reminderMinutesChanged:(id)sender;
- (IBAction)autodetectProxyCheckboxChanged:(id)sender;
@property uint64_t user_id;
@property DisplayCommand *originalCmd;
@end
