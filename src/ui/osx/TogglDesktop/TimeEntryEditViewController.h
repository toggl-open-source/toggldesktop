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
#import "AutoCompleteInput.h"
#import "AutocompleteItem.h"

@class UndoTextField;

@interface TimeEntryEditViewController : NSViewController <NSComboBoxDataSource, NSTextFieldDelegate, NSTableViewDelegate>
@property (strong) IBOutlet MKColorWellCustom *colorPicker;
@property IBOutlet UndoTextField *durationTextField;
@property IBOutlet UndoTextField *startTime;
@property IBOutlet UndoTextField *endTime;
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
@property IBOutlet NSPopUpButton *workspaceSelect;
@property IBOutlet NSCustomComboBox *clientSelect;
@property (strong) IBOutlet NSTextField *dateTimeTextField;
@property (strong) IBOutlet NSBox *timeEditBox;
@property (strong) IBOutlet NSButton *addClientButton;
@property (strong) IBOutlet NSTextField *clientNameTextField;
@property (strong) IBOutlet NSButton *saveNewClientButton;
@property (strong) IBOutlet NSButton *projectOpenButton;
@property (strong) IBOutlet AutoCompleteInput *descriptionAutoCompleteInput;
- (IBAction)descriptionAutoCompleteChanged:(id)sender;
@property (strong) IBOutlet AutoCompleteInput *projectAutoCompleteInput;
- (IBAction)projectAutoCompleteChanged:(id)sender;
- (IBAction)durationTextFieldChanged:(id)sender;
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
- (void)closeEdit;
- (BOOL)autcompleteFocused;
- (void)updateWithSelectedDescription:(AutocompleteItem *)autocomplete withKey:(NSString *)key;
- (IBAction)projectOpenButtonClicked:(id)sender;
- (void)updateWithSelectedProject:(AutocompleteItem *)autocomplete withKey:(NSString *)key;
@end
