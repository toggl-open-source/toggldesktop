//
//  AboutWindowController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 29/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DisplayCommand.h"

@interface AboutWindowController : NSWindowController
@property IBOutlet NSTextField *appnameTextField;
@property IBOutlet NSTextField *versionTextField;
@property IBOutlet NSTextView *creditsTextView;
@property IBOutlet NSButton *checkForUpdateButton;
@property IBOutlet NSComboBox *updateChannelComboBox;
@property DisplayCommand *displayCommand;
@property BOOL windowHasLoad;
- (IBAction)checkForUpdateClicked:(id)sender;
- (IBAction)updateChannelSelected:(id)sender;
- (BOOL)isVisible;
@end
