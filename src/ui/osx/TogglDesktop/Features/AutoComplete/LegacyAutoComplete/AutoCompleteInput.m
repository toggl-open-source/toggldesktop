//
//  AutoComleteInput.m
//  LiteComplete
//
//  Created by Indrek Vändrik on 20/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import "AutoCompleteInput.h"
#import "Toggl_Track-Swift.h"
#import "AutoCompleteTable.h"
#import "AutoCompleteTableCell.h"
#import "AutoCompleteTableContainer.h"
#import <Carbon/Carbon.h>

static NSString *const downArrow = @"\u25BC";
static NSString *const upArrow = @"\u25B2";
static CGFloat maxDropdownWidth = 500;
static CGFloat dropdownBottomPadding = 50;
static CGFloat dropdownHorizontalPadding = 11;

@interface AutoCompleteInput () <NoInteractionViewDelegate, NSTableViewDelegate, NSTableViewDataSource, NSTextFieldDelegate, NSTextViewDelegate>
@property (strong, nonatomic) AutoCompleteTable *autocompleteTableView;
@property (strong, nonatomic) AutoCompleteTableContainer *autocompleteTableContainer;
@property (assign, nonatomic) BOOL constraintsActive;
@property (strong, nonatomic) NoInteractionView *backgroundView;
@property (strong, nonatomic) NSLayoutConstraint *dropdownHeightConstraint;
@property (strong, nonatomic) NSLayoutConstraint *dropdownWidthConstraint;
@property (strong, nonatomic) NSLayoutConstraint *dropdownTopConstraint;
@property (strong, nonatomic) NSLayoutConstraint *backgroundTopConstraint;
@property (strong, nonatomic) NSButton *actionButton;
@property (assign, nonatomic) CGFloat totalHeight;
@property (assign, nonatomic) CGFloat itemHeight;
@property (assign, nonatomic) CGFloat worksapceItemHeight;
@property (nonatomic, readwrite) BOOL isListHidden;
@property (nonatomic, readwrite) NSRange selectedRange;
@end

@implementation AutoCompleteInput

- (id)initWithCoder:(NSCoder *)coder
{
	self = [super initWithCoder:coder];
	if (self)
	{
		self.constraintsActive = NO;
		self.itemHeight = 30.0;
		self.worksapceItemHeight = 40.0;
		[self createAutocomplete];
		self.wantsLayer = YES;
		self.layer.masksToBounds = NO;
		self.displayMode = AutoCompleteDisplayModeCompact;
		self.selectedRange = NSMakeRange(0, 0);
		[self initBackgroundView];
        if (@available(macOS 10.12.2, *))
        {
            self.automaticTextCompletionEnabled = NO;
        }
	}
	return self;
}

- (void)initBackgroundView
{
	self.backgroundView = [[NoInteractionView alloc] initWithFrame:CGRectZero];
	self.backgroundView.delegate = self;
	self.backgroundView.translatesAutoresizingMaskIntoConstraints = NO;
	self.backgroundView.wantsLayer = YES;
	self.backgroundView.layer.backgroundColor = [NSColor colorWithWhite:0 alpha:0.5f].CGColor;
	self.autocompleteTableView.backgroundColor = [NSColor autoCompleteBackground];
}

- (void)layout
{
	[super layout];
	self.dropdownWidthConstraint.constant = [self dropdownWidth];
	[self updateDropdownWithHeight:self.totalHeight];
}

- (void)createAutocomplete
{
	self.autocompleteTableContainer = [[AutoCompleteTableContainer alloc] initWithFrame:CGRectZero];
	self.autocompleteTableView = [[AutoCompleteTable alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
	[self.autocompleteTableView registerNib:[[NSNib alloc] initWithNibNamed:@"AutoCompleteTableCell" bundle:nil]
							  forIdentifier :@"AutoCompleteTableCell"];

	[self.autocompleteTableView setDelegate:self];
	[self.autocompleteTableView setRefusesFirstResponder:YES];

	[self.autocompleteTableContainer setDocumentView:self.autocompleteTableView];
	[self.autocompleteTableContainer setAutohidesScrollers:YES];
	[self.autocompleteTableContainer setHasVerticalScroller:YES];
	[self.autocompleteTableContainer setHidden:YES];
	self.isListHidden = YES;

	self.autocompleteTableContainer.translatesAutoresizingMaskIntoConstraints = NO;
}

- (void)updateConstraints
{
	// re-activating constraints that may have been deactivated when
	// autocomplete view was removed from superview (e.g. during logout)
	if (!self.dropdownTopConstraint.isActive) {
		self.dropdownTopConstraint.active = YES;
	}
	if (!self.backgroundTopConstraint.isActive) {
		self.backgroundTopConstraint.active = YES;
	}

	[super updateConstraints];
}

- (void)addBackgroundViewIfNeed
{
	if (self.backgroundView.superview != nil)
	{
		return;
	}

	if (self.displayMode == AutoCompleteDisplayModeFullscreen)
	{
		[self.window.contentView addSubview:self.backgroundView positioned:NSWindowBelow relativeTo:self.autocompleteTableContainer];

		NSLayoutConstraint *left = [NSLayoutConstraint constraintWithItem:self.backgroundView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self.window.contentView attribute:NSLayoutAttributeLeft multiplier:1 constant:0];
		NSLayoutConstraint *right = [NSLayoutConstraint constraintWithItem:self.backgroundView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self.window.contentView attribute:NSLayoutAttributeRight multiplier:1 constant:0];
		self.backgroundTopConstraint = [NSLayoutConstraint constraintWithItem:self.backgroundView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1 constant:[self topPaddingForContainer]];
		NSLayoutConstraint *bottom = [NSLayoutConstraint constraintWithItem:self.backgroundView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self.window.contentView attribute:NSLayoutAttributeBottom multiplier:1 constant:0];

		[self.window.contentView addConstraints:@[left, right, self.backgroundTopConstraint, bottom]];
	}
}

- (void)addAutocompleteContainerIfNeed
{
	if (self.autocompleteTableContainer.superview != nil)
	{
		return;
	}

	// Add view
	[self.window.contentView addSubview:self.autocompleteTableContainer positioned:NSWindowAbove relativeTo:nil];

	// Get view for leading/trailing
    NSView *view;
	switch (self.displayMode)
	{
		case AutoCompleteDisplayModeCompact :
			view = self;
			break;
		case AutoCompleteDisplayModeFullscreen :
			view = self.window.contentView;
		default :
            view = self;
	}

	// Set constraints to input field so autocomplete size is always connected to input
	NSLayoutConstraint *left = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:view attribute:NSLayoutAttributeLeft multiplier:1 constant:0];

	self.dropdownTopConstraint = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1 constant:[self topPaddingForContainer]];

	self.dropdownHeightConstraint = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1 constant:0];
	self.dropdownHeightConstraint.priority = NSLayoutPriorityDefaultHigh;

	NSLayoutConstraint *bottom = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationGreaterThanOrEqual toItem:view attribute:NSLayoutAttributeBottom multiplier:1 constant:0];
	bottom.priority = NSLayoutPriorityDefaultLow;

	self.dropdownWidthConstraint = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeWidth multiplier:1 constant:[self dropdownWidth]];
	self.dropdownWidthConstraint.priority = NSLayoutPriorityRequired;

	[self.window.contentView addConstraints:@[left, self.dropdownTopConstraint, self.dropdownHeightConstraint, bottom, self.dropdownWidthConstraint]];
}

- (CGFloat)dropdownWidth
{
	CGFloat windowWidth = self.window.contentView.frame.size.width;
	CGFloat dropdownWidth = windowWidth - dropdownHorizontalPadding * 2;
	if (dropdownWidth > maxDropdownWidth)
	{
		return maxDropdownWidth;
	}
	else
	{
		return dropdownWidth;
	}
}

- (void)toggleList:(BOOL)isOn
{
	if (isOn)
	{
		if (self.autocompleteTableContainer.hidden && self.totalHeight > 0)
		{
			[self addAutocompleteContainerIfNeed];
			[self addBackgroundViewIfNeed];
			[self updateDropdownWithHeight:self.totalHeight];
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
	self.totalHeight = height;
	CGFloat maxAllowedHeight = [self convertPoint:CGPointZero toView:self.window.contentView].y - dropdownBottomPadding;
	self.dropdownHeightConstraint.constant = MIN(height, maxAllowedHeight);
}

- (void)hide
{
	[self showAutoComplete:NO];
	[self setHidden:YES];
}

- (void)resetTable
{
	[self.inputDelegate autoCompleteInputShouldResetFilter];
	[self showAutoComplete:NO];
	[self.autocompleteTableView resetSelected];
}

- (void)showAutoComplete:(BOOL)show
{
	if (show)
	{
		self.autocompleteTableContainer.hidden = NO;
		self.backgroundView.hidden = NO;
		if (self.actionButton != nil)
		{
			[self.actionButton setTitle:upArrow];
		}
	}
	else
	{
		self.autocompleteTableContainer.hidden = YES;
		self.backgroundView.hidden = YES;
		if (self.actionButton != nil)
		{
			[self.actionButton setTitle:downArrow];
		}
	}
	[self.autocompleteTableContainer setHidden:!show];
	[self.autocompleteTableView setHidden:!show];
	self.isListHidden = !show;
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

- (CGFloat)topPaddingForContainer
{
	switch (self.displayMode)
	{
		case AutoCompleteDisplayModeCompact :
			return 0;

		case AutoCompleteDisplayModeFullscreen :
			return 10;
	}
}

- (void)noInteractionViewMouseDidDown:(NoInteractionView *)sender
{
	[self resetTable];
}

- (CGFloat)calculateTotalHeightFromArray:(NSArray<AutocompleteItem *> *)array
{
	CGFloat height = 0;

	for (AutocompleteItem *item in array)
	{
		AutoCompleteCellType cellType = [AutoCompleteTableCell cellTypeFrom:item];
		if (cellType == AutoCompleteCellTypeWorkspace)
		{
			height += self.worksapceItemHeight;
		}
		else
		{
			height += self.itemHeight;
		}
	}
	return height;
}

- (void)reloadAutocomplete:(NSArray<AutocompleteItem *> *)array
{
	CGFloat totalHeight = [self calculateTotalHeightFromArray:array];

	[self.autocompleteTableView reloadData];
	[self updateDropdownWithHeight:totalHeight];
	[self toggleList:array.count > 0];
}

- (void)textViewDidChangeSelection:(NSNotification *)notification
{
	NSTextView *editor = (NSTextView *)self.currentEditor;
	if (!editor) {
		return;
	}
	self.selectedRange = [editor selectedRange];
}

@end
