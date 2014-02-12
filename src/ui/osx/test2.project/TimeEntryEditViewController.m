//
//  TimeEntryEditViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TimeEntryEditViewController.h"
#import "UIEvents.h"
#import "TimeEntryViewItem.h"
#import "Context.h"
#import "ModelChange.h"
#import "ErrorHandler.h"
#import "AutocompleteItem.h"
#import "AutocompleteDataSource.h"
#import "NSComboBox_Expansion.h"

@interface TimeEntryEditViewController ()
@property NSString *GUID;
@property AutocompleteDataSource *autocompleteDataSource;
@property NSTimer *timerAutocompleteRendering;
@property NSTimer *timer;
@property TimeEntryViewItem *runningTimeEntry;
@end

@implementation TimeEntryEditViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
      self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                               target:self
                                             selector:@selector(timerFired:)
                                             userInfo:nil
                                              repeats:YES];

      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateTimeEntrySelected
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateUserLoggedIn
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventModelChange
                                                 object:nil];

      self.autocompleteDataSource = [[AutocompleteDataSource alloc] init];
    }
    
    return self;
}

- (IBAction)backButtonClicked:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntryDeselected object:nil];
}

- (NSString *)comboBox:(NSComboBox *)comboBox completedString:(NSString *)partialString {
  return [self.autocompleteDataSource completedString:partialString];
}

- (void)render:(NSString *)view_item_guid {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  NSAssert(view_item_guid != nil, @"GUID is nil");
  TimeEntryViewItem *item = [TimeEntryViewItem findByGUID:view_item_guid];
  NSAssert(item != nil, @"View item not found by GUID!");

  self.runningTimeEntry = item;

  NSLog(@"TimeEntryEditViewController render, %@", item);

  if (nil == self.startDate.listener) {
    self.startDate.listener = self;
  }

  // Reset autocomplete filter
  [self.autocompleteDataSource setFilter:@""];
  [self.projectSelect reloadData];

  // Check if TE's can be marked as billable at all
  char err[KOPSIK_ERR_LEN];
  int has_premium_workspaces = 0;
  kopsik_api_result res = kopsik_user_has_premium_workspaces(ctx,
                                                             err,
                                                             KOPSIK_ERR_LEN,
                                                             &has_premium_workspaces);
  if (KOPSIK_API_SUCCESS != res) {
    handle_error(res, err);
    return;
  }

  if (has_premium_workspaces) {
    [self.billableCheckbox setHidden:NO];
  } else {
    [self.billableCheckbox setHidden:YES];
  }
    
  self.GUID = view_item_guid;
  NSAssert(self.GUID != nil, @"GUID is nil");
  
  [self.descriptionTextField setStringValue:item.Description];
  if (item.ProjectAndTaskLabel != nil) {
    [self.projectSelect setStringValue:item.ProjectAndTaskLabel];
  } else {
    [self.projectSelect setStringValue:@""];
  }
  [self.durationTextField setStringValue:item.duration];
  [self.startTime setDateValue:item.started];
  [self.startDate setDateValue:item.started];

  [self.endTime setDateValue:item.ended];

  if (item.duration_in_seconds < 0) {
    [self.startDate setEnabled:NO];
  } else {
    [self.startDate setEnabled:YES];
  }

  [self.endTime setHidden:(item.duration_in_seconds < 0)];
  
  [self.startEndTimeBox setHidden:item.durOnly];

  if (YES == item.billable) {
    [self.billableCheckbox setState:NSOnState];
  } else {
    [self.billableCheckbox setState:NSOffState];
  }
  
  if (item.updatedAt != nil) {
    NSDateFormatter* df_local = [[NSDateFormatter alloc] init];
    [df_local setTimeZone:[NSTimeZone defaultTimeZone]];
    [df_local setDateFormat:@"yyyy.MM.dd 'at' HH:mm:ss"];
    NSString* localDate = [df_local stringFromDate:item.updatedAt];
    NSString *updatedAt = [@"Last update " stringByAppendingString:localDate];
    [self.lastUpdateTextField setStringValue:updatedAt];

    [self.lastUpdateTextField setHidden:NO];
  } else {
    [self.lastUpdateTextField setHidden:YES];
  }
}

- (void)eventHandler: (NSNotification *) notification {
  if ([notification.name isEqualToString:kUIStateTimeEntrySelected]) {
    [self performSelectorOnMainThread:@selector(render:)
                           withObject:notification.object
                        waitUntilDone:NO];
    return;
  }

  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    [self performSelectorOnMainThread:@selector(scheduleAutocompleteRendering)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }

  if ([notification.name isEqualToString:kUIEventModelChange]) {
    ModelChange *mc = notification.object;
    if ([mc.ModelType isEqualToString:@"tag"]) {
      return; // Tags dont affect autocomplete
    }
    [self performSelectorOnMainThread:@selector(scheduleAutocompleteRendering)
                           withObject:nil
                        waitUntilDone:NO];
    if ([self.GUID isEqualToString:mc.GUID] && [mc.ChangeType isEqualToString:@"update"]) {
      [self performSelectorOnMainThread:@selector(render:)
                             withObject:mc.GUID
                          waitUntilDone:NO];
    }
    return;
  }
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

- (void)renderAutocomplete {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  self.timerAutocompleteRendering = nil;

  [self.autocompleteDataSource fetch:NO withTasks:YES withProjects:YES];

  if (self.projectSelect.dataSource == nil) {
    self.projectSelect.usesDataSource = YES;
    self.projectSelect.dataSource = self;
  }
  [self.projectSelect reloadData];
}

- (IBAction)durationTextFieldChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  const char *value = [[self.durationTextField stringValue] UTF8String];
  kopsik_api_result res = kopsik_set_time_entry_duration(ctx,
                                                         err,
                                                         KOPSIK_ERR_LEN,
                                                         [self.GUID UTF8String],
                                                         value);
  if (KOPSIK_API_SUCCESS != res) {
    handle_error(res, err);
    return;
  }
  // FIXME: move into setter method
  kopsik_sync(ctx, 0, handle_error);
}

- (IBAction)projectSelectChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  NSString *key = [self.projectSelect stringValue];
  AutocompleteItem *autocomplete = [self.autocompleteDataSource get:key];
  unsigned int task_id = 0;
  unsigned int project_id = 0;
  if (autocomplete != nil) {
    task_id = autocomplete.TaskID;
    project_id = autocomplete.ProjectID;
  }
  kopsik_api_result res = kopsik_set_time_entry_project(ctx,
                                                        err,
                                                        KOPSIK_ERR_LEN,
                                                        [self.GUID UTF8String],
                                                        task_id,
                                                        project_id);
  if (KOPSIK_API_SUCCESS != res) {
    handle_error(res, err);
    return;
  }
  kopsik_sync(ctx, 0, handle_error);
}

- (IBAction)startTimeChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  [self applyStartTime];
  kopsik_sync(ctx, 0, handle_error);
}

- (IBAction)applyStartTime {
  NSDate *startDate = [self.startDate dateValue];
  NSDate *startTime = [self.startTime dateValue];
  
  unsigned unitFlags = NSYearCalendarUnit | NSMonthCalendarUnit |  NSDayCalendarUnit;
  NSDateComponents *comps = [[NSCalendar currentCalendar] components:unitFlags fromDate:startDate];
  NSDate *combined = [[NSCalendar currentCalendar] dateFromComponents:comps];
  
  unitFlags = NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit;
  comps = [[NSCalendar currentCalendar] components:unitFlags fromDate:startTime];
  combined = [[NSCalendar currentCalendar] dateByAddingComponents:comps toDate:combined options:0];

  NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
  NSLocale *enUSPOSIXLocale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
  [dateFormatter setLocale:enUSPOSIXLocale];
  [dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ssZ"];
  NSString *iso8601String = [dateFormatter stringFromDate:combined];

  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_start_iso_8601(ctx,
                                                                 err,
                                                                 KOPSIK_ERR_LEN,
                                                                 [self.GUID UTF8String],
                                                                 [iso8601String UTF8String])) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
}

- (IBAction)endTimeChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  [self applyEndTime];
  kopsik_sync(ctx, 0, handle_error);
}

- (IBAction)applyEndTime {
  NSDate *startDate = [self.startDate dateValue];
  NSDate *endTime = [self.endTime dateValue];
  
  unsigned unitFlags = NSYearCalendarUnit | NSMonthCalendarUnit |  NSDayCalendarUnit;
  NSDateComponents *comps = [[NSCalendar currentCalendar] components:unitFlags fromDate:startDate];
  NSDate *combined = [[NSCalendar currentCalendar] dateFromComponents:comps];
  
  unitFlags = NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit;
  comps = [[NSCalendar currentCalendar] components:unitFlags fromDate:endTime];
  combined = [[NSCalendar currentCalendar] dateByAddingComponents:comps toDate:combined options:0];
  
  NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
  NSLocale *enUSPOSIXLocale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
  [dateFormatter setLocale:enUSPOSIXLocale];
  [dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ssZ"];
  NSString *iso8601String = [dateFormatter stringFromDate:combined];
  
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_end_iso_8601(ctx,
                                                               err,
                                                               KOPSIK_ERR_LEN,
                                                               [self.GUID UTF8String],
                                                               [iso8601String UTF8String])) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
}

- (IBAction)dateChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  [self applyStartTime];
  if (!self.endTime.isHidden) {
    [self applyEndTime];
  }
  kopsik_sync(ctx, 0, handle_error);
}

- (IBAction)billableCheckBoxClicked:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  int value = 0;
  if (NSOnState == [self.billableCheckbox state]) {
    value = 1;
  }
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_billable(ctx,
                                                           err,
                                                           KOPSIK_ERR_LEN,
                                                           [self.GUID UTF8String],
                                                           value)) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
  kopsik_sync(ctx, 0, handle_error);
}

- (IBAction)descriptionTextFieldChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  NSString *stringValue = [self.descriptionTextField stringValue] ;
  NSLog(@"descriptionTextFieldChanged, stringValue = %@", stringValue);
  const char *value = [stringValue UTF8String];
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_description(ctx,
                                                              err,
                                                              KOPSIK_ERR_LEN,
                                                              [self.GUID UTF8String],
                                                              value)) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
  kopsik_sync(ctx, 0, handle_error);
}

- (IBAction)deleteButtonClicked:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  
  NSAlert *alert = [[NSAlert alloc] init];
  [alert addButtonWithTitle:@"OK"];
  [alert addButtonWithTitle:@"Cancel"];
  [alert setMessageText:@"Delete the time entry?"];
  [alert setInformativeText:@"Deleted time entries cannot be restored."];
  [alert setAlertStyle:NSWarningAlertStyle];
  if ([alert runModal] != NSAlertFirstButtonReturn) {
    return;
  }

  TimeEntryViewItem *item = [TimeEntryViewItem findByGUID:self.GUID];
  NSAssert(item != nil, @"Time entry view item not found when deleting");

  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_delete_time_entry(ctx,
                                                     err,
                                                     KOPSIK_ERR_LEN,
                                                     [self.GUID UTF8String])) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
  kopsik_sync(ctx, 0, handle_error);
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntryDeselected object:nil];
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

- (void)controlTextDidChange:(NSNotification *)aNotification {
  NSComboBox *box = [aNotification object];
  NSString *filter = [box stringValue];
  [self.autocompleteDataSource setFilter:filter];
  [self.projectSelect reloadData];

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

// If duration field is not focused, render ticking time
// into duration field
- (void)timerFired:(NSTimer*)timer {
  if (self.runningTimeEntry == nil || self.runningTimeEntry.duration_in_seconds >= 0) {
    return; // time entry is not running, ignore
  }
  if ([self.durationTextField currentEditor] != nil) {
    return; // duration field is not focused
  }
  if ([self.durationTextField currentEditor] == [[NSApp keyWindow] firstResponder]) {
    return;
  }
  char str[duration_str_len];
  kopsik_format_duration_in_seconds_hhmmss(self.runningTimeEntry.duration_in_seconds,
                                           str,
                                           duration_str_len);
  NSString *newValue = [NSString stringWithUTF8String:str];
  [self.durationTextField setStringValue:newValue];
}

@end

// http://stackoverflow.com/questions/4499262/how-to-programmatically-open-an-nscomboboxs-list

@implementation NSComboBox (ExpansionAPI)

- (BOOL) isExpanded {
  id ax = NSAccessibilityUnignoredDescendant(self);
  return [[ax accessibilityAttributeValue:
           NSAccessibilityExpandedAttribute] boolValue];
}

- (void) setExpanded: (BOOL)expanded {
  id ax = NSAccessibilityUnignoredDescendant(self);
  [ax accessibilitySetValue: [NSNumber numberWithBool: expanded]
               forAttribute: NSAccessibilityExpandedAttribute];
}

@end