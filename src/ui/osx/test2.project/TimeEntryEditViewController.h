//
//  TimeEntryEditViewController.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TFDatePicker.h"
#import "NSCustomComboBox.h"

@interface TimeEntryEditViewController : NSViewController <NSComboBoxDataSource> {
}
@property (weak) IBOutlet NSCustomComboBox *descriptionCombobox;
@property (weak) IBOutlet NSCustomComboBox *projectSelect;
@property (weak) IBOutlet NSTextField *durationTextField;
@property (weak) IBOutlet NSDatePicker *startTime;
@property (weak) IBOutlet NSDatePicker *endTime;
@property (weak) IBOutlet NSBox *startEndTimeBox;
@property (weak) IBOutlet TFDatePicker *startDate;
@property (weak) IBOutlet NSTokenField *tagsTokenField;
@property (weak) IBOutlet NSButton *billableCheckbox;
@property (weak) IBOutlet NSTextField *lastUpdateTextField;
@property (weak) IBOutlet NSButton *deleteButton;
@property (weak) IBOutlet NSButton *backButton;
@property (weak) IBOutlet NSButton *continueButton;
@property (weak) IBOutlet NSButton *addProjectButton;
@property (weak) IBOutlet NSTextField *workspaceLabel;
- (IBAction)descriptionComboboxChanged:(id)sender;
- (IBAction)durationTextFieldChanged:(id)sender;
- (IBAction)projectSelectChanged:(id)sender;
- (IBAction)startTimeChanged:(id)sender;
- (IBAction)endTimeChanged:(id)sender;
- (IBAction)dateChanged:(id)sender;
- (IBAction)tagsChanged:(id)sender;
- (IBAction)billableCheckBoxClicked:(id)sender;
- (IBAction)deleteButtonClicked:(id)sender;
- (IBAction)backButtonClicked:(id)sender;
- (IBAction)continueButtonClicked:(id)sender;
- (IBAction)addProjectButtonClicked:(id)sender;

// New project related stuff
@property (weak) IBOutlet NSBox *projectSelectBox;
@property (weak) IBOutlet NSBox *addProjectBox;
@property (weak) IBOutlet NSBox *dataholderBox;
@property (weak) IBOutlet NSTextField *projectNameTextField;
@property (weak) IBOutlet NSCustomComboBox *workspaceSelect;
@property (weak) IBOutlet NSCustomComboBox *clientSelect;
- (IBAction)workspaceSelectChanged:(id)sender;
- (IBAction)clientSelectChanged:(id)sender;

@end
