//
//  TimerEditViewController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NSTextFieldClickable.h"
#import "NSTextFieldDuration.h"
#import "NSCustomTimerComboBox.h"
#import "NSHoverButton.h"
#import "NSBoxClickable.h"

@interface TimerEditViewController : NSViewController <NSComboBoxDataSource, NSTextFieldDelegate> {
}
@property (strong) IBOutlet NSBox *hidingBox;
@property (strong) IBOutlet NSBoxClickable *mainBox;
@property IBOutlet NSTextFieldDuration *durationTextField;
@property IBOutlet NSCustomTimerComboBox *descriptionComboBox;
@property IBOutlet NSHoverButton *startButton;
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
