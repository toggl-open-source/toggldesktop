//
//  TimelineViewController.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import "TimelineViewController.h"
#import "TimelineChunkView.h"
#import "DisplayCommand.h"
#import "TimelineEventsListItem.h"
#import "TimeEntryCell.h"
#import "UIEvents.h"

@interface TimelineViewController ()
@property NSNib *nibTimelineEventsListItem;
@end

@implementation TimelineViewController

extern void *ctx;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
	self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
	if (self)
	{
		self.nibTimelineEventsListItem = [[NSNib alloc] initWithNibNamed:@"TimelineEventsListItem"
																  bundle:nil];
		timelineChunks = [NSMutableArray array];
		highlightedRows = [NSMutableArray array];

		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimeline:)
													 name:kDisplayTimeline
												   object:nil];
	}
	return self;
}

- (void)loadView
{
	[super loadView];
	[self.eventsTableView registerNib:self.nibTimelineEventsListItem
						forIdentifier :@"TimelineEventsListItem"];
	self.eventsTableView.delegate = self;
	self.eventsTableView.dataSource = self;
	self.startTimeSet = NO;
	self.today = true;

	[NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask handler:^NSEvent * (NSEvent *theEvent) {
		 if (self.isViewLoaded && self.view.window)
		 {
			 switch ([theEvent keyCode])
			 {
				 case 123 :
					 [self prevButtonClicked:nil];
					 return nil;

				 case 124 :
					 [self nextButtonClicked:nil];
					 return nil;

				 default :
					 break;
			 }
		 }
		 return theEvent;
	 }];
}

- (IBAction)prevButtonClicked:(id)sender
{
	toggl_view_timeline_prev_day(ctx);
}

- (IBAction)nextButtonClicked:(id)sender
{
	if (!self.today)
	{
		toggl_view_timeline_next_day(ctx);
	}
}

- (IBAction)createButtonClicked:(id)sender
{
	NSInteger durationSeconds = (self.endItem.Started - self.startItem.Started) / 60;
	NSString *duration = [[NSString alloc] initWithFormat:@"%ld", (long)durationSeconds];

	char_t *guid = toggl_start(ctx,
							   [self.descriptionText.stringValue UTF8String],
							   [duration UTF8String],
							   0,
							   0,
							   0,
							   0,
							   false,
							   0,
							   0);

	NSString *GUID = [NSString stringWithUTF8String:guid];

	free(guid);

	toggl_edit(ctx, [GUID UTF8String], false, kFocusedFieldNameDescription);

	toggl_set_time_entry_start(ctx,
							   [GUID UTF8String],
							   [self.startTimeLabel.stringValue UTF8String]);
}

- (void)startDisplayTimeline:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayTimeline:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayTimeline:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.dateLabel.stringValue = [NSString stringWithFormat:@"Timeline %@", cmd.timelineDate];
	self.today = [cmd.timelineDate isEqualToString:@"Today"];

	[self.nextButton setHidden:self.today];

	@synchronized(timelineChunks)
	{
		[timelineChunks removeAllObjects];
		[timelineChunks addObjectsFromArray:cmd.timelineChunks];
	}

	[self.eventsTableView reloadData];

	// NSLog(@"Chunks size: %lu", (unsigned long)[timelineChunks count]);
	// FIXME: reload view
}

- (long)numberOfRowsInTableView:(NSTableView *)tv
{
	long result = 0;

	@synchronized(timelineChunks)
	{
		result = (long)[timelineChunks count];
	}
	return result;
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if (aTableView == self.eventsTableView)
	{
		if ([[aTableColumn identifier] isEqualToString:@"first"])
		{
			return [timelineChunks objectAtIndex:rowIndex];
		}
	}
	return nil;
}

- (NSView *) tableView:(NSTableView *)tableView
	viewForTableColumn:(NSTableColumn *)tableColumn
				   row:(NSInteger)row
{
	TimelineChunkView *item = nil;

	@synchronized(timelineChunks)
	{
		item = [timelineChunks objectAtIndex:row];
	}
	NSAssert(item != nil, @"view item from timelineChunks array is nil");

	TimelineEventsListItem *cell = [tableView makeViewWithIdentifier:@"TimelineEventsListItem"
															   owner:self];
	[cell render:item];
	return cell;
}

- (CGFloat)tableView:(NSTableView *)tableView
		 heightOfRow:(NSInteger)row
{
	TimelineChunkView *item = nil;

	@synchronized(timelineChunks)
	{
		if (row < timelineChunks.count)
		{
			item = timelineChunks[row];
		}
	}

	return item.CalculatedHeight;
}

- (IBAction)performClick:(id)sender
{
	NSInteger row = [self.eventsTableView clickedRow];

	if (row < 0)
	{
		return;
	}
	self.lastRow = row;
	TimelineChunkView *item = 0;
	@synchronized(timelineChunks)
	{
		item = timelineChunks[row];
	}

	TimelineEventsListItem *cell = [self getCellByRow:row];

	// save start or stop time cell items
	if (self.startTimeSet)
	{
		if (row < self.startItem.rowIndex)
		{
			[self clearHighlights];
			[self.startItem setUnSelected];
			[cell setSelected:NO row:row];
			self.startItem = cell;
			self.startTimeLabel.stringValue = cell.timeLabel.stringValue;
		}
		else
		{
			[self.endItem setUnSelected];
			[cell setSelected:self.startTimeSet row:row];
			self.endItem = cell;
			self.startTimeSet = NO;
			self.endTimeLabel.stringValue = cell.timeLabel.stringValue;
			[self setHighlights];
		}
	}
	else
	{
		[self clearHighlights];
		[cell setSelected:self.startTimeSet row:row];
		self.startItem = cell;
		self.startTimeSet = YES;
		self.startTimeLabel.stringValue = cell.timeLabel.stringValue;
	}
}

- (void)clearHighlights
{
	while ([highlightedRows count] != 0)
	{
		[highlightedRows.lastObject setUnSelected];
		[highlightedRows removeLastObject];
	}
}

- (void)setHighlights
{
	NSInteger currentRow = self.endItem.rowIndex - 1;
	TimelineEventsListItem *item;

	[highlightedRows addObject:self.endItem];

	while (currentRow > self.startItem.rowIndex)
	{
		item = [self getCellByRow:currentRow];
		[item setActive:NO];
		[highlightedRows addObject:item];
		currentRow--;
	}
	[highlightedRows addObject:self.startItem];
}

- (TimelineEventsListItem *)getCellByRow:(NSInteger)row
{
	NSView *latestView = [self.eventsTableView rowViewAtRow:row
											makeIfNecessary  :NO];

	if (latestView == nil)
	{
		return nil;
	}

	for (NSView *subview in [latestView subviews])
	{
		if ([subview isKindOfClass:[TimelineEventsListItem class]])
		{
			return (TimelineEventsListItem *)subview;
		}
	}
	return nil;
}

@end
