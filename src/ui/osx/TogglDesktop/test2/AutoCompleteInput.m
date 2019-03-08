//
//  AutoComleteInput.m
//  LiteComplete
//
//  Created by Indrek Vändrik on 20/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import "AutoCompleteInput.h"

@interface AutoCompleteInput ()

@property (assign, nonatomic) BOOL constraintsActive;

@end

@implementation AutoCompleteInput

NSString *downArrow = @"\u25BC";
NSString *upArrow = @"\u25B2";

- (id)initWithCoder:(NSCoder *)coder
{
	self = [super initWithCoder:coder];
	if (self)
	{
		self.posY = 0;
		self.constraintsActive = NO;
		[self createAutocomplete];
		self.wantsLayer = YES;
		self.layer.masksToBounds = NO;
		self.expandToMainWindow = NO;
	}
	return self;
}

- (void)createAutocomplete
{
	self.autocompleteTableContainer = [[AutoCompleteTableContainer alloc] initWithFrame:CGRectZero];
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
	NSView *view = self;

	if (self.expandToMainWindow)
	{
		view = self.window.contentView;
	}

	// Set constraints to input field so autocomplete size is always connected to input
	self.leftConstraint = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:view attribute:NSLayoutAttributeLeft multiplier:1 constant:0];

	self.rightConstraint =  [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:view attribute:NSLayoutAttributeRight multiplier:1 constant:0];

	self.topConstraint = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1 constant:18];

	self.heightConstraint = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1 constant:0];
}

- (void)setPos:(int)posy
{
	self.posY = posy;
}

- (void)toggleTableViewWithNumberOfItem:(NSInteger)numberOfItem
{
	if (numberOfItem > 0)
	{
		if (self.autocompleteTableContainer.hidden)
		{
			if (self.autocompleteTableContainer.superview == nil)
			{
				[self.window.contentView addSubview:self.autocompleteTableContainer positioned:NSWindowAbove relativeTo:nil];
				[self setupAutocompleteConstraints];
			}
			[self showAutoComplete:YES];
		}
	}
	else if (self.autocompleteTableContainer != nil)
	{
		[self showAutoComplete:NO];
	}
}

- (void)updateDropdownWithHeight:(CGFloat)height
{
	self.heightConstraint.constant = height;
}

- (void)keyUp:(NSEvent *)event
{
	// NSLog(@"EventCode: %hu", [event keyCode]);
	if ([event keyCode] == kVK_DownArrow)
	{
		if ([event modifierFlags] & NSShiftKeyMask)
		{
			[super keyUp:event];
			return;
		}
		if (self.autocompleteTableContainer.isHidden)
		{
			[self toggleTableViewWithNumberOfItem:self.autocompleteTableView.numberOfRows];
			return;
		}
	}
	else if (event.keyCode == kVK_Escape)
	{
		// Hide autocomplete list
		if (self.autocompleteTableContainer != nil)
		{
			[self resetTable];
			return;
		}
	}
	else if ((event.keyCode == kVK_Return) || (event.keyCode == kVK_ANSI_KeypadEnter))
	{
		if (!self.autocompleteTableView.isHidden)
		{
			[self showAutoComplete:NO];
			return;
		}
	}
	[super keyUp:event];
}

- (void)hide
{
	[self showAutoComplete:NO];
	[self setHidden:YES];
}

- (void)resetTable
{
	[self showAutoComplete:NO];
	[self.autocompleteTableView resetSelected];
}

- (void)showAutoComplete:(BOOL)show
{
	if (show)
	{
		if (!self.constraintsActive)
		{
			[self.window.contentView addConstraints:[NSArray arrayWithObjects:self.leftConstraint, self.rightConstraint, self.heightConstraint, self.topConstraint, nil]];
			self.constraintsActive = YES;
		}
		[[self currentEditor] setSelectedRange:NSMakeRange(0, 0)];
		[[self currentEditor] moveToEndOfLine:nil];
		if (self.actionButton != nil)
		{
			[self.actionButton setTitle:upArrow];
		}
	}
	else
	{
		if (self.constraintsActive)
		{
			[self.window.contentView removeConstraints:[NSArray arrayWithObjects:self.leftConstraint, self.rightConstraint, self.heightConstraint, self.topConstraint, nil]];
			self.constraintsActive = NO;
		}
		if (self.actionButton != nil)
		{
			[self.actionButton setTitle:downArrow];
		}
	}
	[self.autocompleteTableContainer setHidden:!show];
	[self.autocompleteTableView setHidden:!show];
}

- (BOOL)becomeFirstResponder
{
	BOOL success = [super becomeFirstResponder];

	if (success && self.isEditable)
	{
		NSTextView *textField = (NSTextView *)[self currentEditor];
		if ([textField respondsToSelector:@selector(setInsertionPointColor:)])
		{
			[textField setInsertionPointColor:[self textColor]];
		}
	}
	return success;
}

- (void)setButton:(NSButton *)button
{
	self.actionButton = button;
	[self.actionButton setTitle:downArrow];
}

@end
