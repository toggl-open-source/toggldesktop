//
//  TimeEntryEditViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
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
#import "NSCustomComboBoxCell.h"
#import "NSCustomComboBox.h"
#import "User.h"

@interface TimeEntryEditViewController ()
@property NSString *GUID;
@property AutocompleteDataSource *projectAutocompleteDataSource;
@property AutocompleteDataSource *descriptionComboboxDataSource;
@property NSTimer *timerProjectAutocompleteRendering;
@property NSTimer *timerDescriptionComboboxRendering;
@property NSTimer *timerTagsListRendering;
@property NSTimer *timerClientsListRendering;
@property NSTimer *timerWorkspacesListRendering;
@property NSTimer *timerMenubarTimer;
@property TimeEntryViewItem *runningTimeEntry;
@property NSMutableArray *tagsList;
@property NSMutableArray *clientList;
@property NSMutableArray *workspaceList;
@property NSArray *topConstraint;
@property NSLayoutConstraint *addProjectBoxHeight;
@property NSDateFormatter *format;
@property NSDate *startTimeDate;
@property NSDate *endTimeDate;
@property User *userinfo;
@end

@implementation TimeEntryEditViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
      self.timerMenubarTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
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
      self.format = [[NSDateFormatter alloc] init];

      self.projectAutocompleteDataSource = [[AutocompleteDataSource alloc] init];

      self.descriptionComboboxDataSource = [[AutocompleteDataSource alloc] init];
    }
    
    return self;
}

- (IBAction)addProjectButtonClicked:(id)sender {
  self.projectNameTextField.stringValue = @"";
  self.clientSelect.stringValue = @"";
  self.workspaceSelect.stringValue = @"";
  [self.descriptionCombobox setNextKeyView:self.projectNameTextField];

  if (!self.addProjectBoxHeight) {
    self.addProjectBoxHeight = [NSLayoutConstraint constraintWithItem:self.addProjectBox
                                                        attribute:NSLayoutAttributeHeight
                                                        relatedBy:NSLayoutRelationEqual
                                                           toItem:nil
                                                         attribute:NSLayoutAttributeNotAnAttribute
                                                       multiplier:1
                                                         constant:129];
    [self.view addConstraint: self.addProjectBoxHeight];
  }

  // If user has only one workspace, do not show the workspace combobox at all.
  BOOL singleWorkspace = YES;
  if (self.workspaceList.count > 1) {
    singleWorkspace = NO;
    self.addProjectBoxHeight.constant = 129;
  } else {
    self.addProjectBoxHeight.constant = 96;
  }
  [self.workspaceLabel setHidden:singleWorkspace];
  [self.workspaceSelect setHidden:singleWorkspace];

  // Pre-select the workspace that the time entry is tracked to
  TimeEntryViewItem *te = [TimeEntryViewItem findByGUID:self.GUID];
  if (te && te.WorkspaceID) {
    for (int i = 0; i < self.workspaceList.count; i++) {
      ViewItem *workspace = self.workspaceList[i];
      if (workspace.ID == te.WorkspaceID) {
        self.workspaceSelect.stringValue = workspace.Name;
        break;
      }
    }
  }

  [self.addProjectBox setHidden:NO];

  NSDictionary *viewsDict = NSDictionaryOfVariableBindings(_addProjectBox, _dataholderBox);
  self.topConstraint = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[_addProjectBox]-0-[_dataholderBox]"
                                             options:0
                                             metrics:nil
                                               views:viewsDict];
  [self.view addConstraints:self.topConstraint];

  [self.projectSelectBox setHidden:YES];
  [self.addProjectButton setHidden:YES];
  [self.projectNameTextField becomeFirstResponder];
}

- (IBAction)backButtonClicked:(id)sender {
  if (self.topConstraint){
    [self.view removeConstraints:self.topConstraint];
    self.topConstraint = nil;
  }
  [self.descriptionCombobox setNextKeyView:self.projectSelect];
  // This is not a good place for this (on Done button!)
  if (![self applyAddProject]) {
    return;
  }

  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntryDeselected
                                                      object:nil];
}

// Returns NO if there's an error and UI should not go out of the add project
// mode.
- (BOOL)applyAddProject {
  if (self.addProjectBox.isHidden) {
    return YES;
  }
  NSString *projectName = self.projectNameTextField.stringValue;
  if (!projectName || !projectName.length) {
    return YES;
  }
  _Bool is_private = true;
  if (NSOnState == [self.projectPublicCheckbox state]) {
    is_private = false;
  }
  uint64_t workspaceID = [self selectedWorkspaceID];
  if (!workspaceID) {
    [self.workspaceSelect becomeFirstResponder];
    return NO;
  }
  uint64_t clientID = [self selectedClientID];

  // A new project is being added!
  KopsikViewItem *project = 0;
  if (!kopsik_add_project(ctx,
                          workspaceID,
                          clientID,
                          [projectName UTF8String],
                          is_private,
                          &project)) {
    kopsik_view_item_clear(project);
    return NO;
  }
  if (!kopsik_set_time_entry_project(ctx,
                                     [self.GUID UTF8String],
                                     0,
                                     project->ID,
                                     project->GUID)) {
    kopsik_view_item_clear(project);
    return NO;
  }
  kopsik_view_item_clear(project);
  return YES;
}

- (IBAction)continueButtonClicked:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandContinue
                                                      object:self.GUID];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntryDeselected
                                                      object:nil];
}

- (NSString *)comboBox:(NSComboBox *)comboBox completedString:(NSString *)partialString {
  if (comboBox == self.descriptionCombobox) {
    return [self.descriptionComboboxDataSource completedString:partialString];
  }
  if (comboBox == self.projectSelect) {
    return [self.projectAutocompleteDataSource completedString:partialString];
  }
  if (comboBox == self.clientSelect) {
    return @":"; // Not supported at the moment
  }
  if (comboBox == self.workspaceSelect) {
    return @""; // Not supported at the moment
  }
  NSAssert(false, @"Invalid combo box");
  return nil;
}

- (void)viewDidLoad {
 // Setting button text color to blue
    NSColor *color = [NSColor alternateSelectedControlColor];
    NSMutableAttributedString *colorTitle =
    [[NSMutableAttributedString alloc] initWithAttributedString:[self.addProjectButton attributedTitle]];

    NSRange titleRange = NSMakeRange(0, [colorTitle length]);

    [colorTitle addAttribute:NSForegroundColorAttributeName
                     value:color
                     range:titleRange];

    [self.addProjectButton setAttributedTitle:colorTitle];
}

- (void)loadView {
    [super loadView];
    [self viewDidLoad];
}

- (void)render:(EditNotification *)edit {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  NSAssert(edit != nil, @"EditNotification is nil");

  if (nil == edit.GUID) {
    NSLog(@"Cannot render, EditNotification.GUID is nil");
    return;
  }

  NSAssert([edit isKindOfClass:[EditNotification class]], @"EditNotification expected");

  TimeEntryViewItem *item = [TimeEntryViewItem findByGUID:edit.GUID];
  if (nil == item) {
    NSLog(@"Cannot render, time entry not found by GUID %@", edit.GUID);
    return;
  }

  self.runningTimeEntry = item;

  NSLog(@"TimeEntryEditViewController render, %@", item);

  if (nil == self.startDate.listener) {
    self.startDate.listener = self;
  }

  // Reset project autocomplete filter
  [self.projectAutocompleteDataSource setFilter:@""];
  [self.projectSelect reloadData];

  // Reset description autocomplete filter
  [self.descriptionComboboxDataSource setFilter:@""];
  [self.descriptionCombobox reloadData];

  // Check if TE's can be marked as billable at all
  _Bool can_see_billable = false;
  if (!kopsik_user_can_see_billable_flag(ctx,
                                         [item.GUID UTF8String],
                                         &can_see_billable)) {
    return;
  }

  if (can_see_billable) {
    [self.billableCheckbox setHidden:NO];
  } else {
    [self.billableCheckbox setHidden:YES];
  }

  if (item.billable) {
    [self.billableCheckbox setState:NSOnState];
  } else {
    [self.billableCheckbox setState:NSOffState];
  }

  // Check if user can add projects
  _Bool can_add_projects = false;
  if (!kopsik_user_can_add_projects(ctx,
                                    item.WorkspaceID,
                                    &can_add_projects)) {
    return;
  }
  if (!can_add_projects) {
    [self.addProjectButton setHidden:YES];
  } else if ([self.addProjectBox isHidden]) {
    [self.addProjectButton setHidden:NO];
  }

  self.GUID = edit.GUID;
  NSAssert(self.GUID != nil, @"GUID is nil");

  // Overwrite description only if user is not editing it:
  if ([self.descriptionCombobox currentEditor] == nil) {
    [self.descriptionCombobox setStringValue:item.Description];
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

  if ([self.startTime currentEditor] == nil) {
    [self.startTime setStringValue:[[_format stringFromDate:item.started] uppercaseString]];
  }

  if ([self.endTime currentEditor] == nil) {
    [self.endTime setStringValue:[[_format stringFromDate:item.ended] uppercaseString]];
  }

  [self.startDate setDateValue:item.started];
  self.startTimeDate = item.started;
  self.endTimeDate = item.ended;

  if (item.duration_in_seconds < 0) {
    [self.startDate setEnabled:NO];
      [self.continueButton setHidden:YES];
  } else {
    [self.startDate setEnabled:YES];
      [self.continueButton setHidden:NO];
  }

  [self.endTime setHidden:(item.duration_in_seconds < 0)];
  
  [self.startEndTimeBox setHidden:item.durOnly];

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
    [self.descriptionCombobox becomeFirstResponder];
  }
  
  [self startTagsListRendering];
}

- (void)eventHandler: (NSNotification *) notification {
  if ([notification.name isEqualToString:kUIStateTimeEntryDeselected]) {
    [self.addProjectBox setHidden:YES];
    [self.projectSelectBox setHidden:NO];
    [self.projectPublicCheckbox setState:NSOffState];
    return;
  }

  if ([notification.name isEqualToString:kUIStateTimeEntrySelected]) {
    [self performSelectorOnMainThread:@selector(render:)
                           withObject:notification.object
                        waitUntilDone:NO];
    return;
  }

  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    [self performSelectorOnMainThread:@selector(startDescriptionComboboxRendering)
                           withObject:nil
                        waitUntilDone:NO];
    [self performSelectorOnMainThread:@selector(startProjectAutocompleteRendering)
                           withObject:nil
                        waitUntilDone:NO];
    [self performSelectorOnMainThread:@selector(startWorkspaceSelectRendering)
                           withObject:nil
                        waitUntilDone:NO];

    self.userinfo = notification.object;

    if ([self.userinfo.timeOfDayFormat isEqualToString:@"H:mm"]){
      [self.format setDateFormat:@"HH:mm"];
    } else {
      [self.format setDateFormat:@"HH:mm a"];
    }
    return;
  }

  if ([notification.name isEqualToString:kUIEventModelChange]) {
    ModelChange *mc = notification.object;
    if ([mc.ModelType isEqualToString:@"tag"]) {
      [self performSelectorOnMainThread:@selector(startTagsListRendering)
                             withObject:nil
                          waitUntilDone:NO];
      return; // Tags dont affect autocompletes
    }
    
    [self performSelectorOnMainThread:@selector(startProjectAutocompleteRendering)
                           withObject:nil
                        waitUntilDone:NO];
    
    [self performSelectorOnMainThread:@selector(startDescriptionComboboxRendering)
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
      edit.GUID = self.GUID;
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
  if (nil == self.GUID) {
    NSLog(@"Cannot apply tags, self.GUID is nil");
    return;
  }
  NSAssert(self.tagsTokenField != nil, @"tags field cant be nil");
  NSArray *tag_names = [self.tagsTokenField objectValue];
  const char *value = [[tag_names componentsJoinedByString:@"|"] UTF8String];
  if (!kopsik_set_time_entry_tags(ctx,
                                  [self.GUID UTF8String],
                                  value)) {
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
  if (!kopsik_tags(ctx, &tag)) {
    kopsik_view_item_clear(tag);
    return;
  }
  NSMutableArray *tags = [[NSMutableArray alloc] init];
  while (tag) {
    NSString *tagName = [NSString stringWithUTF8String:tag->Name];
    [tags addObject:tagName];
    tag = tag->Next;
  }
  kopsik_view_item_clear(tag);
  @synchronized(self) {
    self.tagsList = tags;
  }
}

- (void) startProjectAutocompleteRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  if (self.timerProjectAutocompleteRendering != nil) {
    return;
  }
  @synchronized(self) {
    self.timerProjectAutocompleteRendering = [NSTimer scheduledTimerWithTimeInterval:kThrottleSeconds
                                                                       target:self
                                                                     selector:@selector(finishProjectAutocompleteRendering)
                                                                     userInfo:nil
                                                                      repeats:NO];
  }
}

- (void)finishProjectAutocompleteRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  self.timerProjectAutocompleteRendering = nil;

  [self.projectAutocompleteDataSource fetch:NO withTasks:YES withProjects:YES];

  if (self.projectSelect.dataSource == nil) {
    self.projectSelect.usesDataSource = YES;
    self.projectSelect.dataSource = self;
  }
  [self.projectSelect reloadData];
}

- (void)startDescriptionComboboxRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  if (self.timerDescriptionComboboxRendering != nil) {
    return;
  }
  @synchronized(self) {
    self.timerDescriptionComboboxRendering = [NSTimer scheduledTimerWithTimeInterval:kThrottleSeconds
                                                                              target:self
                                                                            selector:@selector(finishDescriptionComboboxRendering)
                                                                            userInfo:nil
                                                                             repeats:NO];
  }
}

- (void)finishDescriptionComboboxRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  
  self.timerDescriptionComboboxRendering = nil;
  
  [self.descriptionComboboxDataSource fetch:YES withTasks:YES withProjects:YES];
  
  if (self.descriptionCombobox.dataSource == nil) {
    self.descriptionCombobox.usesDataSource = YES;
    self.descriptionCombobox.dataSource = self;
  }
  [self.descriptionCombobox reloadData];
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
  if (!kopsik_workspaces(ctx,
                         &first)) {
    kopsik_view_item_clear(first);
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

  NSString *workspaceName = [self.workspaceSelect stringValue];

  // If no workspace is selected, attempt to select the user's
  // default workspace.
  if (!workspaceName.length && self.workspaceList.count) {
    uint64_t default_wid = 0;
    if (!kopsik_users_default_wid(ctx, &default_wid)) {
      return;
    }
    for (int i = 0; i < self.workspaceList.count; i++) {
      ViewItem *workspace = self.workspaceList[i];
      if (workspace.ID == default_wid) {
        workspaceName = workspace.Name;
        break;
      }
    }
  }

  // If user has no default workspace available, select the first WS in the
  // workspace list.
  if (!workspaceName.length && self.workspaceList.count) {
    ViewItem *workspace = self.workspaceList[0];
    workspaceName = workspace.Name;
  }
  [self.workspaceSelect setStringValue:workspaceName];

  [self performSelectorOnMainThread:@selector(startClientSelectRendering)
                         withObject:nil
                      waitUntilDone:NO];
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

- (uint64_t)selectedWorkspaceID {
  for (int i = 0; i < self.workspaceList.count; i++ ) {
    ViewItem *workspace = self.workspaceList[i];
    if ([workspace.Name isEqualToString:self.workspaceSelect.stringValue]) {
      return workspace.ID;
    }
  }
  return 0;
}

- (uint64_t)selectedClientID {
  for (int i = 0; i < self.clientList.count; i++ ) {
    ViewItem *client = self.clientList[i];
    if ([client.Name isEqualToString:self.clientSelect.stringValue]) {
      return client.ID;
    }
  }
  return 0;
}

- (void)finishClientSelectRendering {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  self.timerClientsListRendering = nil;

  uint64_t workspace_id = [self selectedWorkspaceID];

  KopsikViewItem *first = 0;
  // If no workspace is selected, don't render clients yet.
  if (workspace_id && !kopsik_clients(ctx,
                                      workspace_id,
                                      &first)) {
    kopsik_view_item_clear(first);
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
  if (nil == self.GUID) {
    NSLog(@"Cannot apply duration text field changes, self.GUID is nil");
    return;
  }

  const char *value = [[self.durationTextField stringValue] UTF8String];
  kopsik_set_time_entry_duration(ctx, [self.GUID UTF8String], value);
}

- (IBAction)projectSelectChanged:(id)sender {
  if (nil == self.GUID) {
    NSLog(@"Cannot apply project selection changes, self.GUID is nil");
    return;
  }

  [self.projectSelect.cell setCalculatedMaxWidth:0];

  NSString *key = [self.projectSelect stringValue];
  AutocompleteItem *autocomplete = [self.projectAutocompleteDataSource get:key];
  uint64_t task_id = 0;
  uint64_t project_id = 0;
  if (autocomplete != nil) {
    task_id = autocomplete.TaskID;
    project_id = autocomplete.ProjectID;
  }
  kopsik_set_time_entry_project(ctx, [self.GUID UTF8String], task_id, project_id, 0);
}

/*
    Returns whether or not an NSString represents a numeric value.
    For more info see:  http://appliedsoftwaredesign.com/blog/iphone-sdk-nsstring-numeric/
*/
-(bool) isNumeric:(NSString*) checkText
{
  NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
  NSNumber* number = [numberFormatter numberFromString:checkText];
  if (number != nil) {
    return true;
  }
  return false;
}

- (NSDateComponents*)parseTime:(NSTextField*)field current:(NSDateComponents*)component {
  int hours = 0;
  int minutes = 0;
  if (!kopsik_parse_time([[field stringValue] UTF8String], &hours, &minutes)) {
    if (field == self.startTime) {
      [field setStringValue:[[self.format stringFromDate:self.startTimeDate] uppercaseString]];
    } else if (field == self.endTime) {
      [field setStringValue:[[self.format stringFromDate:self.endTimeDate] uppercaseString]];
    }
    return component;
  }

  [component setHour:   hours];
  [component setMinute: minutes];

  return component;
}

- (IBAction)startTimeChanged:(id)sender {
  if (nil == self.GUID) {
    NSLog(@"Cannot apply start time change, self.GUID is nil");
    return;
  }

  [self applyStartTime];
}

- (IBAction)applyStartTime {
  NSDate *startDate = [self.startDate dateValue];
  
  unsigned unitFlags = NSYearCalendarUnit | NSMonthCalendarUnit |  NSDayCalendarUnit;
  NSDateComponents *comps = [[NSCalendar currentCalendar] components:unitFlags fromDate:startDate];
  NSDate *combined = [[NSCalendar currentCalendar] dateFromComponents:comps];
  
  unitFlags = NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit;
  comps = [[NSCalendar currentCalendar] components:unitFlags fromDate:self.startTimeDate];
  comps = [self parseTime:self.startTime current:comps];

  combined = [[NSCalendar currentCalendar] dateByAddingComponents:comps toDate:combined options:0];

  NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
  NSLocale *enUSPOSIXLocale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
  [dateFormatter setLocale:enUSPOSIXLocale];
  [dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ssZ"];
  NSString *iso8601String = [dateFormatter stringFromDate:combined];
  self.startTimeDate = combined;

  kopsik_set_time_entry_start_iso_8601(ctx, [self.GUID UTF8String], [iso8601String UTF8String]);
}

- (IBAction)endTimeChanged:(id)sender {
  if (nil == self.GUID) {
    NSLog(@"Cannot apply end time change, self.GUID is nil");
    return;
  }

  [self applyEndTime];
}

- (IBAction)applyEndTime {
  NSDate *startDate = [self.startDate dateValue];
  
  unsigned unitFlags = NSYearCalendarUnit | NSMonthCalendarUnit |  NSDayCalendarUnit;
  NSDateComponents *comps = [[NSCalendar currentCalendar] components:unitFlags fromDate:startDate];
  NSDate *combined = [[NSCalendar currentCalendar] dateFromComponents:comps];
  
  unitFlags = NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit;
  comps = [[NSCalendar currentCalendar] components:unitFlags fromDate:self.endTimeDate];
  comps = [self parseTime:self.endTime current:comps];
  combined = [[NSCalendar currentCalendar] dateByAddingComponents:comps toDate:combined options:0];
  
  NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
  NSLocale *enUSPOSIXLocale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
  [dateFormatter setLocale:enUSPOSIXLocale];
  [dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ssZ"];
  NSString *iso8601String = [dateFormatter stringFromDate:combined];
  
  kopsik_set_time_entry_end_iso_8601(ctx, [self.GUID UTF8String], [iso8601String UTF8String]);
}

- (IBAction)dateChanged:(id)sender {
  if (nil == self.GUID) {
    NSLog(@"Cannot apply date change, self.GUID is nil");
    return;
  }

  [self applyStartTime];
  if (!self.endTime.isHidden) {
    [self applyEndTime];
  }
}

- (IBAction)tagsChanged:(id)sender {
  [self applyTags];
}

- (IBAction)billableCheckBoxClicked:(id)sender {
  if (nil == self.GUID) {
    NSLog(@"Cannot apply billable checkbox change, self.GUID is nil");
    return;
  }

  _Bool value = false;
  if (NSOnState == [self.billableCheckbox state]) {
    value = true;
  }
  kopsik_set_time_entry_billable(ctx, [self.GUID UTF8String], value);
}

- (IBAction)descriptionComboboxChanged:(id)sender {
  if (nil == self.GUID) {
    NSLog(@"Cannot apply description change, self.GUID is nil");
    return;
  }

  NSString *key = [self.descriptionCombobox stringValue];

  NSLog(@"descriptionComboboxChanged, stringValue = %@", key);

  [self.descriptionCombobox.cell setCalculatedMaxWidth:0];

  AutocompleteItem *autocomplete =
    [self.descriptionComboboxDataSource get:key];

  if (!autocomplete) {
    kopsik_set_time_entry_description(ctx,
                                      [self.GUID UTF8String],
                                      [key UTF8String]);
    return;
  }

  if (!kopsik_set_time_entry_project(ctx,
                                     [self.GUID UTF8String],
                                     autocomplete.TaskID,
                                     autocomplete.ProjectID,
                                     0)) {
    return;
  }

  self.descriptionCombobox.stringValue = autocomplete.Description;
  kopsik_set_time_entry_description(ctx, [self.GUID UTF8String], [autocomplete.Description UTF8String]);
}

- (IBAction)deleteButtonClicked:(id)sender {
  if (nil == self.GUID) {
    NSLog(@"Cannot delete time entry, self.GUID is nil");
    return;
  }
  
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
  if (nil == item) {
    NSLog(@"Cannot delete time entry, not found by GUID %@", self.GUID);
    return;
  }

  if (!kopsik_delete_time_entry(ctx, [self.GUID UTF8String])) {
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntryDeselected object:nil];
}

-(NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox{
  if (self.descriptionCombobox == aComboBox) {
    return [self.descriptionComboboxDataSource count];
  }
  if (self.projectSelect == aComboBox) {
    return [self.projectAutocompleteDataSource count];
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
  if (self.descriptionCombobox == aComboBox) {
    return [self.descriptionComboboxDataSource keyAtIndex:row];
  }
  if (self.projectSelect == aComboBox) {
    return [self.projectAutocompleteDataSource keyAtIndex:row];
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
  if (self.descriptionCombobox == aComboBox) {
    return [self.descriptionComboboxDataSource indexOfKey:aString];
  }
  if (self.projectSelect == aComboBox) {
    return [self.projectAutocompleteDataSource indexOfKey:aString];
  }
  if (self.clientSelect == aComboBox) {
    for (int i = 0; i < self.clientList.count; i++) {
      ViewItem *client = [self.clientList objectAtIndex:i];
      if ([client.Name isEqualToString:aString]) {
        return i;
      }
    }
    return NSNotFound;
  }
  if (self.workspaceSelect == aComboBox) {
    for (int i = 0; i < self.workspaceList.count; i++) {
      ViewItem *workspace = [self.workspaceList objectAtIndex:i];
      if ([workspace.Name isEqualToString:aString]) {
        return i;
      }
    }
    return NSNotFound;
  }
  NSAssert(false, @"Invalid combo box");
  return NSNotFound;
}

- (void)controlTextDidEndEditing:(NSNotification *)aNotification {
  if (![[aNotification object] isKindOfClass:[NSTokenField class]]) {
    return;
  }
  [self applyTags];
}

- (void)controlTextDidChange:(NSNotification *)aNotification {
  // Don't trigger combobox autocomplete when inside tags field
  if (![[aNotification object] isKindOfClass:[NSComboBox class]]) {
    return;
  }

  NSCustomComboBox *comboBox = [aNotification object];
  NSString *filter = [comboBox stringValue];

  AutocompleteDataSource *dataSource = nil;
  if (comboBox == self.projectSelect) {
    dataSource = self.projectAutocompleteDataSource;
  }
  if (comboBox == self.descriptionCombobox) {
    dataSource = self.descriptionComboboxDataSource;
  }

  [dataSource setFilter:filter];
  [comboBox reloadingData:dataSource.textLength];

  if (!filter || ![filter length] || !dataSource.count) {
    if ([comboBox isExpanded] == YES) {
      [comboBox setExpanded:NO];
    }
    return;
  }

  if ([comboBox isExpanded] == NO) {
    [comboBox setExpanded:YES];
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
  self.clientSelect.stringValue = @"";
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
