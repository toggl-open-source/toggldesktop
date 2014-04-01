//
//  ModelChange.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 21/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "ModelChange.h"

@implementation ModelChange

- (void)load:(KopsikModelChange *)change {
  self.GUID = [NSString stringWithUTF8String:change->GUID];
  self.ModelType = [NSString stringWithUTF8String:change->ModelType];
  self.ModelID = change->ModelID;
  self.ChangeType = [NSString stringWithUTF8String:change->ChangeType];
}

- (NSString *)description {
  return [NSString stringWithFormat:@"Model type: %@, change type: %@, GUID: %@, ID: %llu",
          self.ModelType, self.ChangeType, self.GUID, self.ModelID];
}

@end
