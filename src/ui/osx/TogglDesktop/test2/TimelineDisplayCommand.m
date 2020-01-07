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
		if (view->DurationString)
		{
			self.durationStr = [NSString stringWithUTF8String:view->DurationString];
		}
		else
		{
			self.durationStr = @"";
		}
		self.duration = view->Duration;
		self.isHeader = view->Header;

		// Load app subevents
		NSMutableArray<TimelineDisplayEvent *> *subEvents = [@[] mutableCopy];
		TogglTimelineEventView *sub_it = view->Event;
		while (sub_it)
		{
			TimelineDisplayEvent *event = [[TimelineDisplayEvent alloc] initWithView:sub_it];
			[subEvents addObject:event];
			sub_it = sub_it->Next;
		}
		self.subEvents = [subEvents copy];
	}
	return self;
}

@end

@implementation TimelineDisplayActivity

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
		if (view->EndTimeString)
		{
			self.endedTimeString = [NSString stringWithUTF8String:view->EndTimeString];
		}
		else
		{
			self.endedTimeString = @"";
		}

		// Events
		NSMutableArray<TimelineDisplayEvent *> *events = [@[] mutableCopy];
		NSTimeInterval duration = 0;
		TogglTimelineEventView *_event = view->FirstEvent;
		while (_event)
		{
			TimelineDisplayEvent *event = [[TimelineDisplayEvent alloc] initWithView:_event];
			duration += event.duration;
			[events addObject:event];
			_event = _event->Next;
		}
		self.duration = duration;
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
		self.activities = [[activities reverseObjectEnumerator] allObjects];
	}
	return self;
}

@end
