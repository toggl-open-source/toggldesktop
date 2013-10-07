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
#import "Bugsnag.h"

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
    self.te = notification.object;
    [self.descriptionTextField setStringValue:self.te.description];
    [self.durationTextField setStringValue:self.te.duration];
    if (self.te.project != nil) {
      [self.projectTextField setStringValue:self.te.project];
    } else {
      [self.projectTextField setStringValue:@""];
    }
    
  } else if ([notification.name isEqualToString:kUIEventTimerStopped]) {
    self.te = nil;
  }
}

void finishPushAfterStop(kopsik_api_result result, char *err, unsigned int errlen) {
  NSLog(@"finishPushAfterStop");
  if (KOPSIK_API_SUCCESS != result) {
    NSLog(@"Error pushing data: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error pushing data"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    free(err);
  }
}

- (IBAction)stopButtonClicked:(id)sender
{
  char err[KOPSIK_ERR_LEN];
  KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
  if (KOPSIK_API_SUCCESS != kopsik_stop(ctx, err, KOPSIK_ERR_LEN, item)) {
    NSLog(@"Error stopping time entry: %s", err);
    kopsik_time_entry_view_item_clear(item);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error stopping time entry"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
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
