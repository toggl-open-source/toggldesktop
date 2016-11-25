//
//  NSUnstripedTableView.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 04/02/2014.
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import "NSUnstripedTableView.h"

#include <Carbon/Carbon.h>

#import "TimeEntryCell.h"
#import "TimeEntryCellWithHeader.h"
#import "UIEvents.h"

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
	else if (event.keyCode == kVK_RightArrow)
	{
		TimeEntryCell *cell = [self getSelectedEntryCell];
		if (cell != nil && cell.GroupName.length && !cell.GroupOpen)
		{
			toggl_toggle_entries_group(ctx, [cell.GroupName UTF8String]);
		}
	}
	else if (event.keyCode == kVK_LeftArrow)
	{
		TimeEntryCell *cell = [self getSelectedEntryCell];
		if (cell != nil && cell.GroupName.length && cell.GroupOpen)
		{
			toggl_toggle_entries_group(ctx, [cell.GroupName UTF8String]);
		}
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
		// If description is empty and duration is less than 15 seconds delete without confirmation
		if (cell.confrimless_delete)
		{
			toggl_delete_time_entry(ctx, [cell.GUID UTF8String]);
			return;
		}

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
