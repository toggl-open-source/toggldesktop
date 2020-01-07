//
//  TimelineLines.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import "TimelineLines.h"

@implementation TimelineLines

- (void)setNewValue:(TimelineChunkView *)view
{
	uint64_t bottom, top, height;

	top = view.EntryStart * view.CalculatedHeight / 100;

	if (view.EntryEnd == 100)
	{
		bottom = 0;
	}
	else
	{
		bottom = ((100 - view.EntryEnd) / 100) * view.CalculatedHeight;
	}

	height = view.CalculatedHeight - top - bottom;

	[self setNewBottom:bottom newHeight:height];
}

- (void)setNewBottom:(uint64_t)bottom newHeight:(uint64_t)height
{
	self.grayColor = [ConvertHexColor hexCodeToNSColor:@"#C6C6C6"];
	self.blueColor = [ConvertHexColor hexCodeToNSColor:@"#2CC1E6"];
	self.bottom = bottom;
	self.height = height;
	[self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect
{
	// Timeline Gray Line
	[self.grayColor set];

	// Vertical line
	[NSBezierPath fillRect:NSMakeRect(10, 0, 1, [self bounds].size.height)];

	// Horizontal line
	[NSBezierPath fillRect:NSMakeRect(0, [self bounds].size.height - 1, 10, 1)];


	if (self.height > 0)
	{
		// Time entry blue line
		[self.blueColor set];
		[NSBezierPath fillRect:NSMakeRect(3, self.bottom, 4, self.height)];
	}
}

@end
