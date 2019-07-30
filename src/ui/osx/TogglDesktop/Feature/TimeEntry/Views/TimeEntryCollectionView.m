//
//  TimeEntryCollectionView.m
//  TogglDesktop
//
//  Created by Nghia Tran on 2/20/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "TimeEntryCollectionView.h"
#import "TimeEntryCell.h"
#import "UIEvents.h"
#import "TogglDesktop-Swift.h"
#include <Carbon/Carbon.h>

@interface TimeEntryCollectionView ()
@property (assign, nonatomic) NSIndexPath *latestSelectedIndexPath;
@end

@implementation TimeEntryCollectionView

extern void *ctx;

- (void)awakeFromNib
{
	[super awakeFromNib];
}

- (BOOL)isFlipped
{
	return YES;
}

- (void)mouseUp:(NSEvent *)event {
	[super mouseUp:event];

	if (@available(macOS 10.12, *))
	{
		// >= macOS 10.12, the mouse Up is executed when selecting the cell
		[self handleMouseSelectionWithEvent:event];
	}
}

- (void)mouseDown:(NSEvent *)event {
	[super mouseDown:event];

	if (@available(macOS 10.12, *))
	{
		// Do nothing
	}
	else
	{
		// In macOS 10.11, the -mouseUp doesn't execute, so we have to handle the logic here
		// if we call this logic in macOS >= 10.12 => The selection doesn't update
		[self handleMouseSelectionWithEvent:event];
	}
}

- (void)handleMouseSelectionWithEvent:(NSEvent *)event
{
	if ([event clickCount] > 1)
	{
		return;
	}

	NSPoint curPoint = [self convertPoint:[event locationInWindow] fromView:nil];
	NSIndexPath *index = [self indexPathForItemAtPoint:curPoint];
	NSCollectionViewItem *item = [self itemAtIndexPath:index];

	if ([item isKindOfClass:[TimeEntryCell class]])
	{
		TimeEntryCell *timeCell = (TimeEntryCell *)item;

		// We have to store the click index
		// so, the displayTimeEntryEditor can detect which cell should be show popover
		self.clickedIndexPath = index;

		// Show popover or open group
		if (timeCell.cellType == CellTypeGroup)
		{
			[[NSNotificationCenter defaultCenter] postNotificationName:kToggleGroup object:timeCell.GroupName];
		}
		else
		{
			[timeCell focusFieldName];
		}
	}
}

- (void)keyDown:(NSEvent *)event {
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
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kEscapeListing
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
	else if (event.keyCode == kVK_Space)
	{
		TimeEntryCell *cell = [self getSelectedEntryCell];
		if (cell != nil)
		{
			toggl_continue(ctx, [cell.GUID UTF8String]);
		}
	}
	else if (event.keyCode == kVK_UpArrow)
	{
		NSIndexPath *index = [self.selectionIndexPaths.allObjects firstObject];
		if (index != nil)
		{
			[self selectPreviousRowFromIndexPath:index];
		}
	}
	else
	{
		[super keyDown:event];
	}
}

- (TimeEntryCell *)getSelectedEntryCell
{
	if (self.selectionIndexPaths.count == 0)
	{
		return nil;
	}
	self.latestSelectedIndexPath = [[self.selectionIndexPaths allObjects] firstObject];

	id view = [self itemAtIndexPath:self.latestSelectedIndexPath];
	if ([view isKindOfClass:[TimeEntryCell class]])
	{
		return (TimeEntryCell *)view;
	}
	return nil;
}

- (void)deleteEntry
{
	TimeEntryCell *cell = [self getSelectedEntryCell];

	if (cell != nil)
	{
		// If description is empty and duration is less than 15 seconds delete without confirmation
		if (cell.confirmless_delete)
		{
			if (toggl_delete_time_entry(ctx, [cell.GUID UTF8String]))
			{
				[self selectPreviousRowFromIndexPath:self.latestSelectedIndexPath];
			}
			return;
		}
		NSString *msg = [NSString stringWithFormat:@"Delete time entry \"%@\"?", cell.descriptionTextField.stringValue];

		NSAlert *alert = [[NSAlert alloc] init];
		[alert addButtonWithTitle:@"OK"];
		[alert addButtonWithTitle:@"Cancel"];
		[alert setMessageText:msg];
		[alert setInformativeText:@"Deleted time entries cannot be restored."];
		[alert setAlertStyle:NSWarningAlertStyle];
		if ([alert runModal] != NSAlertFirstButtonReturn)
		{
			return;
		}

		NSLog(@"Deleting time entry %@", cell.GUID);

		if (toggl_delete_time_entry(ctx, [cell.GUID UTF8String]))
		{
			[self selectPreviousRowFromIndexPath:self.latestSelectedIndexPath];
		}
	}
}

- (void)selectPreviousRowFromIndexPath:(NSIndexPath *)indexPath
{
	if (indexPath == nil)
	{
		return;
	}

	if (![self.dataSource isKindOfClass:[TimeEntryDatasource class]])
	{
		return;
	}
	TimeEntryDatasource *datasource = (TimeEntryDatasource *)self.dataSource;

	// Select previous cell
	NSIndexPath *previousIndexPath = [datasource previousIndexPathFrom:indexPath];
	if (previousIndexPath != nil)
	{
		[self deselectAll:self];
		[self selectItemsAtIndexPaths:[NSSet setWithCollectionViewIndexPath:previousIndexPath]
					   scrollPosition:NSCollectionViewScrollPositionNone];

		// Scroll to visible selected row
		NSCollectionViewLayoutAttributes *attribute = [self layoutAttributesForItemAtIndexPath:previousIndexPath];
		if (!NSContainsRect(self.visibleRect, attribute.frame))
		{
			[self scrollToItemsAtIndexPaths:[NSSet setWithCollectionViewIndexPath:previousIndexPath]
							 scrollPosition:NSCollectionViewScrollPositionTop];
		}
	}
}

@end
