//
//  NSUnstripedTableView.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 04/02/2014.
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import "NSUnstripedTableView.h"
#include <Carbon/Carbon.h>
#import "UIEvents.h"
#import "TimeEntryCell.h"
#import "TimeEntryCellWithHeader.h"

@implementation NSUnstripedTableView

- (void)drawGridInClipRect:(NSRect)clipRect
{
	NSRect lastRowRect = [self rectOfRow:[self numberOfRows] - 1];
	NSRect myClipRect = NSMakeRect(0, 0, lastRowRect.size.width, NSMaxY(lastRowRect));
	NSRect finalClipRect = NSIntersectionRect(clipRect, myClipRect);

	[super drawGridInClipRect:finalClipRect];
}

- (void)keyDown:(NSEvent *)event
{
	if ((event.keyCode == kVK_Return) || (event.keyCode == kVK_ANSI_KeypadEnter))
	{
		NSView *latestView = [self rowViewAtRow:[self selectedRow]
								makeIfNecessary  :NO];

		for (NSView *subview in [latestView subviews])
		{
			if ([subview isKindOfClass:[TimeEntryCell class]] || [subview isKindOfClass:[TimeEntryCellWithHeader class]])
			{
				[(TimeEntryCell *)subview openEdit];
			}
		}
	}
	else if (event.keyCode == kVK_Escape)
	{
		[[NSNotificationCenter defaultCenter] postNotificationName:kFocusTimer
															object:nil
														  userInfo:nil];
	}
	else
	{
		[super keyDown:event];
	}
}

@end
