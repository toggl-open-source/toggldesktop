//
//  IdleNotificationWindowController.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 06/11/2013.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface IdleNotificationWindowController : NSWindowController
@property (weak) IBOutlet NSButton *stopButton;
@property (weak) IBOutlet NSButton *splitButton;
@property (weak) IBOutlet NSButton *ignoreButton;
- (IBAction)stopButtonClicked:(id)sender;
- (IBAction)splitButtonClicked:(id)sender;
- (IBAction)ignoreButtonClicked:(id)sender;
@end
