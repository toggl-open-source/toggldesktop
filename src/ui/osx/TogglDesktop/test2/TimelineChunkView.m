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
		[self.Events addObject:event];

		NSLog(@"Timeline event loaded: %@", event);
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

@end
