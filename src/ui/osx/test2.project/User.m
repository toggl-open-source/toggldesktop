//
//  User.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 02/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "User.h"
#import "kopsik_api.h"

@implementation User

- (void)load:(KopsikUser *)data {
  self.ID = data->ID;
  self.fullname = [NSString stringWithUTF8String:data->Fullname];
}

- (NSString *)description {
  return [NSString stringWithFormat:@"ID: %ld, fullname: %@",
          self.ID, self.fullname];
}

@end
