
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

@interface TimerEditViewController ()
@property AutocompleteDataSource *autocompleteDataSource;
@property NewTimeEntry *time_entry;
@property NSTimer *timerAutocompleteRendering;
@end

@implementation TimerEditViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
      self.autocompleteDataSource = [[AutocompleteDataSource alloc] init];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateUserLoggedIn
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventModelChange
                                                 object:nil];
      self.time_entry = [[NewTimeEntry alloc] init];
    }
    
    return self;
}

- (NSString *)comboBox:(NSComboBox *)comboBox completedString:(NSString *)partialString
{
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

- (void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    [self performSelectorOnMainThread:@selector(scheduleAutocompleteRendering) withObject:nil waitUntilDone:NO];
    return;
  }

  if ([notification.name isEqualToString:kUIEventModelChange]) {
    ModelChange *mc = notification.object;
    if ([mc.ModelType isEqualToString:@"tag"]) {
      return; // Tags dont affect autocomplete
    }
    [self performSelectorOnMainThread:@selector(scheduleAutocompleteRendering) withObject:nil waitUntilDone:NO];
    return;
  }
}

-(NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox{
  return [self.autocompleteDataSource count];
}

-(id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)row{
  return [self.autocompleteDataSource keyAtIndex:row];
}

- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)aString
{
  return [self.autocompleteDataSource indexOfKey:aString];
}

- (IBAction)startButtonClicked:(id)sender {
  self.time_entry.Description = self.descriptionComboBox.stringValue;
  [self.descriptionComboBox setStringValue:@""];
  [self.projectTextField setStringValue:@""];
  [self.projectTextField setHidden:YES];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandNew
                                                      object:self.time_entry];
  self.time_entry = [[NewTimeEntry alloc] init];

  // Reset autocomplete filter
  [self.autocompleteDataSource setFilter:@""];
  [self.descriptionComboBox reloadData];
}

- (IBAction)descriptionComboBoxChanged:(id)sender {

  NSString *key = [self.descriptionComboBox stringValue];
  AutocompleteItem *item = [self.autocompleteDataSource get:key];

  // User has entered free text
  if (item == nil) {
    [self.projectTextField setHidden:YES];
    [self.projectTextField setStringValue:@""];

    self.time_entry.Description = [self.descriptionComboBox stringValue];

    return;
  }

  // User has selected a autocomplete item.
  // It could be a time entry, a task or a project.
  [self.projectTextField setStringValue:[item.ProjectAndTaskLabel uppercaseString]];
  self.projectTextField.backgroundColor = [ConvertHexColor hexCodeToNSColor:item.ProjectColor];
  [self.projectTextField setHidden:NO];

  self.time_entry.ProjectID = item.ProjectID;
  self.time_entry.TaskID = item.TaskID;

  NSLog(@"New time entry desc: %@, task ID: %u, project ID: %u",
        self.time_entry.Description,
        self.time_entry.TaskID,
        self.time_entry.ProjectID);

  if (self.time_entry.ProjectID) {
    [self.descriptionComboBox setStringValue:@""];
  }
}

- (void)controlTextDidChange:(NSNotification *)aNotification
{
  NSComboBox *box = [aNotification object];
  NSString *filter = [box stringValue];
  [self.autocompleteDataSource setFilter:filter];
  [self.descriptionComboBox reloadData];
}

@end
