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
- (IBAction)startButtonClicked:(id)sender;
- (IBAction)descriptionComboBoxChanged:(id)sender;
- (IBAction)durationFieldChanged:(id)sender;
- (void)timerFired:(NSTimer *)timer;
@property (weak) IBOutlet NSTextFieldClickable *durationTextField;
@property (weak) IBOutlet NSTextField *startButtonLabelTextField;
@property (weak) IBOutlet NSCustomComboBox *descriptionComboBox;
@property (weak) IBOutlet NSButton *startButton;
@property (weak) IBOutlet NSTextField *projectTextField;
@property (weak) IBOutlet NSTextFieldClickable *descriptionLabel;
@property (weak) IBOutlet NSBox *startButtonBox;
@end
