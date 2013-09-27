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

@interface TimerViewController ()
@property NSTimer *timer;
@end

#define duration_str_len 20

@implementation TimerViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
      [[NSNotificationCenter defaultCenter]
       addObserver:self
       selector:@selector(eventHandler:)
       name:kUIEventTimerRunning
       object:nil];
      [[NSNotificationCenter defaultCenter]
       addObserver:self
       selector:@selector(eventHandler:)
       name:kUIEventTimerStopped
       object:nil];
    }
    
    return self;
}

- (void)stopTimer {
  if (self.timer != nil && [self.timer isValid] == YES) {
    [self.timer invalidate];
  }
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIEventTimerRunning]) {
    TimeEntryViewItem *te = notification.object;
    [self.descriptionTextField setStringValue:te.description];
    [self.durationTextField setStringValue:te.duration];
    [self.projectTextField setStringValue:te.project];
    if ((self.timer == nil) || ([self.timer isValid] == NO)) {
      self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                    target:self
                                                  selector:@selector(timerFired:)
                                                  userInfo:te
                                                   repeats:YES];
    }
  } else if ([notification.name isEqualToString:kUIEventTimerStopped]) {
    [self stopTimer];
  } else if ([notification.name isEqualToString:kUIEventTimerStopped]) {
    [self stopTimer];
  }
}

- (void)timerFired:(NSTimer*)timer
{
  char str[duration_str_len];
  TimeEntryViewItem *te = timer.userInfo;
  kopsik_format_duration_in_seconds(te.duration_in_seconds, str, duration_str_len);
  NSString *newValue = [NSString stringWithUTF8String:str];
  [self.durationTextField setStringValue:newValue];
}

@end
