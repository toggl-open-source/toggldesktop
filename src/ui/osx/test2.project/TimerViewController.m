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
@property TimeEntryViewItem *running_time_entry;
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
                                                   name:kUIStateTimerRunning
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateTimerStopped
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
  if ([notification.name isEqualToString:kUIStateTimerRunning]) {
    [self render:notification.object];
    
  } else if ([notification.name isEqualToString:kUIStateTimerStopped]) {
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
      if ([change.GUID isEqualToString:self.running_time_entry.GUID]) {
        [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerStopped object:nil];
      }
      
      return;
    }
    
    // Handle update
    TimeEntryViewItem *updated = [TimeEntryViewItem findByGUID:change.GUID];

    // Time entry we thought was running, has been stopped.
    if ((updated.duration_in_seconds >= 0) && [updated.GUID isEqualToString:self.running_time_entry.GUID]) {
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerStopped object:nil];
      return;
    }

    // Time entry we did not know was running, is running.
    if ((updated.duration_in_seconds < 0) && ![updated.GUID isEqualToString:self.running_time_entry.GUID]) {
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerRunning object:updated];
      return;
    }

    // Time entry is still running and needs to be updated.
    if ((updated.duration_in_seconds < 0) && [updated.GUID isEqualToString:self.running_time_entry.GUID]) {
      [self render:updated];
      return;
    }
  }
}

- (void) render:(TimeEntryViewItem *)view_item {
  self.running_time_entry = view_item;
  if (self.running_time_entry != nil) {
    [self.descriptionTextField setStringValue:self.running_time_entry.Description];
    [self.durationTextField setStringValue:self.running_time_entry.duration];
    if (self.running_time_entry.project != nil) {
      [self.projectTextField setStringValue:[self.running_time_entry.project uppercaseString]];
    } else {
      [self.projectTextField setStringValue:@""];
    }
  }
}

- (IBAction)stopButtonClicked:(id)sender
{
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandStop
                                                      object:nil];
}

- (void)timerFired:(NSTimer*)timer
{
  if (self.running_time_entry != nil) {
    char str[duration_str_len];
    kopsik_format_duration_in_seconds_hhmmss(self.running_time_entry.duration_in_seconds, str, duration_str_len);
    NSString *newValue = [NSString stringWithUTF8String:str];
    [self.durationTextField setStringValue:newValue];
  }
}

@end
