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
#import "Bugsnag.h"

@interface TimeEntryEditViewController ()
@property NSString *GUID;
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
    }
    
    return self;
}

- (IBAction)backButtonClicked:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimeEntryDeselected object:nil];
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIEventTimeEntrySelected]) {
    NSString *guid = notification.object;
    NSAssert(guid != nil, @"GUID is nil");
    NSLog(@"guid = %@", guid);
    int was_found = 0;
    KopsikTimeEntryViewItem *view_item = kopsik_time_entry_view_item_init();
    char err[KOPSIK_ERR_LEN];
    if (KOPSIK_API_SUCCESS != kopsik_time_entry_view_item_by_guid(ctx,
                                                                  err,
                                                                  KOPSIK_ERR_LEN,
                                                                  [guid UTF8String],
                                                                  view_item,
                                                                  &was_found)) {
      NSLog(@"Error fetching time entry by GUID %@: %s", guid, err);
      kopsik_time_entry_view_item_clear(view_item);
      [Bugsnag notify:[NSException
                       exceptionWithName:@"Error fetching TE by GUID"
                       reason:[NSString stringWithUTF8String:err]
                       userInfo:nil]];
      return;
    }
    
    if (!was_found) {
      kopsik_time_entry_view_item_clear(view_item);
      return;
    }
    
    TimeEntryViewItem *item = [[TimeEntryViewItem alloc] init];
    [item load:view_item];
    kopsik_time_entry_view_item_clear(view_item);
    
    self.GUID = guid;
    NSAssert(self.GUID != nil, @"GUID is nil");
    
    [self.descriptionTextField setStringValue:item.description];
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
  }
}

void finishPushAfterUpdate(kopsik_api_result result, char *err, unsigned int errlen) {
  NSLog(@"finishPushAfterUpdate");
  if (KOPSIK_API_SUCCESS != result) {
    NSLog(@"Error pushing data: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error pushing data"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    free(err);
  }
}

- (IBAction)durationTextFieldChanged:(id)sender {
  NSLog(@"durationTextFieldChanged");
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  const char *value = [[self.durationTextField stringValue] UTF8String];
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_duration(ctx,
                                                           err,
                                                           KOPSIK_ERR_LEN,
                                                           [self.GUID UTF8String],
                                                           value)) {
    NSLog(@"Error updating time entry: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error updating time entry"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    return;
  }
  kopsik_push_async(ctx, finishPushAfterUpdate);
}

- (IBAction)projectSelectChanged:(id)sender {
  NSLog(@"projectSelectChanged");
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  const char *value = 0;
  id selectedValue = [self.projectSelect objectValueOfSelectedItem];
  if (selectedValue != nil) {
    NSString *stringValue = (NSString *)selectedValue;
    value = [stringValue UTF8String];
  }
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_project(ctx,
                                                          err,
                                                          KOPSIK_ERR_LEN,
                                                          [self.GUID UTF8String],
                                                          value)) {
    NSLog(@"Error updating time entry: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error updating time entry"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    return;
  }
  kopsik_push_async(ctx, finishPushAfterUpdate);
}

- (IBAction)startTimeChanged:(id)sender {
  NSLog(@"startTimeChanged");
  NSAssert(self.GUID != nil, @"GUID is nil");
  [self applyStartTime];
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
    NSLog(@"Error updating time entry: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error updating time entry"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    return;
  }
}

- (IBAction)endTimeChanged:(id)sender {
  NSLog(@"endTimeChanged");
  NSAssert(self.GUID != nil, @"GUID is nil");
  [self applyEndTime];
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
    NSLog(@"Error updating time entry: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error updating time entry"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    return;
  }
}

- (IBAction)dateChanged:(id)sender {
  NSLog(@"dateChanged");
  NSAssert(self.GUID != nil, @"GUID is nil");
  [self applyStartTime];
  [self applyEndTime];
  kopsik_push_async(ctx, finishPushAfterUpdate);
}

- (IBAction)tagsChanged:(id)sender {
  NSLog(@"tagsChanged");
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  NSArray *tag_names = [self.tags objectValue];
  const char *value = [[tag_names componentsJoinedByString:@"|"] UTF8String];
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_tags(ctx,
                                                       err,
                                                       KOPSIK_ERR_LEN,
                                                       [self.GUID UTF8String],
                                                       value)) {
    NSLog(@"Error updating time entry: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error updating time entry"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    return;
  }
  kopsik_push_async(ctx, finishPushAfterUpdate);
}

- (IBAction)billableCheckBoxClicked:(id)sender {
  NSLog(@"billableCheckBoxClicked");
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
    NSLog(@"Error updating time entry: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error updating time entry"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    return;
  }
  kopsik_push_async(ctx, finishPushAfterUpdate);
}

- (IBAction)descriptionTextFieldChanged:(id)sender {
  NSLog(@"descriptionTextFieldChanged");
  NSAssert(self.GUID != nil, @"GUID is nil");
  char err[KOPSIK_ERR_LEN];
  const char *value = [[self.descriptionTextField stringValue] UTF8String];
  if (KOPSIK_API_SUCCESS != kopsik_set_time_entry_description(ctx,
                                                              err,
                                                              KOPSIK_ERR_LEN,
                                                              [self.GUID UTF8String],
                                                              value)) {
    NSLog(@"Error updating time entry: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error updating time entry"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    return;
  }
  kopsik_push_async(ctx, finishPushAfterUpdate);
}

void finishPushAfterDelete(kopsik_api_result result, char *err, unsigned int errlen) {
  NSLog(@"finishPushAfterDelete");
  if (KOPSIK_API_SUCCESS != result) {
    NSLog(@"Error pushing data: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error pushing data"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
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
  
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_delete_time_entry(ctx,
                                                     err,
                                                     KOPSIK_ERR_LEN,
                                                     [self.GUID UTF8String])) {
    NSLog(@"Error deleting time entry: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error deleting time entry"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    return;
  }
  kopsik_push_async(ctx, finishPushAfterDelete);
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimeEntryDeselected object:nil];
}

@end
