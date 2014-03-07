
//
//  TimerEditViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//
#import "EditNotification.h"
#import "TimerEditViewController.h"
#import "UIEvents.h"
#import "AutocompleteItem.h"
#import "Context.h"
#import "ErrorHandler.h"
#import "AutocompleteDataSource.h"
#import "ConvertHexColor.h"
#import "ModelChange.h"
#import "NSComboBox_Expansion.h"
#import "TimeEntryViewItem.h"
#import "NSTextFieldClickable.h"

@interface TimerEditViewController ()
@property AutocompleteDataSource *autocompleteDataSource;
@property TimeEntryViewItem *time_entry;
@property NSTimer *timerAutocompleteRendering;
@property NSTimer *timer;
@end

@implementation TimerEditViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
      self.autocompleteDataSource = [[AutocompleteDataSource alloc] init];

      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateTimerRunning
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateTimerStopped
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateUserLoggedIn
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventModelChange
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUICommandEditRunningTimeEntry
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventShowListView
                                                 object:nil];

      self.time_entry = [[TimeEntryViewItem alloc] init];
      
      self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                    target:self
                                                  selector:@selector(timerFired:)
                                                  userInfo:nil
                                                   repeats:YES];
    }
    
    return self;
}

- (NSString *)comboBox:(NSComboBox *)comboBox completedString:(NSString *)partialString {
  return [self.autocompleteDataSource completedString:partialString];
}

- (void)renderAutocomplete {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  self.timerAutocompleteRendering = nil;

  [self.autocompleteDataSource fetch:YES withTasks:YES withProjects:YES];

  if (self.descriptionComboBox.dataSource == nil) {
    self.descriptionComboBox.usesDataSource = YES;
    self.descriptionComboBox.dataSource = self;
  }
  [self.descriptionComboBox reloadData];
}

- (void) scheduleAutocompleteRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  if (self.timerAutocompleteRendering != nil) {
    return;
  }
  @synchronized(self) {
    self.timerAutocompleteRendering = [NSTimer scheduledTimerWithTimeInterval:kThrottleSeconds
                                                                       target:self
                                                                     selector:@selector(renderAutocomplete)
                                                                     userInfo:nil
                                                                      repeats:NO];
  }
}

- (void)eventHandler: (NSNotification *) notification {
  if ([notification.name isEqualToString:kUIEventShowListView]) {
    [self.descriptionComboBox becomeFirstResponder];
    return;
  }

  if ([notification.name isEqualToString:kUIStateTimerRunning]) {
    self.time_entry = notification.object;
    [self performSelectorOnMainThread:@selector(render)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }

  if ([notification.name isEqualToString:kUIStateTimerStopped]) {
    self.time_entry = [[TimeEntryViewItem alloc] init];
    [self performSelectorOnMainThread:@selector(render)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }

  if ([notification.name isEqualToString:kUICommandEditRunningTimeEntry]) {
    if (self.time_entry != nil && self.time_entry.GUID != nil) {
      EditNotification *edit = [[EditNotification alloc] init];
      edit.EntryGUID = self.time_entry.GUID;
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntrySelected
                                                        object:edit];
    }
    return;
  }
  
  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    [self performSelectorOnMainThread:@selector(scheduleAutocompleteRendering)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }

  if (![notification.name isEqualToString:kUIEventModelChange]) {
    return;
  }
  
  ModelChange *change = notification.object;
  if (![change.ModelType isEqualToString:@"tag"]) {
    [self performSelectorOnMainThread:@selector(scheduleAutocompleteRendering)
                           withObject:nil
                        waitUntilDone:NO];
  }

  // We only care about time entry changes
  if (! [change.ModelType isEqualToString:@"time_entry"]) {
    return;
  }

  // Handle delete
  if ([change.ChangeType isEqualToString:@"delete"]) {
    // Time entry we thought was running, has been deleted.
    if ([change.GUID isEqualToString:self.time_entry.GUID]) {
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerStopped
                                                          object:nil];
    }
    return;
  }

  // Handle update
  TimeEntryViewItem *updated = [TimeEntryViewItem findByGUID:change.GUID];

  // Time entry we thought was running, has been stopped.
  if ((updated.duration_in_seconds >= 0) &&
      [updated.GUID isEqualToString:self.time_entry.GUID]) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerStopped
                                                        object:nil];
    return;
  }

  // Time entry we did not know was running, is running.
  if ((updated.duration_in_seconds < 0) &&
      ![updated.GUID isEqualToString:self.time_entry.GUID]) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerRunning
                                                        object:updated];
    return;
  }

  // Time entry is still running and needs to be updated.
  if ((updated.duration_in_seconds < 0) &&
      [updated.GUID isEqualToString:self.time_entry.GUID]) {
    self.time_entry = updated;
    [self performSelectorOnMainThread:@selector(render)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }
}

-(void)textFieldClicked:(id)sender {
  if (self.time_entry != nil && self.time_entry.GUID != nil) {
    if (sender == self.durationTextField) {
      EditNotification *edit = [[EditNotification alloc] init];
      edit.EntryGUID = self.time_entry.GUID;
      edit.FieldName = kUIDurationClicked;
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntrySelected
                                                        object:edit];
    } else if (sender == self.descriptionLabel) {
      EditNotification *edit = [[EditNotification alloc] init];
      edit.EntryGUID = self.time_entry.GUID;
      edit.FieldName = kUIDescriptionClicked;
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntrySelected
                                                        object:edit];
    }
  }

}

- (void) render {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  
  // Start/stop button title and color depend on
  // whether time entry is running
  if (self.time_entry.duration_in_seconds < 0) {
    self.startButtonLabelTextField.stringValue = @"Stop";
    self.startButton.toolTip = @"Stop";
    self.startButtonBox.borderColor = [ConvertHexColor hexCodeToNSColor:@"#ec0000"];
    self.startButtonBox.fillColor = [ConvertHexColor hexCodeToNSColor:@"#ec0000"];
  } else {
    self.startButtonLabelTextField.stringValue = @"Start";
    self.startButton.toolTip = @"Start";
    self.startButtonBox.borderColor = [ConvertHexColor hexCodeToNSColor:@"#4bc800"];
    self.startButtonBox.fillColor = [ConvertHexColor hexCodeToNSColor:@"#4bc800"];
  }

  // Description and duration cannot be edited
  // while time entry is running
  if (self.time_entry.duration_in_seconds < 0) {
    [self.durationTextField setDelegate:self];
    self.descriptionLabel.stringValue = self.time_entry.Description;
    self.descriptionLabel.toolTip = self.time_entry.Description;
    [self.descriptionComboBox setHidden:YES];
    [self.descriptionLabel setHidden:NO];
    [self.durationTextField setEditable:NO];
    [self.durationTextField setSelectable:NO];
  } else {
    [self.descriptionComboBox setHidden:NO];
    [self.descriptionLabel setHidden:YES];
    [self.durationTextField setEditable:YES];
    [self.durationTextField setSelectable:YES];
    [self.durationTextField setDelegate:self.durationTextField];
  }
  
  // Display description
  if (self.time_entry.Description != nil) {
    self.descriptionComboBox.stringValue = self.time_entry.Description;
    self.descriptionComboBox.toolTip = self.time_entry.Description;
  } else {
    self.descriptionComboBox.toolTip = nil;
    self.descriptionComboBox.stringValue = @"";
  }
  
  // If a project is assigned, then project name
  // is visible.
  if (self.time_entry.ProjectID || self.time_entry.ProjectGUID) {
    [self.projectTextField setHidden:NO];
  } else {
    [self.projectTextField setHidden:YES];
  }
  
  // Display project name
  if (self.time_entry.ProjectAndTaskLabel != nil) {
    self.projectTextField.stringValue =
      [self.time_entry.ProjectAndTaskLabel uppercaseString];
      self.projectTextField.toolTip = self.time_entry.ProjectAndTaskLabel;
  } else {
    self.projectTextField.stringValue = @"";
    self.projectTextField.toolTip = nil;
  }
  self.projectTextField.backgroundColor =
    [ConvertHexColor hexCodeToNSColor:self.time_entry.ProjectColor];
  
  // Display duration
  if (self.time_entry.duration != nil) {
    self.durationTextField.stringValue = self.time_entry.duration;
  } else {
    self.durationTextField.stringValue = @"";
  }
}

-(NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox{
  return [self.autocompleteDataSource count];
}

-(id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)row{
  return [self.autocompleteDataSource keyAtIndex:row];
}

- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)aString {
  return [self.autocompleteDataSource indexOfKey:aString];
}

- (IBAction)startButtonClicked:(id)sender {
  if (self.time_entry.duration_in_seconds < 0) {
    self.durationTextField.stringValue = @"";
    self.descriptionComboBox.stringValue = @"";
    self.projectTextField.stringValue = @"";
    [self.projectTextField setHidden:YES];
    [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandStop
                                                        object:nil];
    return;
  }

  // resign current FirstResponder
  [self.durationTextField.window makeFirstResponder:[self.durationTextField superview]];
  self.time_entry.Duration = self.durationTextField.stringValue;
  self.time_entry.Description = self.descriptionComboBox.stringValue;
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandNew
                                                      object:self.time_entry];

  // Reset autocomplete filter
  [self.autocompleteDataSource setFilter:@""];
  [self.descriptionComboBox reloadData];

  // Clear Time entry form fields after stop
  if ([[self.durationTextField stringValue]length]>0){
    self.durationTextField.stringValue=@"";
    self.descriptionComboBox.stringValue=@"";
  }
}

- (IBAction)durationFieldChanged:(id)sender {
  if (![self.durationTextField.stringValue length]){
    return;
  }

  // Parse text into seconds
  const char *duration_string = [self.durationTextField.stringValue UTF8String];
  int seconds = kopsik_parse_duration_string_into_seconds(duration_string);

  // Format seconds as text again
  char str[duration_str_len];
  kopsik_format_duration_in_seconds_hhmmss(seconds,
                                           str,
                                           duration_str_len);
  NSString *newValue = [NSString stringWithUTF8String:str];
  [self.durationTextField setStringValue:newValue];
}

- (IBAction)descriptionComboBoxChanged:(id)sender {
  NSString *key = [self.descriptionComboBox stringValue];
  AutocompleteItem *item = [self.autocompleteDataSource get:key];

  // User has entered free text
  if (item == nil) {
    self.time_entry.Description = [self.descriptionComboBox stringValue];
    return;
  }

  // User has selected a autocomplete item.
  // It could be a time entry, a task or a project.
  self.time_entry.ProjectID = item.ProjectID;
  self.time_entry.TaskID = item.TaskID;
  self.time_entry.Description = item.Text;
  self.time_entry.ProjectAndTaskLabel = item.ProjectAndTaskLabel;
  self.time_entry.ProjectColor = item.ProjectColor;
  self.time_entry.Description = item.Description;

  [self render];
}

- (void)controlTextDidChange:(NSNotification *)aNotification {
  NSComboBox *box = [aNotification object];
  NSString *filter = [box stringValue];

  [self.autocompleteDataSource setFilter:filter];
  [self.descriptionComboBox reloadData];

  // Hide dropdown if filter is empty
  // or nothing was found
  if (!filter || ![filter length] || !self.autocompleteDataSource.count) {
    if ([box isExpanded] == YES) {
      [box setExpanded:NO];
    }
    return;
  }

  if ([box isExpanded] == NO) {
    [box setExpanded:YES];
  }
}

- (void)timerFired:(NSTimer*)timer {
  if (self.time_entry != nil && self.time_entry.duration_in_seconds < 0) {
    char str[duration_str_len];
    kopsik_format_duration_in_seconds_hhmmss(self.time_entry.duration_in_seconds,
                                             str,
                                             duration_str_len);
    NSString *newValue = [NSString stringWithUTF8String:str];
    [self.durationTextField setStringValue:newValue];
  }
}

@end
