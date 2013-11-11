//
//  DateHeader.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 11/11/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "DateHeader.h"

@implementation DateHeader

- (NSString *)title
{
  return [NSString stringWithFormat:@"%@ (%@)", self.date, self.duration];
}

@end
