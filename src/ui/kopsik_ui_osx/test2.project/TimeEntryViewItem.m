//
//  TimeEntryViewItem.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 25/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TimeEntryViewItem.h"
#import "kopsik_api.h"

@implementation TimeEntryViewItem

- (void)load:(KopsikTimeEntryViewItem *)data {
  self.GUID = [NSString stringWithUTF8String:data->GUID];
  self.duration_in_seconds = data->DurationInSeconds;
  self.description = [NSString stringWithUTF8String:data->Description];
  if (data->Project) {
    self.project = [NSString stringWithUTF8String:data->Project];
  } else {
    self.project = nil;
  }
  if (data->Color) {
    self.color = [NSString stringWithUTF8String:data->Color];
  } else {
    self.color = nil;
  }
  self.duration = [NSString stringWithUTF8String:data->Duration];
  if (data->Tags) {
    NSString *tagList = [NSString stringWithUTF8String:data->Tags];
    self.tags = [tagList componentsSeparatedByString:@"|"];
  } else {
    self.tags = nil;
  }
  if (data->Billable) {
    self.Billable = YES;
  } else {
    self.Billable = NO;
  }
  self.started = [NSDate dateWithTimeIntervalSince1970:data->Started];
  self.ended = [NSDate dateWithTimeIntervalSince1970:data->Ended];
}

@end
