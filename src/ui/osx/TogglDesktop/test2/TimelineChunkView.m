//
//  TimelineChunkView.m
//  TogglDesktop
//
//  Created by Tanel Lebedev on 26/10/15.
//  Copyright © 2015 Toggl Desktop Developers. All rights reserved.
//

#import "TimelineChunkView.h"
#import "TimelineEventView.h"

@implementation TimelineChunkView

- (void)load:(TogglTimelineChunkView *)view
{
	self.activeDuration = 0;
	self.Started = view->Started;
	self.StartTimeString = [NSString stringWithUTF8String:view->StartTimeString];
	self.Events = [[NSMutableArray alloc] init];
	NSLog(@"Timeline chunk loaded: %@", self);

	TogglTimelineEventView *it = view->FirstEvent;
	while (it)
	{
		TimelineEventView *event = [[TimelineEventView alloc] init];
		[event load:it];
		[self.Events addObject:event];
		self.activeDuration += event.Duration;

		// Load app subevents
		TogglTimelineEventView *sub_it = it->event;
		while (sub_it)
		{
			TimelineEventView *sub_event = [[TimelineEventView alloc] init];
			[sub_event load:sub_it];
			[self.Events addObject:sub_event];
			sub_it = sub_it->Next;
		}

		NSLog(@"Timeline event loaded: %@", event);
		it = it->Next;
	}
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"Started: %@", self.StartTimeString];
}

@end
