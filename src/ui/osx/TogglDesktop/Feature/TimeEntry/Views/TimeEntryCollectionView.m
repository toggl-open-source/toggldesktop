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
#import "Utils.h"

@interface TimeEntryCollectionView ()
@property (strong, nonatomic) NSIndexPath *latestSelectedIndexPath;
@property (strong, nonatomic) NSSet<NSIndexPath *> *previousSelectionSet;
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
	// Store previous selection before any actions
	// To deselect the selected item if need
	self.previousSelectionSet = [self.selectionIndexPaths copy];
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

	if (index && [item isKindOfClass:[TimeEntryCell class]])
	{
		TimeEntryCell *timeCell = (TimeEntryCell *)item;

		// We have to store the click index
		// so, the displayTimeEntryEditor can detect which cell should be show popover
		self.clickedIndexPath = index;

		// If we're pressing SHIFT and click
		// Don't show Editor, just select
		NSUInteger flags = [[NSApp currentEvent] modifierFlags];
		if (flags & NSShiftKeyMask)
		{
			NSSet *set = [NSSet setWithObject:index];

			// Select or deselect
			if ([self.previousSelectionSet containsObject:index])
			{
				[self deselectItemsAtIndexPaths:set];
			}
			else
			{
				[self selectItemsAtIndexPaths:set scrollPosition:NSCollectionViewScrollPositionLeft];
			}

			return;
		}

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
	NSArray<TimeEntryCell *> *cells = [self getSelectedEntryCells];

	// Ignore if there is no selection
	if (cells == nil)
	{
		[super keyDown:event];
		return;
	}

	if ((event.keyCode == kVK_Return) || (event.keyCode == kVK_ANSI_KeypadEnter))
	{
		TimeEntryCell *cell = cells.firstObject;
		if (cell != nil)
		{
			if (cell.Group)
			{
				if (cell.GroupName.length)
				{
					toggl_toggle_entries_group(ctx, [cell.GroupName UTF8String]);
				}
			}
			else
			{
				[cell openEdit];
			}
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
		for (TimeEntryCell *cell in cells)
		{
			if (cell != nil && cell.GroupName.length && !cell.GroupOpen)
			{
				toggl_toggle_entries_group(ctx, [cell.GroupName UTF8String]);
			}
		}
	}
	else if (event.keyCode == kVK_LeftArrow)
	{
		for (TimeEntryCell *cell in cells)
		{
			if (cell != nil && cell.GroupName.length && cell.GroupOpen)
			{
				toggl_toggle_entries_group(ctx, [cell.GroupName UTF8String]);
			}
		}
	}
	else if (event.keyCode == kVK_Space)
	{
		// Only start TE if we select one TE
		if (cells.count >= 2)
		{
			[super keyDown:event];
			return;
		}

		TimeEntryCell *cell = cells.firstObject;
		if (cell != nil)
		{
			toggl_continue(ctx, [cell.GUID UTF8String]);

			// Focus on timer
			[[NSNotificationCenter defaultCenter] postNotificationName:kFocusTimer object:nil];
		}
	}
	else if (event.keyCode == kVK_UpArrow)
	{
		NSIndexPath *index = [self.selectionIndexPaths.allObjects lastObject];
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

- (NSArray<TimeEntryCell *> *)getSelectedEntryCells
{
	if (self.selectionIndexPaths.count == 0)
	{
		return nil;
	}
	self.latestSelectedIndexPath = [[self.selectionIndexPaths allObjects] lastObject];

	// Get all selected cells
	NSMutableArray<TimeEntryCell *> *items = [@[] mutableCopy];
	for (NSIndexPath *indexPath in self.selectionIndexPaths.allObjects)
	{
		id view = [self itemAtIndexPath:indexPath];
		if ([view isKindOfClass:[TimeEntryCell class]])
		{
			[items addObject:(TimeEntryCell *)view];
		}
	}
	return [items copy];
}

- (void)deleteEntry
{
	NSArray<TimeEntryCell *> *cells = [self getSelectedEntryCells];

	if (cells.count == 0)
	{
		return;
	}

	if (cells.count >= 2)
	{
		NSString *msg = [NSString stringWithFormat:@"Are you sure to delete %lu items", (unsigned long)cells.count];

		NSAlert *alert = [[NSAlert alloc] init];
		[alert addButtonWithTitle:@"Delete"];
		[alert addButtonWithTitle:@"Cancel"];
		[alert setMessageText:msg];
		[alert setInformativeText:@"Deleted time entries cannot be restored."];
		[alert setAlertStyle:NSWarningAlertStyle];
		if ([alert runModal] != NSAlertFirstButtonReturn)
		{
			return;
		}

		// Delete all
		[self deleteTimeEntries:cells];
	}
	else
	{
		// Delete single with confirmation
		TimeEntryCell *cell = cells.firstObject;
		[self deleteTimeEntry:cell];
	}
}

- (void)deleteTimeEntries:(NSArray<TimeEntryCell *> *)cells
{
	for (TimeEntryCell *cell in cells)
	{
		toggl_delete_time_entry(ctx, [cell.GUID UTF8String]);
	}
	[self selectPreviousRowFromIndexPath:self.latestSelectedIndexPath];
}

- (void)deleteTimeEntry:(TimeEntryCell *)cell
{
	if (cell == nil)
	{
		return;
	}

	// If description is empty and duration is less than 15 seconds delete without confirmation
	if (cell.confirmless_delete)
	{
        [[DesktopLibraryBridge shared] deleteTimeEntryItem:cell.item undoManager:self.undoManager];
        [self selectPreviousRowFromIndexPath:self.latestSelectedIndexPath];
		return;
	}

	// Delete and select preview cell
    [[DesktopLibraryBridge shared] deleteTimeEntryItem:cell.item undoManager:self.undoManager];
    [self selectPreviousRowFromIndexPath:self.latestSelectedIndexPath];
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
		// deselect all previous if we don't hold Shift
		NSUInteger flags = [[NSApp currentEvent] modifierFlags];
		if (flags & NSShiftKeyMask)
		{
			if ([self.selectionIndexPaths.allObjects containsObject:previousIndexPath])
			{
				[self deselectItemsAtIndexPaths:[NSSet setWithCollectionViewIndexPath:indexPath]];
			}
		}
		else
		{
			[self deselectAll:self];
		}

		// Skip if the indexpath is invalid
		if ([self itemAtIndexPath:previousIndexPath] == nil)
		{
			return;
		}

		// Select previous cell
		[self selectItemsAtIndexPaths:[NSSet setWithCollectionViewIndexPath:previousIndexPath]
					   scrollPosition:NSCollectionViewScrollPositionNone];
		self.latestSelectedIndexPath = previousIndexPath;

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
