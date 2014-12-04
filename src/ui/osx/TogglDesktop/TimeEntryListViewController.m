//
//  TimeEntryListViewController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "TimeEntryListViewController.h"
#import "TimeEntryViewItem.h"
#import "TimerEditViewController.h"
#import "UIEvents.h"
#import "toggl_api.h"
#import "TimeEntryCell.h"
#import "TimeEntryCellWithHeader.h"
#import "UIEvents.h"
#import "DisplayCommand.h"
#import "TimeEntryEditViewController.h"
#import "ConvertHexColor.h"

@interface TimeEntryListViewController ()
@property (nonatomic, strong) IBOutlet TimerEditViewController *timerEditViewController;
@property NSNib *nibTimeEntryCell;
@property NSNib *nibTimeEntryCellWithHeader;
@property NSNib *nibTimeEntryEditViewController;
@property NSInteger defaultPopupHeight;
@property NSInteger defaultPopupWidth;
@property NSInteger addedHeight;
@property NSInteger minimumEditFormWidth;
@property (nonatomic, strong) IBOutlet TimeEntryEditViewController *timeEntryEditViewController;
@end

@implementation TimeEntryListViewController

extern void *ctx;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
	self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
	if (self)
	{
		self.timerEditViewController = [[TimerEditViewController alloc]
										initWithNibName:@"TimerEditViewController" bundle:nil];
		self.timeEntryEditViewController = [[TimeEntryEditViewController alloc]
											initWithNibName:@"TimeEntryEditViewController" bundle:nil];
		[self.timerEditViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		[self.timeEntryEditViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

		viewitems = [NSMutableArray array];

		self.nibTimeEntryCell = [[NSNib alloc] initWithNibNamed:@"TimeEntryCell"
														 bundle:nil];
		self.nibTimeEntryCellWithHeader = [[NSNib alloc] initWithNibNamed:@"TimeEntryCellWithHeader"
																   bundle:nil];
		self.nibTimeEntryEditViewController = [[NSNib alloc] initWithNibNamed:@"TimeEntryEditViewController"
																	   bundle:nil];

		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimeEntryList:)
													 name:kDisplayTimeEntryList
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimeEntryEditor:)
													 name:kDisplayTimeEntryEditor
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayLogin:)
													 name:kDisplayLogin
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(closeEditPopup:)
													 name:kForceCloseEditPopover
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(resizeEditPopupHeight:)
													 name:kResizeEditForm
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(resizeEditPopupWidth:)
													 name:kResizeEditFormWidth
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(resetEditPopover:)
													 name:NSPopoverDidCloseNotification
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(closeRunningEditPopup:)
													 name:kCommandStop
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(resetEditPopoverSize:)
													 name:kResetEditPopoverSize
												   object:nil];
	}
	return self;
}

- (void)loadView
{
	[super loadView];
	[self.timeEntriesTableView registerNib:self.nibTimeEntryCell
							 forIdentifier:@"TimeEntryCell"];
	[self.timeEntriesTableView registerNib:self.nibTimeEntryCellWithHeader
							 forIdentifier:@"TimeEntryCellWithHeader"];

	[self.headerView addSubview:self.timerEditViewController.view];
	[self.timerEditViewController.view setFrame:self.headerView.bounds];

	[self.timeEntryPopupEditView addSubview:self.timeEntryEditViewController.view];
	[self.timeEntryEditViewController.view setFrame:self.timeEntryPopupEditView.bounds];
	self.defaultPopupHeight = self.timeEntryPopupEditView.bounds.size.height;
	self.addedHeight = 0;
	self.minimumEditFormWidth = self.timeEntryPopupEditView.bounds.size.width;

	[self setupEmptyLabel];
}

- (void)setupEmptyLabel
{
	NSMutableParagraphStyle *paragrapStyle = NSMutableParagraphStyle.new;

	paragrapStyle.alignment                = kCTTextAlignmentCenter;

	NSMutableAttributedString *string = [[NSMutableAttributedString alloc] initWithString:@" reports"];

	[string setAttributes:
	 @{
		 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
		 NSForegroundColorAttributeName:[NSColor alternateSelectedControlColor]
	 }
					range:NSMakeRange(0, [string length])];
	NSMutableAttributedString *text = [[NSMutableAttributedString alloc] initWithString:@"Welcome back! Your previous entries are available in the web under" attributes:
									   @{ NSParagraphStyleAttributeName:paragrapStyle }];
	[text appendAttributedString:string];
	[self.emptyLabel setAttributedStringValue:text];
	[self.emptyLabel setAlignment:NSCenterTextAlignment];
}

- (void)startDisplayTimeEntryList:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayTimeEntryList:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayTimeEntryList:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSLog(@"TimeEntryListViewController displayTimeEntryList, thread %@", [NSThread currentThread]);

	@synchronized(viewitems)
	{
		[viewitems removeAllObjects];
		[viewitems addObjectsFromArray:cmd.timeEntries];
	}

	[self.timeEntriesTableView reloadData];
	if (cmd.open && self.timeEntrypopover.shown)
	{
		[self.timeEntrypopover close];
		[self setDefaultPopupSize];
	}

	BOOL hasItems = self.timeEntriesTableView.numberOfRows > 0;
	[self.timeEntryListScrollView setHidden:!hasItems];
	[self.emptyLabel setEnabled:!hasItems];
}

- (void)resetEditPopover:(NSNotification *)notification
{
	if (notification.object == self.timeEntrypopover)
	{
		[[NSNotificationCenter defaultCenter] postNotificationName:kResetEditPopover
															object:nil
														  userInfo:nil];
	}
}

- (void)popoverWillClose:(NSNotification *)notification
{
	NSLog(@"%@", notification.userInfo);
}

- (void)displayTimeEntryEditor:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSLog(@"TimeEntryListViewController displayTimeEntryEditor, thread %@", [NSThread currentThread]);
	if (cmd.open)
	{
		self.timeEntrypopover.contentViewController = self.timeEntrypopoverViewController;
		NSRect positionRect = [self.view bounds];

		[self.timeEntrypopover showRelativeToRect:positionRect
										   ofView:self.view
									preferredEdge:NSMaxXEdge];
	}
}

- (void)startDisplayTimeEntryEditor:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayTimeEntryEditor:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (int)numberOfRowsInTableView:(NSTableView *)tv
{
	int result = 0;

	@synchronized(viewitems)
	{
		result = (int)[viewitems count];
	}
	return result;
}

- (NSView *) tableView:(NSTableView *)tableView
	viewForTableColumn:(NSTableColumn *)tableColumn
				   row:(NSInteger)row
{
	TimeEntryViewItem *item = nil;

	@synchronized(viewitems)
	{
		item = [viewitems objectAtIndex:row];
	}
	NSAssert(item != nil, @"view item from viewitems array is nil");

	if (item.isHeader)
	{
		TimeEntryCellWithHeader *cell = [tableView makeViewWithIdentifier:@"TimeEntryCellWithHeader"
																	owner:self];
		[cell render:item];
		return cell;
	}

	TimeEntryCell *cell = [tableView makeViewWithIdentifier:@"TimeEntryCell"
													  owner:self];
	[cell render:item];
	return cell;
}

- (CGFloat)tableView:(NSTableView *)tableView
		 heightOfRow:(NSInteger)row
{
	TimeEntryViewItem *item = nil;

	@synchronized(viewitems)
	{
		if (row < viewitems.count)
		{
			item = viewitems[row];
		}
	}
	if (item && item.isHeader)
	{
		return 102;
	}
	return 56;
}

- (IBAction)performClick:(id)sender
{
	NSInteger row = [self.timeEntriesTableView clickedRow];

	if (row < 0)
	{
		return;
	}

	TimeEntryViewItem *item = 0;
	@synchronized(viewitems)
	{
		item = viewitems[row];
	}

	NSView *latestView = [self.timeEntriesTableView rowViewAtRow:row
												 makeIfNecessary:NO];

	for (NSView *subview in [latestView subviews])
	{
		if ([subview isKindOfClass:[TimeEntryCell class]])
		{
			[(TimeEntryCell *)subview focusFieldName];
		}
		else if ([subview isKindOfClass:[TimeEntryCellWithHeader class]])
		{
			[(TimeEntryCellWithHeader *)subview focusFieldName];
		}
	}
}

- (void)resetEditPopoverSize:(NSNotification *)notification
{
	[[NSNotificationCenter defaultCenter] postNotificationName:kResetEditPopover
														object:nil
													  userInfo:nil];
	[self setDefaultPopupSize];
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
	NSInteger selectedRow = [self.timeEntriesTableView selectedRow];

	if (selectedRow < 0)
	{
		return;
	}
	NSTableRowView *rowView = [self.timeEntriesTableView rowViewAtRow:selectedRow
													  makeIfNecessary:NO];
	[rowView setEmphasized:NO];
	[rowView setSelected:NO];
}

- (void)resizing:(NSSize)n
{
	[self.timeEntrypopover setContentSize:n];
	NSRect r = NSMakeRect(self.timeEntryEditViewController.view.frame.origin.x,
						  self.timeEntryEditViewController.view.frame.origin.y,
						  n.width,
						  n.height);

	[self.timeEntryPopupEditView setBounds:r];
	[self.timeEntryEditViewController.view setFrame:self.timeEntryPopupEditView.bounds];
}

- (void)resizeEditPopupHeight:(NSNotification *)notification
{
	if (!self.timeEntrypopover.shown)
	{
		return;
	}
	NSInteger addHeight = [[[notification userInfo] valueForKey:@"height"] intValue];
	if (addHeight == self.addedHeight)
	{
		return;
	}
	self.addedHeight = addHeight;
	float newHeight = self.timeEntrypopover.contentSize.height + self.addedHeight;
	NSSize n = NSMakeSize(self.timeEntrypopover.contentSize.width, newHeight);

	[self resizing:n];
}

- (void)resizeEditPopupWidth:(NSNotification *)notification
{
	if (!self.timeEntrypopover.shown)
	{
		return;
	}
	int i = [[[notification userInfo] valueForKey:@"width"] intValue];
	float newWidth = self.timeEntrypopover.contentSize.width + i;

	if (newWidth < self.minimumEditFormWidth)
	{
		return;
	}
	NSSize n = NSMakeSize(newWidth, self.timeEntrypopover.contentSize.height);

	[self resizing:n];
}

- (void)closeEditPopup:(NSNotification *)notification
{
	if (self.timeEntrypopover.shown)
	{
		[self.timeEntrypopover close];
		[self setDefaultPopupSize];
	}
}

- (void)closeRunningEditPopup:(NSNotification *)notification
{
	[self closeEditPopup:notification];
}

- (void)setDefaultPopupSize
{
	if (self.addedHeight != 0)
	{
		NSSize n = NSMakeSize(self.timeEntrypopover.contentSize.width, self.defaultPopupHeight);

		[self resizing:n];
		self.addedHeight = 0;
	}
}

- (void)startDisplayLogin:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayLogin:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayLogin:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	if (cmd.open && self.timeEntrypopover.shown)
	{
		[self.timeEntrypopover close];
		[self setDefaultPopupSize];
	}
}

- (void)textFieldClicked:(id)sender
{
	if (sender == self.emptyLabel && [self.emptyLabel isEnabled])
	{
		toggl_open_in_browser(ctx);
	}
}

@end
