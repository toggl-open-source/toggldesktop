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

- (void)load:(TogglTimeEntryViewItem *)data {
  self.description = [NSString stringWithUTF8String:data->Description];
  if (data->Project) {
    self.project = [NSString stringWithUTF8String:data->Project];
  }
  if (data->Color) {
    self.color = [NSString stringWithUTF8String:data->Color];
  }
  self.duration = [NSString stringWithUTF8String:data->Duration];
}

@end
