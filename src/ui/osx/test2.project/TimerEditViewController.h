//
//  TimerEditViewController.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TimerEditViewController : NSViewController <NSComboBoxDataSource> {
}
- (IBAction)startButtonClicked:(id)sender;
- (IBAction)descriptionComboBoxChanged:(id)sender;
- (IBAction)durationFieldChanged:(id)sender;
- (void)timerFired:(NSTimer *)timer;
@property (weak) IBOutlet NSTextField *durationTextField;
@property (weak) IBOutlet NSTextField *startButtonLabelTextField;
@property (weak) IBOutlet NSComboBox *descriptionComboBox;
@property (weak) IBOutlet NSButton *startButton;
@property (weak) IBOutlet NSTextField *projectTextField;
@property (weak) IBOutlet NSBox *startButtonBox;
@end
