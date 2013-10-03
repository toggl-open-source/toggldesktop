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

- (IBAction)durationTextFieldChanged:(id)sender {
}

- (IBAction)projectSelectChanged:(id)sender {
}

- (IBAction)startTimeChanged:(id)sender {
}

- (IBAction)endTimeChanged:(id)sender {
}

- (IBAction)dateChanged:(id)sender {
}

- (IBAction)tagsChanged:(id)sender {
}

- (IBAction)billableCheckBoxClicked:(id)sender {
}

- (IBAction)deleteButtonClicked:(id)sender {
}

- (IBAction)descriptionTextFieldChanged:(id)sender {
}

@end
