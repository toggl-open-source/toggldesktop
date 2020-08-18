//
//  TimerEditViewController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class NSBoxClickable;

@interface TimerEditViewController : NSViewController
@property (weak) IBOutlet NSBoxClickable *manualBox;
@property (weak) IBOutlet NSBoxClickable *mainBox;

- (void)startButtonClicked;
- (void)focusTimer;

@end
