//
//  TimelineChunkCircle.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import "TimelineChunkCircle.h"
#import "ConvertHexColor.h"

@implementation TimelineChunkCircle

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];

	// Drawing code here.
	if (self.value)
	{
		[self drawCircle];
	}
}

- (void)drawCircle
{
	NSBezierPath *path = [NSBezierPath bezierPathWithOvalInRect:NSInsetRect([self bounds], 0, 0)];

	if (self.value >= 900)
	{
		[[ConvertHexColor hexCodeToNSColor:@"#2CC1E6"] set];
		[path fill];
		[[ConvertHexColor hexCodeToNSColor:@"#C6C6C6"] set];
		[path setLineWidth:1.0];
		[path stroke];
	}
	else
	{
		[[ConvertHexColor hexCodeToNSColor:@"#F0F0F0"] set];
		[path fill];
		[[NSColor whiteColor] set];
		[path setLineWidth:2.0];
		[path stroke];

		// pie chart radius and degrees
		double radius = self.bounds.size.height / 2;
		double start = 90.0;
		double end = start - ((self.value * 360) / 900);

		NSPoint center = NSMakePoint(radius, radius);

		NSBezierPath *sector = [NSBezierPath bezierPath];
		[[ConvertHexColor hexCodeToNSColor:@"#2CC1E6"] set];
		[sector moveToPoint:center];
		[sector appendBezierPathWithArcWithCenter:center radius:radius startAngle:start endAngle:end clockwise:YES];
		[sector lineToPoint:center];
		[sector fill];
		[[ConvertHexColor hexCodeToNSColor:@"#C6C6C6"] set];
		[sector setLineWidth:1.0];
		[sector stroke];
	}

	// Circle on top to make pie chart a donut
	NSBezierPath *topCirclePath = [NSBezierPath bezierPathWithOvalInRect:NSInsetRect(NSMakeRect(5, 5, 20, 20), 0, 0)];
	[[ConvertHexColor hexCodeToNSColor:@"#FFFFFF"] set];
	[topCirclePath fill];
}

- (void)setNewValue:(int)newValue
{
	self.value = MIN(newValue, 900);
	[self setNeedsDisplay:YES];
}

@end
