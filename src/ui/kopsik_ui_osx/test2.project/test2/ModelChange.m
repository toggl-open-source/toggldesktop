//
//  ModelChange.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 21/10/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "ModelChange.h"

@implementation ModelChange

- (void)load:(KopsikModelChange *)change {
  self.GUID = [NSString stringWithUTF8String:change->GUID];
  self.ModelType = [NSString stringWithUTF8String:change->ModelType];
  self.ModelID = change->ModelID;
  self.ChangeType = [NSString stringWithUTF8String:change->ChangeType];
}

@end
