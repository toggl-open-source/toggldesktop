//
//  AboutWindowController.h
//  Toggl Desktop on the Mac
//
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DisplayCommand.h"
#import "Sparkle.h"

@interface AboutWindowController : NSWindowController <SUUpdaterDelegate>
@property IBOutlet NSTextField *appnameTextField;
@property IBOutlet NSTextField *versionTextField;
@property IBOutlet NSTextView *creditsTextView;
@property IBOutlet NSTextField *updateStatusTextField;
@property IBOutlet NSComboBox *updateChannelComboBox;
@property BOOL windowHasLoad;
@property BOOL restart;
@property NSString *updateStatus;
@property IBOutlet NSTextField *updateChannelLabel;
@property (strong) IBOutlet NSButton *restartButton;
- (IBAction)updateChannelSelected:(id)sender;
- (BOOL)isVisible;
- (void)checkForUpdates;
- (IBAction)clickRestartButton:(id)sender;
@end
