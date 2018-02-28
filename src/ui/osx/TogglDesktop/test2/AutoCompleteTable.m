//
//  AutoCompleteTable.m
//  LiteComplete
//
//  Created by Indrek Vändrik on 21/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import "AutoCompleteTable.h"
#include <Carbon/Carbon.h>

@implementation AutoCompleteTable

- (instancetype)initWithFrame:(NSRect)frame
{
	self = [super initWithFrame:frame];
	if (self)
	{
		self.lastSelected = -1;
		NSTableColumn *column = [[NSTableColumn alloc] initWithIdentifier:@"column"];
		column.width = 400;
		[self addTableColumn:column];
		[self setHeaderView:nil];
		[self setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleNone];
		[self setIntercellSpacing:NSMakeSize(0, 0)];
	}
	return self;
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

	[self setCurrentSelected:0];
}

- (void)nextItem
{
	if (self.lastSelected < self.numberOfRows - 1)
	{
		[self setCurrentSelected:self.lastSelected + 1];
	}
}

- (void)previousItem
{
	if (self.lastSelected > 0)
	{
		[self setCurrentSelected:self.lastSelected - 1];
	}
}

- (void)setCurrentSelected:(NSInteger)index
{
	[self resetSelected];
	self.lastSelected = index;
	AutoCompleteTableCell *cell = [self getSelectedCell:index];
	if (cell != nil)
	{
		[cell setFocused:YES];
		[self scrollRowToVisible:self.lastSelected];
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

