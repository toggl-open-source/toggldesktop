//
//  TimerEditViewController.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NSTextFieldClickable.h"
#import "NSCustomComboBox.h"

@interface TimerEditViewController : NSViewController <NSComboBoxDataSource> {
}
@property IBOutlet NSTextFieldClickable *durationTextField;
@property IBOutlet NSTextField *startButtonLabelTextField;
@property IBOutlet NSCustomComboBox *descriptionComboBox;
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
