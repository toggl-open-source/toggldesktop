//
//  AboutWindowController.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 29/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AboutWindowController : NSWindowController
@property (weak) IBOutlet NSTextField *appnameTextField;
@property (weak) IBOutlet NSTextField *versionTextField;
@property (unsafe_unretained) IBOutlet NSTextView *creditsTextView;
@property (weak) IBOutlet NSButton *checkForUpdateButton;
@property (weak) IBOutlet NSComboBox *updateChannelComboBox;
- (IBAction)checkForUpdateClicked:(id)sender;
- (IBAction)updateChannelSelected:(id)sender;
@end
