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

extern void *ctx;

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
		TimeEntryCell *cell = [self getSelectedEntryCell];
		if (cell != nil)
		{
			[cell openEdit];
		}
	}
	else if (event.keyCode == kVK_Escape)
	{
		[[NSNotificationCenter defaultCenter] postNotificationName:kEscapeListing
															object:nil
														  userInfo:nil];
	}
	else if (event.keyCode == kVK_Delete)
	{
		[self deleteEntry];
	}
	else
	{
		[super keyDown:event];
	}
}

- (TimeEntryCell *)getSelectedEntryCell
{
	if (self.selectedRow != -1)
	{
		self.latestSelectedRow = self.selectedRow;
	}
	NSView *latestView = [self rowViewAtRow:[self latestSelectedRow]
							makeIfNecessary  :NO];

	for (NSView *subview in [latestView subviews])
	{
		if ([subview isKindOfClass:[TimeEntryCell class]] || [subview isKindOfClass:[TimeEntryCellWithHeader class]])
		{
			return (TimeEntryCell *)subview;
		}
	}
	return nil;
}

- (void)deleteEntry
{
	TimeEntryCell *cell = [self getSelectedEntryCell];

	if (cell != nil)
	{
		NSAlert *alert = [[NSAlert alloc] init];
		[alert addButtonWithTitle:@"OK"];
		[alert addButtonWithTitle:@"Cancel"];
		[alert setMessageText:@"Delete the time entry?"];
		[alert setInformativeText:@"Deleted time entries cannot be restored."];
		[alert setAlertStyle:NSWarningAlertStyle];
		if ([alert runModal] != NSAlertFirstButtonReturn)
		{
			return;
		}

		NSLog(@"Deleting time entry %@", cell.GUID);

		toggl_delete_time_entry(ctx, [cell.GUID UTF8String]);
	}
}

@end
