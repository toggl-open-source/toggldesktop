//
//  TimerViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TimerViewController.h"
#import "UIEvents.h"
#import "kopsik_api.h"
#import "TimeEntryViewItem.h"
#import "Context.h"
#import "ModelChange.h"
#import "ErrorHandler.h"

@interface TimerViewController ()
@property TimeEntryViewItem *te;
@property NSTimer *timer;
@end

#define duration_str_len 20

@implementation TimerViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventTimerRunning
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventTimerStopped
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventModelChange
                                                 object:nil];
      self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                               target:self
                                             selector:@selector(timerFired:)
                                             userInfo:nil
                                              repeats:YES];
    }
  
    return self;
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIEventTimerRunning]) {
    [self render:notification.object];
    
  } else if ([notification.name isEqualToString:kUIEventTimerStopped]) {
    [self render:nil];

  } else if ([notification.name isEqualToString:kUIEventModelChange]) {
    
    ModelChange *change = notification.object;
    
    // We only care about time entry changes
    if (! [change.ModelType isEqualToString:@"time_entry"]) {
      return;
    }
    
    // Handle delete
    if ([change.ChangeType isEqualToString:@"delete"]) {

      // Time entry we thought was running, has been deleted.
      if ([change.GUID isEqualToString:self.te.GUID]) {
        [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimerStopped object:nil];
      }
      
      return;
    }
    
    // Handle update
    TimeEntryViewItem *updated = [TimeEntryViewItem findByGUID:change.GUID];

    // Time entry we thought was running, has been stopped.
    if ((updated.duration_in_seconds >= 0) && [updated.GUID isEqualToString:self.te.GUID]) {
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimerStopped object:nil];
      return;
    }

    // Time entry we did not know was running, is running.
    if ((updated.duration_in_seconds < 0) && ![updated.GUID isEqualToString:self.te.GUID]) {
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimerRunning object:updated];
      return;
    }

    // Time entry is still running and needs to be updated.
    if ((updated.duration_in_seconds < 0) && [updated.GUID isEqualToString:self.te.GUID]) {
      [self render:updated];
      return;
    }
  }
}

- (void) render:(TimeEntryViewItem *)view_item {
  self.te = view_item;
  if (self.te != nil) {
    [self.descriptionTextField setStringValue:self.te.Description];
    [self.durationTextField setStringValue:self.te.duration];
    if (self.te.project != nil) {
      [self.projectTextField setStringValue:self.te.project];
    } else {
      [self.projectTextField setStringValue:@""];
    }
  }
}

- (IBAction)stopButtonClicked:(id)sender
{
  char err[KOPSIK_ERR_LEN];
  KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
  kopsik_api_result res = kopsik_stop(ctx, err, KOPSIK_ERR_LEN, item);
  if (KOPSIK_API_SUCCESS != res) {
    kopsik_time_entry_view_item_clear(item);
    handle_error(res, err);
    return;
  }
  
  TimeEntryViewItem *te = [[TimeEntryViewItem alloc] init];
  [te load:item];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimerStopped object:te];

  kopsik_time_entry_view_item_clear(item);

  kopsik_push_async(ctx, handle_error);
}

- (void)timerFired:(NSTimer*)timer
{
  if (self.te != nil) {
    char str[duration_str_len];
    kopsik_format_duration_in_seconds(self.te.duration_in_seconds, str, duration_str_len);
    NSString *newValue = [NSString stringWithUTF8String:str];
    [self.durationTextField setStringValue:newValue];
  }
}

@end
