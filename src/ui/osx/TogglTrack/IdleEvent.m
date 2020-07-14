//
//  IdleEvent.m
//  Toggl Track on the Mac
//
//  Created by Tanel Lebedev on 04/11/2013.
//  Copyright (c) 2013 TogglTrack developers. All rights reserved.
//

#import "IdleEvent.h"

@implementation IdleEvent

- (NSString *)description
{
	return [NSString stringWithFormat:@"since: %@, duration: %@, started: %ld",
			self.since, self.duration, (long)self.started];
}

-(BOOL)hasProject {
	if (self.projectName.length && self.projectColor != nil) {
		return YES;
	}
	return NO;
}

@end
