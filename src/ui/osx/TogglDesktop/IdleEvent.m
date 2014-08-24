//
//  IdleEvent.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 04/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "IdleEvent.h"

@implementation IdleEvent

- (NSString *)description
{
	return [NSString stringWithFormat:@"since: %@, duration: %@, started: %lld",
			self.since, self.duration, self.started];
}

@end
