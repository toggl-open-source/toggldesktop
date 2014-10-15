//
//  NSResize.m
//  Toggl Desktop on the Mac
//
//  Created by Indrek Vändrik on 14/10/14.
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import "NSResize.h"

@implementation NSResize

- (void)resetCursorRects
{
	if (self.cursor)
	{
		[self addCursorRect:[self bounds] cursor:self.cursor];
	}
	else
	{
		[super resetCursorRects];
	}
}

- (BOOL)acceptsFirstMouse:(NSEvent *)e
{
	return YES;
}

- (void)mouseDown:(NSEvent *)e
{
	[self sendAction:@selector(draggingResizeStart:) to:[self delegate]];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	[[NSCursor resizeLeftRightCursor] set];
	[self sendAction:@selector(draggingResize:) to:[self delegate]];
}

@end
