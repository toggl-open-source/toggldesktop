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
                                                   name:kUIEventUpdate
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
    self.te = nil;

  } else if ([notification.name isEqualToString:kUIEventUpdate]) {
    NSString *GUID = notification.object;
    if ([GUID isEqualToString:self.te.GUID]) {
      TimeEntryViewItem *te = [TimeEntryViewItem findByGUID:GUID];
      if (te.duration_in_seconds >= 0 ) {
        // Looks like the time entry we thought is running was actually
        // stopped meanwhile. Pass forward the information.
        [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimerStopped
                                                            object:nil];
      } else {
        // Phew, the updated time entry is still running. Render it.
        [self render:te];
      }
    }
  }
}

- (void) render:(TimeEntryViewItem *)view_item {
  self.te = view_item;
  [self.descriptionTextField setStringValue:self.te.Description];
  [self.durationTextField setStringValue:self.te.duration];
  if (self.te.project != nil) {
    [self.projectTextField setStringValue:self.te.project];
  } else {
    [self.projectTextField setStringValue:@""];
  }
}

void finishPushAfterStop(kopsik_api_result result, char *err, unsigned int errlen) {
  if (KOPSIK_API_SUCCESS != result) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    free(err);
  }
}

- (IBAction)stopButtonClicked:(id)sender
{
  char err[KOPSIK_ERR_LEN];
  KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
  if (KOPSIK_API_SUCCESS != kopsik_stop(ctx, err, KOPSIK_ERR_LEN, item)) {
    kopsik_time_entry_view_item_clear(item);
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
  
  TimeEntryViewItem *te = [[TimeEntryViewItem alloc] init];
  [te load:item];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimerStopped object:te];

  kopsik_time_entry_view_item_clear(item);

  kopsik_push_async(ctx, finishPushAfterStop);
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
