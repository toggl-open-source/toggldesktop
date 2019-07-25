//
//  AutoCompleteTable.m
//  LiteComplete
//
//  Created by Indrek Vändrik on 21/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import "AutoCompleteTable.h"
#import "AutoCompleteTableCell.h"

@interface AutoCompleteTable ()
@property (nonatomic, assign) NSInteger lastSelected;
@property (nonatomic, assign) NSInteger lastClicked;
@end

@implementation AutoCompleteTable

- (instancetype)initWithFrame:(NSRect)frame
{
	self = [super initWithFrame:frame];
	if (self)
	{
		self.lastSelected = -1;
		NSTableColumn *column = [[NSTableColumn alloc] initWithIdentifier:@"column"];
		column.minWidth = 100;
		[self addTableColumn:column];
		[self setHeaderView:nil];
		[self setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleNone];
		[self setIntercellSpacing:NSMakeSize(0, 0)];
	}
	return self;
}

- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint globalLocation = [theEvent locationInWindow];
	NSPoint localLocation = [self convertPoint:globalLocation fromView:nil];

	self.lastClicked = [self rowAtPoint:localLocation];
	[super mouseDown:theEvent];
}

- (void)drawGridInClipRect:(NSRect)clipRect
{
	NSRect lastRowRect = [self rectOfRow:[self numberOfRows] - 1];
	NSRect myClipRect = NSMakeRect(0, 0, lastRowRect.size.width, NSMaxY(lastRowRect));
	NSRect finalClipRect = NSIntersectionRect(clipRect, myClipRect);

	[super drawGridInClipRect:finalClipRect];
}

- (void)setFirstRowAsSelected
{
	[self deselectAll:nil];

	NSIndexSet *indexSet = [NSIndexSet indexSetWithIndex:0];
	[self selectRowIndexes:indexSet byExtendingSelection:NO];

	[self setCurrentSelected:1 next:YES];
}

- (void)nextItem
{
	if (self.lastSelected < self.numberOfRows - 1)
	{
		[self setCurrentSelected:self.lastSelected + 1 next:YES];
	}
}

- (void)previousItem
{
	if (self.lastSelected > 0)
	{
		[self setCurrentSelected:self.lastSelected - 1 next:NO];
	}
}

- (void)setCurrentSelected:(NSInteger)index next:(BOOL)isNext
{
	AutoCompleteTableCell *cell = [self getSelectedCell:index];

	if (cell != nil)
	{
		// item not selectable, jump to next/previous item
		if (!cell.isSelectable)
		{
			if (index < 2 && !isNext)
			{
				[self scrollRowToVisible:0];

				if (index == 0)
				{
					[self setCurrentSelected:self.numberOfRows - 1 next:NO];
					return;
				}
			}
			[self resetSelected];
			self.lastSelected = index;

			if (isNext)
			{
				[self nextItem];
			}
			else
			{
				[self previousItem];
			}
			return;
		}
		[self resetSelected];
		self.lastSelected = index;

		[cell setFocused:YES];
		if (self.lastSelected == 1)
		{
			[self scrollRowToVisible:0];
		}
		else
		{
			[self scrollRowToVisible:self.lastSelected];
		}
	}
}

- (AutoCompleteTableCell *)getSelectedCell:(NSInteger)row
{
	if (row < 0)
	{
		return nil;
	}

	NSView *latestView = [self rowViewAtRow:row makeIfNecessary:YES];

	if (latestView == nil)
	{
		return nil;
	}

	for (NSView *subview in [latestView subviews])
	{
		if ([subview isKindOfClass:[AutoCompleteTableCell class]])
		{
			return (AutoCompleteTableCell *)subview;
		}
	}

	return nil;
}

- (void)resetSelected
{
	if (self.lastSelected != -1)
	{
		AutoCompleteTableCell *cell = [self getSelectedCell:self.lastSelected];
		[cell setFocused:NO];
	}
	self.lastSelected = -1;
}

@end

