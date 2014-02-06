//
//  TimeEntryViewItem.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 25/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TimeEntryViewItem.h"
#import "kopsik_api.h"
#import "Context.h"
#import "UIEvents.h"  

@implementation TimeEntryViewItem

- (void)load:(KopsikTimeEntryViewItem *)data {
  self.GUID = [NSString stringWithUTF8String:data->GUID];
  self.duration_in_seconds = data->DurationInSeconds;
  self.Description = [NSString stringWithUTF8String:data->Description];
  if (data->ProjectAndTaskLabel) {
    self.ProjectAndTaskLabel = [NSString stringWithUTF8String:data->ProjectAndTaskLabel];
  } else {
    self.ProjectAndTaskLabel = nil;
  }
  self.ProjectID = data->PID;
  self.TaskID = data->TID;
  if (data->Color) {
    self.ProjectColor = [NSString stringWithUTF8String:data->Color];
  } else {
    self.ProjectColor = nil;
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
  if (data->UpdatedAt) {
    self.updatedAt = [NSString stringWithUTF8String:data->UpdatedAt];
  } else {
    self.updatedAt = nil;
  }
  self.formattedDate = [NSString stringWithUTF8String:data->DateHeader];
  if (data->DateDuration) {
    self.dateDuration = [NSString stringWithUTF8String:data->DateDuration];
  } else {
    self.dateDuration = nil;
  }

  self.durOnly = NO;
  if (data->DurOnly) {
    self.durOnly = YES;
  }
}

+ (TimeEntryViewItem *)findByGUID:(NSString *)guid {
  int was_found = 0;
  KopsikTimeEntryViewItem *view_item = kopsik_time_entry_view_item_init();
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_time_entry_view_item_by_guid(ctx,
                                                                err,
                                                                KOPSIK_ERR_LEN,
                                                                [guid UTF8String],
                                                                view_item,
                                                                &was_found)) {
    kopsik_time_entry_view_item_clear(view_item);
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
    return nil;
  }
  
  if (!was_found) {
    kopsik_time_entry_view_item_clear(view_item);
    return nil;
  }
  
  TimeEntryViewItem *item = [[TimeEntryViewItem alloc] init];
  [item load:view_item];
  kopsik_time_entry_view_item_clear(view_item);

  return item;
}

- (NSString *)description {
  return [NSString stringWithFormat:@"GUID: %@, description: %@, started: %@, ended: %@, project: %@, seconds: %d, duration: %@, color: %@, billable: %i, tags: %@",
          self.GUID, self.Description, self.started, self.ended,
          self.ProjectAndTaskLabel, self.duration_in_seconds, self.duration,
          self.ProjectColor, self.billable, self.tags];
}

@end
