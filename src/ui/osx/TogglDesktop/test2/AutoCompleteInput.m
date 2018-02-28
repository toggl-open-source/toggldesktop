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
		self.posY = 0;
		self.itemHeight = 35;
		self.maxVisibleItems = 6;
		[self createAutocomplete];
	}
	return self;
}

- (void)createAutocomplete
{
	self.autocompleteTableContainer = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
	self.nibAutoCompleteTableCell = [[NSNib alloc] initWithNibNamed:@"AutoCompleteTableCell" bundle:nil];
	self.autocompleteTableView = [[AutoCompleteTable alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
	[self.autocompleteTableView registerNib:self.nibAutoCompleteTableCell
							  forIdentifier :@"AutoCompleteTableCell"];

	[self.autocompleteTableView setDelegate:self];

	[self.autocompleteTableContainer setDocumentView:self.autocompleteTableView];
	[self.autocompleteTableContainer setAutohidesScrollers:YES];
	[self.autocompleteTableContainer setHasVerticalScroller:YES];
	[self.autocompleteTableContainer setHidden:YES];

	self.autocompleteTableContainer.translatesAutoresizingMaskIntoConstraints = NO;
}

- (void)setupAutocompleteConstraints
{
	// Set constraints to input field so autocomplete size is always connected to input
	NSLayoutConstraint *leftConstraint = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1 constant:0];

	NSLayoutConstraint *rightConstraint =  [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1 constant:0];

	NSLayoutConstraint *topConstraint = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1 constant:1];

	self.heightConstraint = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1 constant:self.itemHeight];

	[NSLayoutConstraint activateConstraints:[NSArray arrayWithObjects:leftConstraint, rightConstraint, self.heightConstraint, topConstraint, nil]];
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
		if (self.autocompleteTableContainer.hidden)
		{
			if (self.heightConstraint == nil)
			{
				[self.window.contentView addSubview:self.autocompleteTableContainer positioned:NSWindowAbove relativeTo:nil];
				[self setupAutocompleteConstraints];
			}
			[self.autocompleteTableContainer setHidden:NO];
		}
		[self updateDropdownHeight:itemCount];
	}
	else if (self.autocompleteTableContainer != nil)
	{
		[self.autocompleteTableContainer setHidden:YES];
	}
	self.lastItemCount = itemCount;
}

- (void)updateDropdownHeight:(int)count
{
	int h = MIN((count * self.itemHeight), self.posY - 100);

	self.heightConstraint.constant = h;
	NSLog(@"Update table position | H: %d, POSY: %d", h, self.posY);
}

- (void)keyUp:(NSEvent *)event
{
	NSLog(@"EventCode: %hu", [event keyCode]);
	if ([event keyCode] == kVK_DownArrow)
	{
		if (self.autocompleteTableContainer.hidden)
		{
			[self toggleTableView:(int)self.autocompleteTableView.numberOfRows];
		}
		[self.autocompleteTableView nextItem];
	}
	else if (event.keyCode == kVK_UpArrow)
	{
		[self.autocompleteTableView previousItem];
	}
	else if (event.keyCode == kVK_Escape)
	{
		// Hide autocomplete list
		if (self.autocompleteTableContainer != nil)
		{
			[self.autocompleteTableContainer setHidden:YES];
		}
	}
}

@end
