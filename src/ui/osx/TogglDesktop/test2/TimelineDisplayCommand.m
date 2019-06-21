//
//  TimelineDisplayCommand.m
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "TimelineDisplayCommand.h"
#import "TimelineChunkView.h"
#import "TimeEntryViewItem.h"

@implementation TimelineDisplayCommand

- (instancetype)initWithOpen:(BOOL)open
						date:(NSString *)date
				  firstChunk:(TogglTimelineChunkView *)firstChunk
				  firstEntry:(TogglTimeEntryView *)firstEntry
					startDay:(NSTimeInterval)startDay
					  endDay:(NSTimeInterval)endDay
{
	self = [super init];
	if (self)
	{
		// Chunks
		NSMutableArray<TimelineChunkView *> *timelineChunks = [NSMutableArray<TimelineChunkView *> array];
		TogglTimelineChunkView *it = firstChunk;
		while (it)
		{
			TimelineChunkView *chunk = [[TimelineChunkView alloc] init];
			[chunk load:it];
			[timelineChunks addObject:chunk];
			it = it->Next;
		}

		// Get entries
		NSMutableArray<TimeEntryViewItem *> *timeEntries = [NSMutableArray<TimeEntryViewItem *> array];
		TogglTimeEntryView *it_entry = firstEntry;
		while (it_entry)
		{
			TimeEntryViewItem *item = [[TimeEntryViewItem alloc] init];
			[item load:it_entry];
			[timeEntries addObject:item];
			it_entry = it_entry->Next;
		}

		self.open = open;
		self.timelineDate = date;
		self.start = startDay;
		self.end = endDay;
		self.timeEntries = [[timeEntries reverseObjectEnumerator] allObjects];
		self.timelineChunks = [[timelineChunks reverseObjectEnumerator] allObjects];
	}
	return self;
}

@end
