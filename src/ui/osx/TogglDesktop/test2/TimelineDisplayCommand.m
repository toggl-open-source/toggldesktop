//
//  TimelineDisplayCommand.m
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "TimelineDisplayCommand.h"
#import "TimeEntryViewItem.h"

@implementation TimelineDisplayEvent
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

- (instancetype)initWithView:(TogglTimelineEventView *)view
{
	self = [super init];
	if (self)
	{
		if (view->Title)
		{
			self.title = [NSString stringWithUTF8String:view->Title];
		}
		else
		{
			self.title = @"";
		}
		if (view->Filename)
		{
			self.fileName = [NSString stringWithUTF8String:view->Filename];
		}
		else
		{
			self.fileName = @"";
		}
		self.duration = view->Duration;
		self.isHeader = view->Header;
	}
	return self;
}

@end

@implementation TimelineDisplayActivity

- (instancetype)initWithStarted:(NSTimeInterval)started startedTimeString:(NSString *)startedTimeString events:(NSArray<TimelineDisplayEvent *> *)events
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

- (instancetype)initWithView:(TogglTimelineChunkView *)view
{
	self = [super init];
	if (self)
	{
		self.started = view->Started;
		if (view->StartTimeString)
		{
			self.startedTimeString = [NSString stringWithUTF8String:view->StartTimeString];
		}
		else
		{
			self.startedTimeString = @"";
		}

		// Events
		NSMutableArray<TimelineDisplayEvent *> *events = [@[] mutableCopy];
		TogglTimelineEventView *_event = view->FirstEvent;
		while (_event)
		{
			TimelineDisplayEvent *event = [[TimelineDisplayEvent alloc] initWithView:_event];
			[events addObject:event];
			_event = _event->Next;
		}
		self.events = [events copy];
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

		// Get activity
		NSMutableArray<TimelineDisplayActivity *> *activities = [@[] mutableCopy];
		TogglTimelineChunkView *_activity = firstActivity;
		while (_activity)
		{
			TimelineDisplayActivity *activity = [[TimelineDisplayActivity alloc] initWithView:_activity];
			[activities addObject:activity];
			_activity = _activity->Next;
		}

		self.open = open;
		self.timelineDate = date;
		self.start = startDay;
		self.end = endDay;
		self.timeEntries = [[timeEntries reverseObjectEnumerator] allObjects];
		self.activities = [activities copy];
	}
	return self;
}

@end
