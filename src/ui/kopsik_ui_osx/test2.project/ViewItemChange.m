//
//  ViewItemChange.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 10/10/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "ViewItemChange.h"

@implementation ViewItemChange
- (void)load:(KopsikViewItemChange *)data {
  self.GUID = [NSString stringWithUTF8String:data->GUID];
  self.model_id = data->model_id;
  self.model_type = data->model_type;
}

@end

