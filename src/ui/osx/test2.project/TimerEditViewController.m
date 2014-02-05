
//
//  TimerEditViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TimerEditViewController.h"
#import "UIEvents.h"
#import "AutocompleteItem.h"
#import "Context.h"
#import "ErrorHandler.h"
#import "AutocompleteDataSource.h"
#import "ConvertHexColor.h"
#import "NewTimeEntry.h"
#import "ModelChange.h"
#import "NSComboBox_Expansion.h"
#import "TimeEntryViewItem.h"

@interface TimerEditViewController ()
@property AutocompleteDataSource *autocompleteDataSource;
@property NewTimeEntry *next_time_entry;
@property TimeEntryViewItem *running_time_entry;
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

      self.next_time_entry = [[NewTimeEntry alloc] init];
      
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

  NSLog(@"TimerEditViewController renderAutocomplete");

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
  if ([notification.name isEqualToString:kUIStateTimerRunning]) {
    self.running_time_entry = notification.object;
    [self performSelectorOnMainThread:@selector(render)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }

  if ([notification.name isEqualToString:kUIStateTimerStopped]) {
    self.running_time_entry = nil;
    [self performSelectorOnMainThread:@selector(render)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }

  if ([notification.name isEqualToString:kUICommandEditRunningTimeEntry]) {
    if (self.running_time_entry != nil) {
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntrySelected
                                                          object:self.running_time_entry.GUID];
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
    if ([change.GUID isEqualToString:self.running_time_entry.GUID]) {
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerStopped
                                                          object:nil];
    }
    return;
  }

  // Handle update
  TimeEntryViewItem *updated = [TimeEntryViewItem findByGUID:change.GUID];

  // Time entry we thought was running, has been stopped.
  if ((updated.duration_in_seconds >= 0) &&
      [updated.GUID isEqualToString:self.running_time_entry.GUID]) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerStopped
                                                        object:nil];
    return;
  }

  // Time entry we did not know was running, is running.
  if ((updated.duration_in_seconds < 0) &&
      ![updated.GUID isEqualToString:self.running_time_entry.GUID]) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerRunning
                                                        object:updated];
    return;
  }

  // Time entry is still running and needs to be updated.
  if ((updated.duration_in_seconds < 0) &&
      [updated.GUID isEqualToString:self.running_time_entry.GUID]) {
    self.running_time_entry = updated;
    [self performSelectorOnMainThread:@selector(render)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }
}

- (void) render {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  
  NSLog(@"Timer edit view render");
  
  // Start/stop button title and color depend on
  // whether time entry is running
  if (self.running_time_entry != nil) {
    self.startButtonLabelTextField.stringValue = @"Stop";
    self.startButtonBox.borderColor = [ConvertHexColor hexCodeToNSColor:@"#ec0000"];
    self.startButtonBox.fillColor = [ConvertHexColor hexCodeToNSColor:@"#ec0000"];
  } else {
    self.startButtonLabelTextField.stringValue = @"Start";
    self.startButtonBox.borderColor = [ConvertHexColor hexCodeToNSColor:@"#4bc800"];
    self.startButtonBox.fillColor = [ConvertHexColor hexCodeToNSColor:@"#4bc800"];
  }

  // Description and duration cannot be edited
  // while time entry is running
  if (self.running_time_entry != nil) {
    [self.descriptionComboBox setEnabled:NO];
    [self.durationTextField setEnabled:NO];
  } else {
    [self.descriptionComboBox setEnabled:YES];
    [self.durationTextField setEnabled:YES];
  }
  
  // Display description
  if (self.running_time_entry != nil) {
    self.descriptionComboBox.stringValue = self.running_time_entry.Description;
  } else {
    self.descriptionComboBox.stringValue = self.next_time_entry.Description;
  }
  
  // If a project is assigned, then project name
  // is visible.
  if (self.next_time_entry.ProjectID) {
    [self.projectTextField setHidden:NO];
  } else {
    [self.projectTextField setHidden:YES];
  }
  
  // Display project name
  if (self.running_time_entry != nil) {
    self.projectTextField.stringValue =
      [self.running_time_entry.ProjectAndTaskLabel uppercaseString];
  } else {
    self.projectTextField.stringValue = @"FIXME: project name autocomplete?";
  }

  // If a project is selected then description
  // field is higher on the screen.
  NSPoint pt;
  pt.x = self.descriptionComboBox.frame.origin.x;
  if (self.next_time_entry.ProjectID) {
    pt.y = 16;
  } else {
    pt.y = 8;
  }
  [self.descriptionComboBox setFrameOrigin:pt];
  
  // Display duration
  if (self.running_time_entry != nil) {
    self.durationTextField.stringValue = self.running_time_entry.duration;
  } else {
    self.durationTextField.stringValue = self.next_time_entry.Duration;
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
  if (self.running_time_entry != nil) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandStop
                                                        object:nil];
    return;
  }

  self.next_time_entry.Duration = self.durationTextField.stringValue;
  self.next_time_entry.Description = self.descriptionComboBox.stringValue;
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandNew
                                                      object:self.next_time_entry];
  self.next_time_entry = [[NewTimeEntry alloc] init];

  // Reset autocomplete filter
  [self.autocompleteDataSource setFilter:@""];
  [self.descriptionComboBox reloadData];
}

- (IBAction)descriptionComboBoxChanged:(id)sender {

  NSString *key = [self.descriptionComboBox stringValue];
  AutocompleteItem *item = [self.autocompleteDataSource get:key];

  // User has entered free text
  if (item == nil) {
    self.next_time_entry.Description = [self.descriptionComboBox stringValue];
    return;
  }

  // User has selected a autocomplete item.
  // It could be a time entry, a task or a project.
  self.next_time_entry.ProjectID = item.ProjectID;
  self.next_time_entry.TaskID = item.TaskID;

  NSLog(@"New time entry desc: %@, task ID: %u, project ID: %u",
        self.next_time_entry.Description,
        self.next_time_entry.TaskID,
        self.next_time_entry.ProjectID);
  
  [self render];
}

- (void)controlTextDidChange:(NSNotification *)aNotification {
  NSComboBox *box = [aNotification object];
  NSString *filter = [box stringValue];
  [self.autocompleteDataSource setFilter:filter];
  [self.descriptionComboBox reloadData];

  if (filter == nil || [filter length] == 0) {
    if ([box isExpanded] == YES) {
      [box setExpanded:NO];
    }
  } else {
    if ([box isExpanded] == NO) {
      [box setExpanded:YES];
    }
  }
}

- (void)timerFired:(NSTimer*)timer {
  if (self.running_time_entry != nil) {
    char str[duration_str_len];
    kopsik_format_duration_in_seconds_hhmmss(self.running_time_entry.duration_in_seconds,
                                             str,
                                             duration_str_len);
    NSString *newValue = [NSString stringWithUTF8String:str];
    [self.durationTextField setStringValue:newValue];
  }
}

@end
