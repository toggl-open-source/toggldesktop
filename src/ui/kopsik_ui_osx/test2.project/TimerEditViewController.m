//
//  TimerEditViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TimerEditViewController.h"
#import "kopsik_api.h"
#import "Context.h"
#import "TimeEntryViewItem.h"
#import "UIEvents.h"

@interface TimerEditViewController ()

@end

@implementation TimerEditViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

void finishPushAfterStart(kopsik_api_result result, char *err, unsigned int errlen) {
  if (KOPSIK_API_SUCCESS != result) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    free(err);
  }
}

- (IBAction)startButtonClicked:(id)sender {
  NSString *description = [self.descriptionTextField stringValue];
  if ([description length] == 0) {
    [self.descriptionTextField becomeFirstResponder];
    return;
  }
  char err[KOPSIK_ERR_LEN];
  KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
  if (KOPSIK_API_SUCCESS != kopsik_start(ctx, err, KOPSIK_ERR_LEN, [description UTF8String], item)) {
    kopsik_time_entry_view_item_clear(item);
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }

  TimeEntryViewItem *te = [[TimeEntryViewItem alloc] init];
  [te load:item];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimerRunning object:te];

  kopsik_push_async(ctx, finishPushAfterStart);
}

@end
