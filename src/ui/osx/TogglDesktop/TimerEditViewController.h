//
//  TimerEditViewController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NSTextFieldClickable.h"
#import "NSCustomTimerComboBox.h"

@interface TimerEditViewController : NSViewController <NSComboBoxDataSource, NSTextFieldDelegate> {
}
@property IBOutlet NSTextFieldClickable *durationTextField;
@property IBOutlet NSCustomTimerComboBox *descriptionComboBox;
@property IBOutlet NSButton *startButton;
@property IBOutlet NSTextField *projectTextField;
@property IBOutlet NSTextFieldClickable *descriptionLabel;
@property IBOutlet NSBox *startButtonBox;
@property NSArray *projectComboConstraint;
@property NSArray *projectLabelConstraint;
- (IBAction)startButtonClicked:(id)sender;
- (IBAction)descriptionComboBoxChanged:(id)sender;
- (IBAction)durationFieldChanged:(id)sender;
- (void)timerFired:(NSTimer *)timer;
@end
