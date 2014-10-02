//
//  AboutWindowController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 29/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DisplayCommand.h"
#import "Sparkle.h"

@interface AboutWindowController : NSWindowController
@property IBOutlet NSTextField *appnameTextField;
@property IBOutlet NSTextField *versionTextField;
@property IBOutlet NSTextView *creditsTextView;
@property IBOutlet NSComboBox *updateChannelComboBox;
@property BOOL windowHasLoad;
- (IBAction)updateChannelSelected:(id)sender;
- (BOOL)isVisible;
@end
