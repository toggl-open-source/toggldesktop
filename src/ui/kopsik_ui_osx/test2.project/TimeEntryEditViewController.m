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

@interface TimeEntryEditViewController ()
@property NSString *GUID;
@property NSMutableArray *projectNames;
@end

@implementation TimeEntryEditViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventTimeEntrySelected
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventUserLoggedIn
                                                 object:nil];
      self.projectNames = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (IBAction)backButtonClicked:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimeEntryDeselected object:nil];
}

- (void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIEventTimeEntrySelected]) {
    NSString *guid = notification.object;
    NSAssert(guid != nil, @"GUID is nil");
    TimeEntryViewItem *item = [TimeEntryViewItem findByGUID:guid];
    NSAssert(item != nil, @"View item not found by GUID!");

    self.GUID = guid;
    NSAssert(self.GUID != nil, @"GUID is nil");
    
    [self.descriptionTextField setStringValue:item.Description];
    if (item.project != nil) {
      [self.projectSelect setStringValue:item.project];
    } else {
      [self.projectSelect setStringValue:@""];
    }
    [self.durationTextField setStringValue:item.duration];
    [self.startTime setDateValue:item.started];
    [self.startDate setDateValue:item.started];
    [self.endTime setDateValue:item.ended];
    if (YES == item.billable) {
      [self.billableCheckbox setState:NSOnState];
    } else {
      [self.billableCheckbox setState:NSOffState];
    }
    
    if ([item.tags count] == 0) {
      [self.tags setObjectValue:nil];
    } else {
      [self.tags setObjectValue:item.tags];
    }

  } else if ([notification.name isEqualToString:kUIEventUserLoggedIn]) {
    [self.projectNames removeAllObjects];
    KopsikProjectSelectItemList *list = kopsik_project_select_item_list_init();
    char err[KOPSIK_ERR_LEN];
    if (KOPSIK_API_SUCCESS != kopsik_project_select_items(ctx, err, KOPSIK_ERR_LEN, list)) {
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                          object:[NSString stringWithUTF8String:err]];
      kopsik_project_select_item_list_clear(list);
      return;
    }
    for (int i = 0; i < list->Length; i++) {
      KopsikProjectSelectItem *item = list->ViewItems[i];
      NSString *project_name = [NSString stringWithUTF8String:item->Name];
      [self.projectNames addObject:project_name];
    }
    kopsik_project_select_item_list_clear(list);
    [self.projectSelect reloadData];
  }
}

void finishPushAfterUpdate(kopsik_api_result result, char *err, unsigned int errlen) {
  if (KOPSIK_API_SUCCESS != result) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    free(err);
  }
}

- (IBAction)durationTextFieldChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  const char *value = [[self.durationTextField stringValue] UTF8String];
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_duration(ctx,
                                                           err,
                                                           KOPSIK_ERR_LEN,
                                                           [self.GUID UTF8String],
                                                           value)) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUpdate
                                                      object:self.GUID];
  kopsik_push_async(ctx, finishPushAfterUpdate);
}

- (IBAction)projectSelectChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  const char *value = 0;
  id selectedValue = [self.projectSelect objectValueOfSelectedItem];
  return;
  if (selectedValue != nil) {
    NSString *stringValue = (NSString *)selectedValue;
    value = [stringValue UTF8String];
  }
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_project(ctx,
                                                          err,
                                                          KOPSIK_ERR_LEN,
                                                          [self.GUID UTF8String],
                                                          value)) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUpdate
                                                      object:self.GUID];
  kopsik_push_async(ctx, finishPushAfterUpdate);
}

- (IBAction)startTimeChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  [self applyStartTime];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUpdate
                                                      object:self.GUID];
  kopsik_push_async(ctx, finishPushAfterUpdate);
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
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
}

- (IBAction)endTimeChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  [self applyEndTime];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUpdate
                                                      object:self.GUID];
  kopsik_push_async(ctx, finishPushAfterUpdate);
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
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
}

- (IBAction)dateChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  [self applyStartTime];
  [self applyEndTime];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUpdate
                                                      object:self.GUID];
  kopsik_push_async(ctx, finishPushAfterUpdate);
}

- (IBAction)tagsChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  NSArray *tag_names = [self.tags objectValue];
  const char *value = [[tag_names componentsJoinedByString:@"|"] UTF8String];
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_tags(ctx,
                                                       err,
                                                       KOPSIK_ERR_LEN,
                                                       [self.GUID UTF8String],
                                                       value)) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUpdate
                                                      object:self.GUID];
  kopsik_push_async(ctx, finishPushAfterUpdate);
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
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUpdate
                                                      object:self.GUID];
  kopsik_push_async(ctx, finishPushAfterUpdate);
}

- (IBAction)descriptionTextFieldChanged:(id)sender {
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  const char *value = [[self.descriptionTextField stringValue] UTF8String];
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_description(ctx,
                                                              err,
                                                              KOPSIK_ERR_LEN,
                                                              [self.GUID UTF8String],
                                                              value)) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUpdate
                                                      object:self.GUID];
  kopsik_push_async(ctx, finishPushAfterUpdate);
}

void finishPushAfterDelete(kopsik_api_result result, char *err, unsigned int errlen) {
  if (KOPSIK_API_SUCCESS != result) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    free(err);
  }
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

  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventDelete
                                                      object:item];

  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_delete_time_entry(ctx,
                                                     err,
                                                     KOPSIK_ERR_LEN,
                                                     [self.GUID UTF8String])) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
  kopsik_push_async(ctx, finishPushAfterDelete);
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimeEntryDeselected object:nil];
}

-(NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox{
  return [self.projectNames count];
}

-(id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)row{
  return [self.projectNames objectAtIndex:row];
}

@end
