//
//  NSBoxClickable.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 13/11/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "NSBoxClickable.h"

@implementation NSBoxClickable

- (void)mouseUp:(NSEvent*) event {
  NSLog(@"Mouse up!");
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
  return YES;
}

@end
