//
//  TimeEntryListViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "TimeEntryListViewController.h"
#import "TimeEntryViewItem.h"
#import "TimerEditViewController.h"
#import "UIEvents.h"
#import "kopsik_api.h"
#import "TimeEntryCell.h"
#import "TimeEntryCellWithHeader.h"
#import "UIEvents.h"
#import "DisplayCommand.h"
#import "TimeEntryEditViewController.h"

@interface TimeEntryListViewController ()
@property (nonatomic, strong) IBOutlet TimerEditViewController *timerEditViewController;
@property NSNib *nibTimeEntryCell;
@property NSNib *nibTimeEntryCellWithHeader;
@property NSNib *nibTimeEntryEditViewController;
@property NSView *selectedRowView;
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
	}
}

- (void)displayTimeEntryEditor:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSLog(@"TimeEntryListViewController displayTimeEntryEditor, thread %@", [NSThread currentThread]);
	if (cmd.open)
	{
		if (cmd.timeEntry.duration_in_seconds < 0)
		{
			self.selectedRowView = self.headerView;
		}
		[self.timeEntrypopover showRelativeToRect:[[self selectedRowView] bounds]
										   ofView:[self selectedRowView]
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
	self.selectedRowView = [self.timeEntriesTableView rowViewAtRow:row
												   makeIfNecessary:NO];

	kopsik_edit(ctx, [item.GUID UTF8String], false, "");
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

@end
