//
//  NSTextFieldVerticallyAligned.m
//  Toggl Desktop on the Mac
//
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import "NSTextFieldVerticallyAligned.h"

@implementation NSTextFieldVerticallyAligned

- (NSRect)adjustedFrameToVerticallyCenterText:(NSRect)frame
{
	// super would normally draw text at the top of the cell
	NSInteger offset = floor((NSHeight(frame) -
							  ([[self font] ascender] - [[self font] descender])) / 3) - 1;

	return NSInsetRect(frame, 5.0, offset);
}

- (void)editWithFrame:(NSRect)aRect inView:(NSView *)controlView
			   editor:(NSText *)editor delegate:(id)delegate event:(NSEvent *)event
{
	[super editWithFrame:[self adjustedFrameToVerticallyCenterText:aRect]
				  inView:controlView editor:editor delegate:delegate event:event];
}

- (void)selectWithFrame:(NSRect)aRect inView:(NSView *)controlView
				 editor:(NSText *)editor delegate:(id)delegate
				  start:(NSInteger)start length:(NSInteger)length
{
	[super selectWithFrame:[self adjustedFrameToVerticallyCenterText:aRect]
					inView:controlView editor:editor delegate:delegate
					 start:start length:length];
}

- (void)drawInteriorWithFrame:(NSRect)frame inView:(NSView *)view
{
	[super drawInteriorWithFrame:
	 [self adjustedFrameToVerticallyCenterText:frame] inView:view];
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	NSBezierPath *betterBounds = [NSBezierPath bezierPathWithRoundedRect:cellFrame xRadius:3 yRadius:3];

	[betterBounds addClip];
	[super drawWithFrame:cellFrame inView:controlView];
	if (self.isBezeled)
	{
		[betterBounds setLineWidth:2];
		[self.backgroundColor setStroke];
		[betterBounds stroke];
	}
}

@end
