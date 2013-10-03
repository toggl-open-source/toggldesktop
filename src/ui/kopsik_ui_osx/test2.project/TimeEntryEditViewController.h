//
//  TimeEntryEditViewController.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TimeEntryEditViewController : NSViewController
- (IBAction)backButtonClicked:(id)sender;
@property (weak) IBOutlet NSComboBox *projectSelect;
@property (weak) IBOutlet NSTextField *durationTextField;
@property (weak) IBOutlet NSDatePicker *startTime;
@property (weak) IBOutlet NSDatePicker *endTime;
@property (weak) IBOutlet NSDatePicker *startDate;
@property (weak) IBOutlet NSTokenField *tags;
@property (weak) IBOutlet NSButton *billableCheckbox;
@property (weak) IBOutlet NSTextField *lastEditTextField;
@property (weak) IBOutlet NSButton *deleteButton;
@property (weak) IBOutlet NSButton *backButton;
@end
