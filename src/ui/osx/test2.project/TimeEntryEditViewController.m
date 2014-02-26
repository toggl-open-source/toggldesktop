//
//  TimeEntryEditViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//
#import "EditNotification.h"
#import "TimeEntryEditViewController.h"
#import "UIEvents.h"
#import "TimeEntryViewItem.h"
#import "Context.h"
#import "ModelChange.h"
#import "ErrorHandler.h"
#import "AutocompleteItem.h"
#import "AutocompleteDataSource.h"
#import "NSComboBox_Expansion.h"
#import "ViewItem.h"

@interface TimeEntryEditViewController ()
@property NSString *GUID;
@property AutocompleteDataSource *autocompleteDataSource;
@property NSTimer *timerAutocompleteRendering;
@property NSTimer *timerTagsListRendering;
@property NSTimer *timerClientsListRendering;
@property NSTimer *timerWorkspacesListRendering;
@property NSTimer *timer;
@property TimeEntryViewItem *runningTimeEntry;
@property NSMutableArray *tagsList;
@property NSMutableArray *clientList;
@property NSMutableArray *workspaceList;
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
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateTimeEntryDeselected
                                                 object:nil];

      self.autocompleteDataSource = [[AutocompleteDataSource alloc] init];
    }
    
    return self;
}

- (IBAction)addProjectButtonClicked:(id)sender {
  self.projectNameTextField.stringValue = @"";
  self.clientSelect.stringValue = @"";
  self.workspaceSelect.stringValue = @"";
  [self.addProjectBox setHidden:NO];
  [self.projectSelectBox setHidden:YES];
  [self.projectNameTextField becomeFirstResponder];
}

- (IBAction)backButtonClicked:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntryDeselected
                                                      object:nil];
}

- (IBAction)continueButtonClicked:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandContinue
                                                      object:self.GUID];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntryDeselected
                                                      object:nil];
}

- (NSString *)comboBox:(NSComboBox *)comboBox completedString:(NSString *)partialString {
  if (comboBox == self.projectSelect) {
    return [self.autocompleteDataSource completedString:partialString];
  }
  if (comboBox == self.clientSelect) {
    return @"FIXME:";
  }
  if (comboBox == self.workspaceSelect) {
    return @"FIXME";
  }
  NSAssert(false, @"Invalid combo box");
  return nil;
}

- (void)render:(EditNotification *)edit {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  NSAssert(edit != nil, @"EditNotification is nil");
  NSAssert(edit.EntryGUID != nil, @"EditNotification.GUID is nil");
  NSAssert([edit isKindOfClass:[EditNotification class]], @"EditNotification expected");

  TimeEntryViewItem *item = [TimeEntryViewItem findByGUID:edit.EntryGUID];
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
    handle_error(err);
    return;
  }

  if (has_premium_workspaces) {
    [self.billableCheckbox setHidden:NO];
  } else {
    [self.billableCheckbox setHidden:YES];
  }
    
  self.GUID = edit.EntryGUID;
  NSAssert(self.GUID != nil, @"GUID is nil");

  // Overwrite description only if user is not editing it:
  if ([self.descriptionTextField currentEditor] == nil) {
    [self.descriptionTextField setStringValue:item.Description];
  }
  
  // Overwrite project only if user is not editing it
  if ([self.projectSelect currentEditor] == nil) {
    if (item.ProjectAndTaskLabel != nil) {
      [self.projectSelect setStringValue:item.ProjectAndTaskLabel];
    } else {
      [self.projectSelect setStringValue:@""];
    }
  }

  // Overwrite duration only if user is not editing it:
  if ([self.durationTextField currentEditor] == nil) {
    [self.durationTextField setStringValue:item.duration];
  }

  [self.startTime setDateValue:item.started];
  [self.startDate setDateValue:item.started];

  [self.endTime setDateValue:item.ended];

  if (item.duration_in_seconds < 0) {
    [self.startDate setEnabled:NO];
      [self.continueButton setHidden:YES];
  } else {
    [self.startDate setEnabled:YES];
      [self.continueButton setHidden:NO];
  }

  [self.endTime setHidden:(item.duration_in_seconds < 0)];
  
  [self.startEndTimeBox setHidden:item.durOnly];

  if (YES == item.billable) {
    [self.billableCheckbox setState:NSOnState];
  } else {
    [self.billableCheckbox setState:NSOffState];
  }

  // Overwrite tags only if user is not editing them right now
  if ([self.tagsTokenField currentEditor] == nil) {
    if ([item.tags count] == 0) {
      [self.tagsTokenField setObjectValue:nil];
    } else {
      [self.tagsTokenField setObjectValue:item.tags];
    }
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

  if ([edit.FieldName isEqualToString:kUIDurationClicked]){
    [self.durationTextField becomeFirstResponder];
  }
  if ([edit.FieldName isEqualToString:kUIDescriptionClicked]){
    [self.descriptionTextField becomeFirstResponder];
  }
  
  [self startTagsListRendering];
}

// FIXME: move into lib
- (void)eventHandler: (NSNotification *) notification {
  if ([notification.name isEqualToString:kUIStateTimeEntryDeselected]) {
    [self.addProjectBox setHidden:YES];
    [self.projectSelectBox setHidden:NO];
    return;
  }
  
  if ([notification.name isEqualToString:kUIStateTimeEntrySelected]) {
    [self performSelectorOnMainThread:@selector(render:)
                           withObject:notification.object
                        waitUntilDone:NO];
    return;
  }

  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    [self performSelectorOnMainThread:@selector(startAutocompleteRendering)
                           withObject:nil
                        waitUntilDone:NO];
    [self performSelectorOnMainThread:@selector(startClientSelectRendering)
                           withObject:nil
                        waitUntilDone:NO];
    [self performSelectorOnMainThread:@selector(startWorkspaceSelectRendering)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }

  if ([notification.name isEqualToString:kUIEventModelChange]) {
    ModelChange *mc = notification.object;
    if ([mc.ModelType isEqualToString:@"tag"]) {
      [self startTagsListRendering];
      return; // Tags dont affect autocomplete
    }
    
    [self performSelectorOnMainThread:@selector(startAutocompleteRendering)
                           withObject:nil
                        waitUntilDone:NO];
    

    if ([mc.ModelType isEqualToString:@"workspace"]
        || [mc.ModelType isEqualToString:@"client"]) {
      [self performSelectorOnMainThread:@selector(startClientSelectRendering)
                             withObject:nil
                          waitUntilDone:NO];
    }

    if ([mc.ModelType isEqualToString:@"workspace"]) {
      [self performSelectorOnMainThread:@selector(startWorkspaceSelectRendering)
                             withObject:nil
                          waitUntilDone:NO];
    }
    
    if ([self.GUID isEqualToString:mc.GUID] && [mc.ChangeType isEqualToString:@"update"]) {
      EditNotification *edit = [[EditNotification alloc] init];
      edit.EntryGUID = self.GUID;
      [self performSelectorOnMainThread:@selector(render:)
                             withObject:edit
                          waitUntilDone:NO];
    }
    return;
  }
}

- (NSArray *)tokenField:(NSTokenField *)tokenField
completionsForSubstring:(NSString *)substring
           indexOfToken:(NSInteger)tokenIndex
    indexOfSelectedItem:(NSInteger *)selectedIndex {
  NSMutableArray *filteredCompletions = [NSMutableArray array];

  [self.tagsList enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
    if ([[obj lowercaseString] hasPrefix:[substring lowercaseString]]) {
      [filteredCompletions addObject:obj];
    }
  }];

  return filteredCompletions;
}

- (void) applyTags {
  NSAssert(self.GUID != nil, @"GUID is nil");
  NSAssert(self.tagsTokenField != nil, @"tags field cant be nil");
  NSArray *tag_names = [self.tagsTokenField objectValue];
  const char *value = [[tag_names componentsJoinedByString:@"|"] UTF8String];
  char errmsg[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_tags(ctx,
                                                       errmsg,
                                                       KOPSIK_ERR_LEN,
                                                       [self.GUID UTF8String],
                                                       value)) {
    handle_error(errmsg);
    return;
  }
}

- (void) startTagsListRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  
  if (self.timerTagsListRendering != nil) {
    return;
  }
  @synchronized(self) {
    self.timerTagsListRendering = [NSTimer scheduledTimerWithTimeInterval:kThrottleSeconds
                                                                       target:self
                                                                     selector:@selector(finishTagsListRendering)
                                                                     userInfo:nil
                                                                      repeats:NO];
  }
}

- (void) finishTagsListRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  
  self.timerTagsListRendering = nil;
  
  KopsikViewItem *tag = 0;
  char errmsg[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_tags(ctx, errmsg, KOPSIK_ERR_LEN, &tag)) {
    handle_error(errmsg);
    kopsik_view_item_clear(tag);
    return;
  }
  NSMutableArray *tags = [[NSMutableArray alloc] init];
  while (tag) {
    NSString *tagName = [NSString stringWithUTF8String:tag->Name];
    [self.tagsList addObject:tagName];
    tag = tag->Next;
  }
  kopsik_view_item_clear(tag);
  @synchronized(self) {
    self.tagsList = tags;
  }
}

- (void) startAutocompleteRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  if (self.timerAutocompleteRendering != nil) {
    return;
  }
  @synchronized(self) {
    self.timerAutocompleteRendering = [NSTimer scheduledTimerWithTimeInterval:kThrottleSeconds
                                                                       target:self
                                                                     selector:@selector(finishRenderAutocomplete)
                                                                     userInfo:nil
                                                                      repeats:NO];
  }
}

- (void)finishRenderAutocomplete {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  self.timerAutocompleteRendering = nil;

  [self.autocompleteDataSource fetch:NO withTasks:YES withProjects:YES];

  if (self.projectSelect.dataSource == nil) {
    self.projectSelect.usesDataSource = YES;
    self.projectSelect.dataSource = self;
  }
  [self.projectSelect reloadData];
}

- (void)startWorkspaceSelectRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  
  if (self.timerWorkspacesListRendering != nil) {
    return;
  }
  @synchronized(self) {
    self.timerWorkspacesListRendering = [NSTimer scheduledTimerWithTimeInterval:kThrottleSeconds
                                                                         target:self
                                                                       selector:@selector(finishWorkspaceSelectRendering)
                                                                       userInfo:nil
                                                                        repeats:NO];
   }
}

- (void)finishWorkspaceSelectRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  self.timerWorkspacesListRendering = nil;
  
  KopsikViewItem *first = 0;
  char errmsg[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_workspaces(ctx,
                                              errmsg,
                                              KOPSIK_ERR_LEN,
                                              &first)) {
    kopsik_view_item_clear(first);
    handle_error(errmsg);
    return;
  }

  NSMutableArray *workspaces = [[NSMutableArray alloc] init];
  while (first) {
    ViewItem *workspace = [[ViewItem alloc] init];
    [workspace load:first];
    [workspaces addObject:workspace];
    first = first->Next;
  }
  kopsik_view_item_clear(first);

  @synchronized(self) {
    self.workspaceList = workspaces;
  }

  if (self.workspaceSelect.dataSource == nil) {
    self.workspaceSelect.usesDataSource = YES;
    self.workspaceSelect.dataSource = self;
  }

  [self.workspaceSelect reloadData];
}

- (void)startClientSelectRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  if (self.timerClientsListRendering != nil) {
    return;
  }
  @synchronized(self) {
    self.timerClientsListRendering = [NSTimer scheduledTimerWithTimeInterval:kThrottleSeconds
                                                                       target:self
                                                                    selector:@selector(finishClientSelectRendering)
                                                                     userInfo:nil
                                                                      repeats:NO];
  }
}

- (void)finishClientSelectRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  self.timerClientsListRendering = nil;

  unsigned int workspace_id = 0;
  for (int i = 0; i < self.workspaceList.count; i++ ) {
    ViewItem *workspace = self.workspaceList[i];
    if ([workspace.Name isEqualToString:self.workspaceSelect.stringValue]) {
      workspace_id = workspace.ID;
      break;
    }
  }

  KopsikViewItem *first = 0;
  char errmsg[KOPSIK_ERR_LEN];
  // If no workspace is selected, don't render clients yet.
  if (workspace_id && KOPSIK_API_SUCCESS != kopsik_clients(ctx,
                                                           errmsg,
                                                           KOPSIK_ERR_LEN,
                                                           workspace_id,
                                                           &first)) {
    kopsik_view_item_clear(first);
    handle_error(errmsg);
    return;
  }

  NSMutableArray *clients = [[NSMutableArray alloc] init];
  while (first) {
    ViewItem *client = [[ViewItem alloc] init];
    [client load:first];
    [clients addObject:client];
    first = first->Next;
  }
  kopsik_view_item_clear(first);

  @synchronized(self) {
    self.clientList = clients;
  }

  if (self.clientSelect.dataSource == nil) {
    self.clientSelect.usesDataSource = YES;
    self.clientSelect.dataSource = self;
  }
  [self.clientSelect reloadData];
  
  // Client selection is not mandatory, so don't select a client
  // automatically, when nothing's selected yet.
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
  handle_result(res, err);
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
  handle_result(res, err);
}

- (IBAction)startTimeChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  [self applyStartTime];
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
  kopsik_api_result res = kopsik_set_time_entry_start_iso_8601(ctx,
                                                               err,
                                                               KOPSIK_ERR_LEN,
                                                               [self.GUID UTF8String],
                                                               [iso8601String UTF8String]);
  handle_result(res, err);
}

- (IBAction)endTimeChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  [self applyEndTime];
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
  kopsik_api_result res = kopsik_set_time_entry_end_iso_8601(ctx,
                                                             err,
                                                             KOPSIK_ERR_LEN,
                                                             [self.GUID UTF8String],
                                                             [iso8601String UTF8String]);
  handle_result(res, err);
}

- (IBAction)dateChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  [self applyStartTime];
  if (!self.endTime.isHidden) {
    [self applyEndTime];
  }
}

- (IBAction)tagsChanged:(id)sender {
  [self applyTags];
}

- (IBAction)billableCheckBoxClicked:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  int value = 0;
  if (NSOnState == [self.billableCheckbox state]) {
    value = 1;
  }
  kopsik_api_result res = kopsik_set_time_entry_billable(ctx,
                                                         err,
                                                         KOPSIK_ERR_LEN,
                                                         [self.GUID UTF8String],
                                                         value);
  handle_result(res, err);
}

- (IBAction)descriptionTextFieldChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  NSString *stringValue = [self.descriptionTextField stringValue] ;
  NSLog(@"descriptionTextFieldChanged, stringValue = %@", stringValue);
  const char *value = [stringValue UTF8String];
  kopsik_api_result res = kopsik_set_time_entry_description(ctx,
                                                            err,
                                                            KOPSIK_ERR_LEN,
                                                            [self.GUID UTF8String],
                                                            value);
  handle_result(res, err);
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
  kopsik_api_result res = kopsik_delete_time_entry(ctx,
                                                   err,
                                                   KOPSIK_ERR_LEN,
                                                   [self.GUID UTF8String]);
  if (res != KOPSIK_API_SUCCESS) {
    handle_error(err);
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntryDeselected object:nil];
}

-(NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox{
  if (self.projectSelect == aComboBox) {
    return [self.autocompleteDataSource count];
  }
  if (self.clientSelect == aComboBox) {
    return [self.clientList count];
  }
  if (self.workspaceSelect == aComboBox) {
    return [self.workspaceList count];
  }
  NSAssert(false, @"Invalid combo box");
  return 0;
}

-(id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)row{
  if (self.projectSelect == aComboBox) {
    return [self.autocompleteDataSource keyAtIndex:row];
  }
  if (self.clientSelect == aComboBox) {
    ViewItem *client = [self.clientList objectAtIndex:row];
    return client.Name;
  }
  if (self.workspaceSelect == aComboBox) {
    ViewItem *workspace = [self.workspaceList objectAtIndex:row];
    return workspace.Name;
  }
  NSAssert(false, @"Invalid combo box");
  return nil;
}

- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)aString {
  if (self.projectSelect == aComboBox) {
    return [self.autocompleteDataSource indexOfKey:aString];
  }
  if (self.clientSelect == aComboBox) {
    for (int i = 0; i < self.clientList.count; i++) {
      ViewItem *client = [self.clientList objectAtIndex:i];
      if ([client.Name isEqualToString:aString]) {
        return i;
      }
    }
    return -1;
  }
  if (self.workspaceSelect == aComboBox) {
    for (int i = 0; i < self.workspaceList.count; i++) {
      ViewItem *workspace = [self.workspaceList objectAtIndex:i];
      if ([workspace.Name isEqualToString:aString]) {
        return i;
      }
    }
    return -1;
  }
  NSAssert(false, @"Invalid combo box");
  return -1;
}

- (void)controlTextDidEndEditing:(NSNotification *)aNotification {
  if (![[aNotification object] isKindOfClass:[NSTokenField class]]) {
    return;
  }
  [self applyTags];
}

- (void)controlTextDidChange:(NSNotification *)aNotification {
  // FIXME: this looks kind of similar to code in timer edit view

  // Don't trigger combobox autocomplete when inside tags field
  if (![[aNotification object] isKindOfClass:[NSComboBox class]]) {
    return;
  }

  NSComboBox *box = [aNotification object];
  NSString *filter = [box stringValue];

  [self.autocompleteDataSource setFilter:filter];
  [self.projectSelect reloadData];

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

// If duration field is not focused, render ticking time
// into duration field
- (void)timerFired:(NSTimer*)timer {
  if (self.runningTimeEntry == nil || self.runningTimeEntry.duration_in_seconds >= 0) {
    return; // time entry is not running, ignore
  }
  if ([self.durationTextField currentEditor] != nil) {
    return; // duration field is focussed by user, don't mess with it
  }
  char str[duration_str_len];
  kopsik_format_duration_in_seconds_hhmmss(self.runningTimeEntry.duration_in_seconds,
                                           str,
                                           duration_str_len);
  NSString *newValue = [NSString stringWithUTF8String:str];
  [self.durationTextField setStringValue:newValue];
}

- (IBAction)workspaceSelectChanged:(id)sender {
  NSLog(@"workspaceSelectChanged");
  // Changing workspace should render the clients
  // of the selected workspace in the client select combobox.
  [self startClientSelectRendering];
}

- (IBAction)clientSelectChanged:(id)sender {
  NSLog(@"clientSelectChanged");
  // Changing client does not change anything in new project view.
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
