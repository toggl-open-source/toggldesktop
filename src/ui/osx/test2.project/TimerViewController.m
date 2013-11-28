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
#import "ConvertHexColor.h"

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
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUICommandEditRunningTimeEntry
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
    [self performSelectorOnMainThread:@selector(render:)
                           withObject:notification.object
                        waitUntilDone:NO];
    return;
  }
  
  if ([notification.name isEqualToString:kUIStateTimerStopped]) {
    [self performSelectorOnMainThread:@selector(render:)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }
  
  if ([notification.name isEqualToString:kUICommandEditRunningTimeEntry]) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntrySelected
                                                        object:self.running_time_entry.GUID];
    return;
  }

  if ([notification.name isEqualToString:kUIEventModelChange]) {
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
      [self performSelectorOnMainThread:@selector(render:)
                             withObject:updated
                          waitUntilDone:NO];
      return;
    }
  }
}

- (void) render:(TimeEntryViewItem *)view_item {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  self.running_time_entry = view_item;
  if (self.running_time_entry != nil) {
    [self.descriptionTextField setStringValue:self.running_time_entry.Description];
    [self.durationTextField setStringValue:self.running_time_entry.duration];
    self.projectTextField.backgroundColor = [ConvertHexColor hexCodeToNSColor:view_item.color];
    if (self.running_time_entry.ProjectAndTaskLabel != nil) {
      [self.projectTextField setStringValue:[self.running_time_entry.ProjectAndTaskLabel uppercaseString]];
      [self.projectTextField setHidden:NO];
    } else {
      [self.projectTextField setHidden:YES];
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
