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
	self.Apps = [[NSMutableArray alloc] init];
	NSLog(@"Timeline chunk loaded: %@", self);

	TogglTimelineEventView *it = view->FirstEvent;
	while (it)
	{
		TimelineEventView *event = [[TimelineEventView alloc] init];
		[event load:it];
		[self needsAdd:event];

		TimelineEventView *appEvent = [[TimelineEventView alloc] init];
		[appEvent load:it];
		[self needsAppAdd:appEvent];

		// NSLog(@"Timeline event loaded: %@", event);
		it = it->Next;
	}

	[self.Events addObjectsFromArray:self.Apps];
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"Started: %@", self.StartTimeString];
}

- (void)needsAppAdd:(TimelineEventView *)event
{
	int counter = 0;

	for (TimelineEventView *ev in self.Apps)
	{
		if ([event.Filename isEqualToString:ev.Filename])
		{
			ev.Duration += event.Duration;
			return;
		}
		counter++;
	}


	event.Title = @"";
	[self.Apps addObject:event];
	NSLog(@"ADD: [%@] --- title ( %@ ), Filename:[%@] -> Duration: %lldd ", event.Filename, event.Title, event.Filename, event.Duration);
}

- (void)needsAdd:(TimelineEventView *)event
{
	int counter = 0;

	for (TimelineEventView *ev in self.Events)
	{
		if ([event.Title isEqualToString:ev.Title] && [event.Filename isEqualToString:ev.Filename])
		{
			ev.Duration += event.Duration;
			return;
		}
		counter++;
	}
	[self.Events addObject:event];
}

@end
