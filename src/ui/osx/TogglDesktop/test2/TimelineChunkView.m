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
	[self sortEvents];
}

- (void)sortEvents
{
	NSSortDescriptor *sortDescriptor;

	sortDescriptor = [[NSSortDescriptor alloc] initWithKey:@"Sorter"
												 ascending:YES];
	NSArray *sortDescriptors = [NSArray arrayWithObject:sortDescriptor];
	[sortDescriptors arrayByAddingObject:sortDescriptor];
	NSArray *sorted = [self.Events sortedArrayUsingDescriptors:sortDescriptors];
	self.Events = [sorted mutableCopy];
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
	[event updateSorter];
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
	[event updateSorter];
}

@end
