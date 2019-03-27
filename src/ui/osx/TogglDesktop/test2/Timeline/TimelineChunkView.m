//
//  TimelineChunkView.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import "TimelineChunkView.h"
#import "TimelineEventView.h"
#import "toggl_api.h"
#import "TimeEntryViewItem.h"

@implementation TimelineChunkView

- (void)load:(TogglTimelineChunkView *)view
{
	self.activeDuration = 0;
	self.Started = view->Started;
	self.StartTimeString = [NSString stringWithUTF8String:view->StartTimeString];
	self.Events = [[NSMutableArray alloc] init];
	self.Entries = [[NSMutableArray alloc] init];
	self.EntryDescription = @"";
	self.EntryStart = 0;
	self.EntryEnd = 0;
	// NSLog(@"Timeline chunk loaded: %@", self);

	if (view->Entry != NULL)
	{
		int start = 100;
		int end = 0;
		uint64_t chunk_end = self.Started + 900;

		TogglTimeEntryView *entry = view->Entry;
		while (entry)
		{
			TimeEntryViewItem *model = [[TimeEntryViewItem alloc] init];
			[model load:entry];

			// Start time is in this chunk
			if (entry->Started >= self.Started
				&& entry->Started < chunk_end)
			{
				start = MIN(start, (int)entry->RoundedStart);
			}

			// End time is in this chunk
			if (entry->Ended <= chunk_end
				&& entry->Ended > self.Started)
			{
				end = MAX(end, (int)entry->RoundedEnd);
			}

			if (entry->Started <= self.Started)
			{
				start = 0;
			}

			if (entry->Ended >= chunk_end)
			{
				end = 100;
			}

			if ([self.EntryDescription length] > 0)
			{
				self.EntryDescription = [self.EntryDescription stringByAppendingString:[NSString stringWithFormat:@" | "]];
			}

			self.EntryDescription = [self.EntryDescription stringByAppendingString:[NSString stringWithFormat:@"%@ [%@ - %@]", model.Description, model.startTimeString, model.endTimeString]];

			entry = entry->Next;
		}
		self.EntryStart = start;
		self.EntryEnd = end;
	}

	TogglTimelineEventView *it = view->FirstEvent;
	while (it)
	{
		TimelineEventView *event = [[TimelineEventView alloc] init];
		[event load:it];
		[self.Events addObject:event];
		self.activeDuration += event.Duration;

		// Load app subevents
		TogglTimelineEventView *sub_it = it->Event;
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
	self.CalculatedHeight = MAX(20, ([self.Events count] * 20));
	//    self.CalculatedHeight = (60 + ([self.Events count] * 20));
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"Started: %@", self.StartTimeString];
}

@end
