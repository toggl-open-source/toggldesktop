//
//  TimeEntryEditViewController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TFDatePicker.h"
#import "NSCustomComboBox.h"
#import "NSTextFieldClickable.h"
#import "NSResize.h"
#import "MkColorWellCustom.h"

@interface TimeEntryEditViewController : NSViewController <NSComboBoxDataSource> {
}
@property (strong) IBOutlet MKColorWellCustom *colorPicker;
@property IBOutlet NSCustomComboBox *descriptionCombobox;
@property IBOutlet NSCustomComboBox *projectSelect;
@property IBOutlet NSTextField *durationTextField;
@property IBOutlet NSTextField *startTime;
@property IBOutlet NSTextField *endTime;
@property IBOutlet NSBox *startEndTimeBox;
@property IBOutlet TFDatePicker *startDate;
@property IBOutlet NSTokenField *tagsTokenField;
@property IBOutlet NSButton *billableCheckbox;
@property IBOutlet NSButton *projectPublicCheckbox;
@property IBOutlet NSTextField *lastUpdateTextField;
@property IBOutlet NSButton *deleteButton;
@property IBOutlet NSButton *backButton;
@property IBOutlet NSButton *addProjectButton;
@property IBOutlet NSTextField *workspaceLabel;
@property IBOutlet NSTextField *currentWorkspaceLabel;
@property IBOutlet NSResize *resizeHandle;
@property IBOutlet NSResize *resizeHandleLeft;
@property (strong) IBOutlet NSBox *timeTextBox;
@property (strong) IBOutlet NSBox *dateBox;
@property (strong) IBOutlet NSBox *durationBox;
@property NSPoint lastPosition;
@property IBOutlet NSBox *projectSelectBox;
@property IBOutlet NSBox *addProjectBox;
@property IBOutlet NSBox *dataholderBox;
@property IBOutlet NSTextField *projectNameTextField;
@property IBOutlet NSCustomComboBox *workspaceSelect;
@property IBOutlet NSCustomComboBox *clientSelect;
@property (strong) IBOutlet NSTextField *dateTimeTextField;
@property (strong) IBOutlet NSBox *timeEditBox;
@property (strong) IBOutlet NSButton *addClientButton;
@property (strong) IBOutlet NSTextField *clientNameTextField;
@property (strong) IBOutlet NSButton *saveNewClientButton;
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
- (IBAction)addProjectButtonClicked:(id)sender;
- (IBAction)workspaceSelectChanged:(id)sender;
- (IBAction)clientSelectChanged:(id)sender;
- (IBAction)addClientButtonClicked:(id)sender;
- (IBAction)saveAddClientButtonClicked:(id)sender;
- (void)setDragHandle:(BOOL)onLeft;
- (void)setInsertionPointColor;
- (void)closeEdit;
@end
