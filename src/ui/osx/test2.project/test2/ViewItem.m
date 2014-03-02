//
//  ViewItem.m
//  kopsik_ui_osx
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import "ViewItem.h"

@implementation ViewItem

- (void)load:(KopsikViewItem *)data {
  self.ID = data->ID;
  self.Name = [NSString stringWithUTF8String:data->Name];
  self.GUID = nil;
  if (data->GUID) {
    self.GUID = [NSString stringWithUTF8String:data->GUID];
  }
}

@end
