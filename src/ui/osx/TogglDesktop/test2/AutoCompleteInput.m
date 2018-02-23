//
//  AutoComleteInput.m
//  LiteComplete
//
//  Created by Indrek Vändrik on 20/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import "AutoCompleteInput.h"

@implementation AutoCompleteInput

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];

	// Drawing code here.
}

- (id)initWithCoder:(NSCoder *)coder
{
	self = [super initWithCoder:coder];
	if (self)
	{
		// self.rect = NSMakeRect(0, 0, 0, 0);
		self.posY = 0;
		NSLog(@"// ** Create ViewItems");
		viewitems = [NSMutableArray array];
		[viewitems addObject:@"Test1"];
		[viewitems addObject:@"Test2"];
		[viewitems addObject:@"Test3"];
		[viewitems addObject:@"Test4"];
		[viewitems addObject:@"Test5"];

		NSRect s = self.frame;
		int h = 200;
		self.autocompleteTableContainer = [[NSScrollView alloc] initWithFrame:NSMakeRect(s.origin.x, s.origin.y - h, s.size.width, h)];
		self.nibAutoCompleteTableCell = [[NSNib alloc] initWithNibNamed:@"AutoCompleteTableCell" bundle:nil];
		self.autocompleteTableView = [[AutoCompleteTable alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
		[self.autocompleteTableView registerNib:self.nibAutoCompleteTableCell
								  forIdentifier :@"AutoCompleteTableCell"];

		[self.autocompleteTableView setDelegate:self];
		[self.autocompleteTableView setBackgroundColor:NSColor.greenColor];

		[self.autocompleteTableContainer setDocumentView:self.autocompleteTableView];
		[self.autocompleteTableContainer setAutohidesScrollers:YES];
		[self.autocompleteTableContainer setHasVerticalScroller:YES];
	}
	return self;
}

- (void)setPos:(int)posy
{
	self.posY = posy;
}

- (void)toggleTableView:(int)itemCount
{
	NSLog(@"// ** Toggle table (items: %d)  ** //", itemCount);
	if (itemCount > 0 || (itemCount == 0 && self.lastItemCount > 0))
	{
		[self updateDropdownHeight:itemCount];
		if (self.autocompleteTableView.listVisible == NO)
		{
			[self.window.contentView addSubview:self.autocompleteTableContainer positioned:NSWindowAbove relativeTo:nil];
			self.autocompleteTableView.listVisible = YES;
		}
		// [self.autocompleteTableView reloadData];
	}
	else
	{
		self.autocompleteTableView.listVisible = NO;
		[self.autocompleteTableContainer removeFromSuperview];
	}
	self.lastItemCount = itemCount;
}

- (void)updateDropdownHeight:(int)count
{
	int itemHeight = 35;
	int h = count * itemHeight;

	NSLog(@"Update table position | H: %d, POSY: %d", h, self.posY);
	[self.autocompleteTableContainer setFrame:NSMakeRect(
		 self.frame.origin.x,
		 self.frame.origin.y - h + self.posY,
		 self.frame.size.width,
		 h)];
}

- (void)keyUp:(NSEvent *)event
{
	NSLog(@"EventCode: %hu", [event keyCode]);
	if ([event keyCode] == kVK_DownArrow)
	{
		if (self.autocompleteTableView.listVisible == NO)
		{
			[self toggleTableView:(int)self.autocompleteTableView.numberOfRows];
		}
		[[self window] makeFirstResponder:self.autocompleteTableView];
		[self.autocompleteTableView setFirstRowAsSelected];
	}
	else if (event.keyCode == kVK_Escape)
	{
		// Hide autocomplete list
		[self.autocompleteTableContainer removeFromSuperview];
		self.autocompleteTableView.listVisible = NO;
	}
}

@end
