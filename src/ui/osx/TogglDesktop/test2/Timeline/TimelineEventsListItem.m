//
//  TimelineEventsListItem.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import "TimelineEventsListItem.h"
#import "TimelineEventView.h"
#import "ConvertHexColor.h"

@implementation TimelineEventsListItem

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];

	// Drawing code here.
}

- (void)render:(TimelineChunkView *)view_item
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSMutableAttributedString *events = [[NSMutableAttributedString alloc] initWithString:@""];

	self.Started = view_item.Started;
	self.timeLabel.stringValue = view_item.StartTimeString;

	for (TimelineEventView *event in view_item.Events)
	{
		[events appendAttributedString:[event descriptionString]];
	}

	[[self.appTitleTextView textStorage] setAttributedString:events];

	//    [self.appsBox setHidden:[events length] == 0];

	[self.barChart setNewValue:view_item.activeDuration];
	[self.lines setNewValue:view_item];
}

- (void)setSelected:(BOOL)endTime row:(NSInteger)rowIndex;
{
	self.rowIndex = rowIndex;
	NSString *aString;
	[self setActive:YES];

	if (endTime)
	{
		aString = @"End time";
	}
	else
	{
		aString = @"Start time";
	}

	self.selectedLabel.stringValue = aString;
	[self.selectedLabel setHidden:NO];
}

- (void)setActive:(BOOL)edge
{
	[self.backgroundBox setFillColor:[ConvertHexColor hexCodeToNSColor:@"#F0F0F0"]];
}

- (void)setUnSelected
{
	[self.backgroundBox setFillColor:[ConvertHexColor hexCodeToNSColor:@"#FFFFFF"]];
	[self.selectedLabel setHidden:YES];
}

@end
