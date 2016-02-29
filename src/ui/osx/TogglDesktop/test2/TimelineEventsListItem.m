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
	self.activityCircle.value = MIN(view_item.activeDuration, 900);
}

- (void)setSelected:(BOOL)endTime row:(NSInteger)rowIndex;
{
	self.rowIndex = rowIndex;
	NSString *aString;
	[self setActive:YES];

	NSImage *aImage = [NSImage imageNamed:@"NSAutosaveTriangle.pdf"];
	if (endTime)
	{
		// Flip the down arrow image vertically
		NSAffineTransform *flipper = [NSAffineTransform transform];
		NSSize dimensions = aImage.size;
		[aImage lockFocus];

		[flipper scaleXBy:1.0 yBy:-1.0];
		[flipper set];

		[aImage drawAtPoint:NSMakePoint(0, -dimensions.height)
				   fromRect:NSMakeRect(0, 0, dimensions.width, dimensions.height)
				  operation:NSCompositeCopy fraction:1.0];

		[aImage unlockFocus];

		aString = @"End time";
	}
	else
	{
		aString = @"Start time";
	}

	self.selectedLabel.stringValue = aString;
	[self.selectedLabel setHidden:NO];

	[self.arrowImage setImage:aImage];
}

- (void)setActive:(BOOL)edge
{
	if (!edge)
	{
		[self.arrowImage setHidden:YES];
	}
	NSColor *color = [ConvertHexColor hexCodeToNSColor:@"#F0F0F0"];
	[self.backgroundBox setFillColor:color];
}

- (void)setUnSelected
{
	[self.selectedLabel setHidden:YES];
	[self.arrowImage setHidden:NO];
	[self.backgroundBox setFillColor:[ConvertHexColor hexCodeToNSColor:@"#EBEBEB"]];
	[self.arrowImage setImage:[NSImage imageNamed:@"NSRightFacingTriangleTemplate.pdf"]];
}

@end
