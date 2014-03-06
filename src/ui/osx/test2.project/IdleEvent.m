//
//  IdleEvent.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 04/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "IdleEvent.h"

@implementation IdleEvent

- (NSString *)description {
  return [NSString stringWithFormat:@"started: %@, finished: %@, seconds: %d",
          self.started, self.finished, self.seconds];
}

@end
