//
//  TimelineDisplayCommand.m
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "TimelineDisplayCommand.h"
#import "TimeEntryViewItem.h"

@implementation TimelineEvent
- (instancetype)initWithTitle:(NSString *)title fileName:(NSString *)filename duration:(NSTimeInterval)duration isHeader:(BOOL)isHeader
{
	self = [super init];
	if (self)
	{
		self.title = title;
		self.fileName = filename;
		self.duration = duration;
		self.isHeader = isHeader;
	}
	return self;
}

@end

@implementation TimelineActivity

- (instancetype)initWithStarted:(NSTimeInterval)started startedTimeString:(NSString *)startedTimeString events:(NSArray<TimelineEvent *> *)events
{
	self = [super init];
	if (self)
	{
		self.started = started;
		self.startedTimeString = startedTimeString;
		self.events = events;
	}
	return self;
}

@end

@implementation TimelineDisplayCommand

- (instancetype)initWithOpen:(BOOL)open
						date:(NSString *)date
			   firstActivity:(TogglTimelineChunkView *)firstActivity
				  firstEntry:(TogglTimeEntryView *)firstEntry
					startDay:(NSTimeInterval)startDay
					  endDay:(NSTimeInterval)endDay
{
	self = [super init];
	if (self)
	{
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
	}
	return self;
}

@end
