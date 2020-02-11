//
//  AutoComleteInput.m
//  LiteComplete
//
//  Created by Indrek Vändrik on 20/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import "AutoCompleteInput.h"
#import "TogglDesktop-Swift.h"
#import "AutoCompleteTable.h"
#import "AutoCompleteTableCell.h"
#import "AutoCompleteTableContainer.h"
#import <Carbon/Carbon.h>

static NSString *const downArrow = @"\u25BC";
static NSString *const upArrow = @"\u25B2";

@interface AutoCompleteInput () <NoInteractionViewDelegate, NSTableViewDelegate, NSTableViewDataSource, NSTextFieldDelegate>
@property (strong, nonatomic) AutoCompleteTable *autocompleteTableView;
@property (strong, nonatomic) AutoCompleteTableContainer *autocompleteTableContainer;
@property (assign, nonatomic) BOOL constraintsActive;
@property (strong, nonatomic) NoInteractionView *backgroundView;
@property (strong, nonatomic) NSLayoutConstraint *heightConstraint;
@property (assign, nonatomic) NSInteger posY;
@property (strong, nonatomic) NSButton *actionButton;
@property (assign, nonatomic) CGFloat totalHeight;
@property (assign, nonatomic) CGFloat itemHeight;
@property (assign, nonatomic) CGFloat worksapceItemHeight;
@end

@implementation AutoCompleteInput

- (id)initWithCoder:(NSCoder *)coder
{
	self = [super initWithCoder:coder];
	if (self)
	{
		self.posY = 0;
		self.constraintsActive = NO;
		self.itemHeight = 30.0;
		self.worksapceItemHeight = 40.0;
		[self createAutocomplete];
		self.wantsLayer = YES;
		self.layer.masksToBounds = NO;
		self.displayMode = AutoCompleteDisplayModeCompact;
		[self initBackgroundView];
        if (@available(macOS 10.12.2, *))
        {
            self.automaticTextCompletionEnabled = NO;
        }
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(escTouchBarButtonOnClickNoti:) name:kEscTouchBarButtonOnClickNotification object:nil];
	}
	return self;
}

-(void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)initBackgroundView
{
	self.backgroundView = [[NoInteractionView alloc] initWithFrame:CGRectZero];
	self.backgroundView.delegate = self;
	self.backgroundView.translatesAutoresizingMaskIntoConstraints = NO;
	self.backgroundView.wantsLayer = YES;
	self.backgroundView.layer.backgroundColor = [NSColor colorWithWhite:0 alpha:0.5f].CGColor;
}

- (void)createAutocomplete
{
	self.autocompleteTableContainer = [[AutoCompleteTableContainer alloc] initWithFrame:CGRectZero];
	self.autocompleteTableView = [[AutoCompleteTable alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
	[self.autocompleteTableView registerNib:[[NSNib alloc] initWithNibNamed:@"AutoCompleteTableCell" bundle:nil]
							  forIdentifier :@"AutoCompleteTableCell"];

	[self.autocompleteTableView setDelegate:self];

	[self.autocompleteTableContainer setDocumentView:self.autocompleteTableView];
	[self.autocompleteTableContainer setAutohidesScrollers:YES];
	[self.autocompleteTableContainer setHasVerticalScroller:YES];
	[self.autocompleteTableContainer setHidden:YES];

	self.autocompleteTableContainer.translatesAutoresizingMaskIntoConstraints = NO;
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
		NSLayoutConstraint *top = [NSLayoutConstraint constraintWithItem:self.backgroundView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1 constant:[self topPaddingForContainer]];
		NSLayoutConstraint *bottom = [NSLayoutConstraint constraintWithItem:self.backgroundView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self.window.contentView attribute:NSLayoutAttributeBottom multiplier:1 constant:0];

		[self.window.contentView addConstraints:@[left, right, top, bottom]];
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

	NSLayoutConstraint *right =  [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:view attribute:NSLayoutAttributeRight multiplier:1 constant:0];

	NSLayoutConstraint *top = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1 constant:[self topPaddingForContainer]];

	self.heightConstraint = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1 constant:0];
	self.heightConstraint.priority = NSLayoutPriorityDefaultHigh;

	NSLayoutConstraint *bottom = [NSLayoutConstraint constraintWithItem:self.autocompleteTableContainer attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationGreaterThanOrEqual toItem:view attribute:NSLayoutAttributeBottom multiplier:1 constant:0];
	bottom.priority = NSLayoutPriorityDefaultLow;

	[self.window.contentView addConstraints:@[left, right, top, self.heightConstraint, bottom]];
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
	CGFloat suitableHeight;

	switch (self.displayMode)
	{
		case AutoCompleteDisplayModeCompact :
			suitableHeight = MIN(height, self.posY - 50);
			break;
		case AutoCompleteDisplayModeFullscreen :
			suitableHeight = MIN(height, self.posY - 25);
			break;
		default :
			break;
	}

	self.heightConstraint.constant = suitableHeight;
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
		[[self currentEditor] setSelectedRange:NSMakeRange(0, 0)];
		[[self currentEditor] moveToEndOfLine:nil];
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
			return 18;
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
	[self toggleTableViewWithNumberOfItem:array.count];
	[self updateDropdownWithHeight:totalHeight];
}

- (void) escTouchBarButtonOnClickNoti:(NSNotification *) noti
{
    [self resetTable];
}
@end
