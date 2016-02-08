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
	self.Started = view->Started;
	self.StartTimeString = [NSString stringWithUTF8String:view->StartTimeString];
	self.Events = [[NSMutableArray alloc] init];
	NSLog(@"Timeline chunk loaded: %@", self);

	TogglTimelineEventView *it = view->FirstEvent;
	while (it)
	{
		TimelineEventView *event = [[TimelineEventView alloc] init];
		[event load:it];
		if (![self contains:event])
		{
			[self.Events addObject:event];
		}
		NSLog(@"Timeline event loaded: %@", event);
		it = it->Next;
	}
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"Started: %@", self.StartTimeString];
}

- (BOOL)contains:(TimelineEventView *)event
{
	int counter = 0;

	for (TimelineEventView *ev in self.Events)
	{
		if ([event.Title isEqualToString:ev.Title] && [event.Filename isEqualToString:ev.Filename])
		{
			ev.Duration += event.Duration;
			[self.Events replaceObjectAtIndex:counter withObject:ev];
			return YES;
		}
		counter++;
	}
	return NO;
}

@end
