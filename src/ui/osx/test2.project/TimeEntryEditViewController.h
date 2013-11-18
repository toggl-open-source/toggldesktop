//
//  TimeEntryEditViewController.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TimeEntryEditViewController : NSViewController <NSComboBoxDataSource> {
}
@property (weak) IBOutlet NSTextField *descriptionTextField;
@property (weak) IBOutlet NSComboBox *projectSelect;
@property (weak) IBOutlet NSTextField *durationTextField;
@property (weak) IBOutlet NSDatePicker *startTime;
@property (weak) IBOutlet NSDatePicker *endTime;
@property (weak) IBOutlet NSDatePicker *startDate;
@property (weak) IBOutlet NSTokenField *tagsTokenField;
@property (weak) IBOutlet NSButton *billableCheckbox;
@property (weak) IBOutlet NSTextField *lastUpdateTextField;
@property (weak) IBOutlet NSButton *deleteButton;
@property (strong) IBOutlet NSArrayController *projectItemsArrayController;
@property (weak) IBOutlet NSButton *backButton;
- (IBAction)descriptionTextFieldChanged:(id)sender;
- (IBAction)durationTextFieldChanged:(id)sender;
- (IBAction)projectSelectChanged:(id)sender;
- (IBAction)startTimeChanged:(id)sender;
- (IBAction)endTimeChanged:(id)sender;
- (IBAction)dateChanged:(id)sender;
- (IBAction)tagsChanged:(id)sender;
- (IBAction)billableCheckBoxClicked:(id)sender;
- (IBAction)deleteButtonClicked:(id)sender;
- (IBAction)backButtonClicked:(id)sender;
@end
