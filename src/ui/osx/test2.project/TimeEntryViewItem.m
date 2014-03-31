//
//  TimeEntryViewItem.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 25/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
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
  self.WorkspaceID = data->WID;
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
    self.updatedAt = [NSDate dateWithTimeIntervalSince1970:data->UpdatedAt];
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
  if (nil == guid || 0 == guid.length) {
    return nil;
  }
  _Bool was_found = false;
  KopsikTimeEntryViewItem *view_item = kopsik_time_entry_view_item_init();
  if (KOPSIK_API_SUCCESS != kopsik_time_entry_view_item_by_guid(ctx,
                                                                [guid UTF8String],
                                                                view_item,
                                                                &was_found)) {

    kopsik_time_entry_view_item_clear(view_item);
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
