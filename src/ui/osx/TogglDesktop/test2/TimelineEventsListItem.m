//
//  TimelineEventsListItem.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 05/02/16.
//  Copyright © 2016 Alari. All rights reserved.
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

	self.timeLabel.stringValue = view_item.StartTimeString;
	for (TimelineEventView *event in view_item.Events)
	{
		[events appendAttributedString:[event descriptionString]];
	}
	[[self.appTitleTextView textStorage] setAttributedString:events];

	[self.appsBox setHidden:[events length] == 0];
	self.Started = view_item.Started;
	[self.activityCircle setNewValue:view_item.activeDuration];

	if (view_item.EntryEnd != 0)
	{
		// set time entry line location
		uint64_t top = view_item.EntryStart * view_item.CalculatedHeight / 100;
		uint64_t bottom = ((100 - view_item.EntryEnd) / 100) * view_item.CalculatedHeight;

		[self.backgroundBox removeConstraint:self.topConstraint];
		self.topConstraint = [NSLayoutConstraint constraintWithItem:self.entryLine
														  attribute:NSLayoutAttributeTop
														  relatedBy:NSLayoutRelationEqual
															 toItem:self.backgroundBox
														  attribute:NSLayoutAttributeTop
														 multiplier:1
														   constant:top];
		[self.backgroundBox addConstraint:self.topConstraint];


		[self.backgroundBox removeConstraint:self.bottomConstraint];

		self.bottomConstraint = [NSLayoutConstraint constraintWithItem:self.entryLine
															 attribute:NSLayoutAttributeBottom
															 relatedBy:NSLayoutRelationEqual
																toItem:self.backgroundBox
															 attribute:NSLayoutAttributeBottom
															multiplier:1
															  constant:bottom];
		[self.backgroundBox addConstraint:self.bottomConstraint];


		[self.entryLine setToolTip:view_item.EntryDescription];
	}
	[self.entryLine setHidden:!(view_item.EntryEnd != 0)];
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
