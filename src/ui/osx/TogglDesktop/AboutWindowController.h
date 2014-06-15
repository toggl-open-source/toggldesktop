//
//  AboutWindowController.h
//  kopsik_ui_osx
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
- (IBAction)checkForUpdateClicked:(id)sender;
- (IBAction)updateChannelSelected:(id)sender;
@property DisplayCommand *displayCommand;
@end
