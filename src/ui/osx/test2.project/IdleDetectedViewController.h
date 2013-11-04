//
//  IdleDetectedViewController.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 04/11/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface IdleDetectedViewController : NSViewController
@property (weak) IBOutlet NSButton *stopButton;
@property (weak) IBOutlet NSButton *splitButton;
@property (weak) IBOutlet NSButton *ignoreButton;
- (IBAction)stopButtonClicked:(id)sender;
- (IBAction)splitButtonClicked:(id)sender;
- (IBAction)ignoreButtonClicked:(id)sender;

@end
