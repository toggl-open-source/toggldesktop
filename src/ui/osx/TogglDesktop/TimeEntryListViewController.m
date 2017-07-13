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
#import "LoadMoreCell.h"
#import "TimeEntryCell.h"
#import "TimeEntryCellWithHeader.h"
#import "UIEvents.h"
#import "DisplayCommand.h"
#import "TimeEntryEditViewController.h"
#import "ConvertHexColor.h"
#include <Carbon/Carbon.h>

@interface TimeEntryListViewController ()
@property (nonatomic, strong) IBOutlet TimerEditViewController *timerEditViewController;
@property NSNib *nibTimeEntryCell;
@property NSNib *nibTimeEntryCellWithHeader;
@property NSNib *nibTimeEntryEditViewController;
@property NSNib *nibLoadMoreCell;
@property NSInteger defaultPopupHeight;
@property NSInteger defaultPopupWidth;
@property NSInteger addedHeight;
@property NSInteger minimumEditFormWidth;
@property NSInteger lastSelectedRowIndex;
@property BOOL runningEdit;
@property TimeEntryCell *selectedEntryCell;
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
		self.nibLoadMoreCell = [[NSNib alloc] initWithNibNamed:@"LoadMoreCell"
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
												 selector:@selector(closeEditPopup:)
													 name:kCommandStop
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(resetEditPopoverSize:)
													 name:kResetEditPopoverSize
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(focusListing:)
													 name:kFocusListing
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(escapeListing:)
													 name:kEscapeListing
												   object:nil];
	}
	return self;
}

- (void)loadView
{
	[super loadView];
	[self.timeEntriesTableView registerNib:self.nibTimeEntryCell
							 forIdentifier :@"TimeEntryCell"];
	[self.timeEntriesTableView registerNib:self.nibTimeEntryCellWithHeader
							 forIdentifier :@"TimeEntryCellWithHeader"];
	[self.timeEntriesTableView registerNib:self.nibLoadMoreCell
							 forIdentifier :@"LoadMoreCell"];

	[self.headerView addSubview:self.timerEditViewController.view];
	[self.timerEditViewController.view setFrame:self.headerView.bounds];

	[self.timeEntryPopupEditView addSubview:self.timeEntryEditViewController.view];
	[self.timeEntryEditViewController.view setFrame:self.timeEntryPopupEditView.bounds];
	self.defaultPopupHeight = self.timeEntryPopupEditView.bounds.size.height;
	self.addedHeight = 0;
	self.lastSelectedRowIndex = 0;
	self.minimumEditFormWidth = self.timeEntryPopupEditView.bounds.size.width;
	self.runningEdit = NO;

	[self setupEmptyLabel];

	// Drag and drop
	[self.timeEntriesTableView setDraggingSourceOperationMask:NSDragOperationLink forLocal:NO];
	[self.timeEntriesTableView setDraggingSourceOperationMask:NSDragOperationMove forLocal:YES];
	[self.timeEntriesTableView registerForDraggedTypes:[NSArray arrayWithObject:NSStringPboardType]];
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

	// Save the current scroll position of entries list
	NSPoint scrollOrigin;
	NSRect rowRect = [self.timeEntriesTableView rectOfRow:0];
	BOOL adjustScroll = !NSEqualRects(rowRect, NSZeroRect) && !NSContainsRect(self.timeEntriesTableView.visibleRect, rowRect);
	if (adjustScroll)
	{
		// get scroll position from the bottom: get bottom left of the visible part of the table view
		scrollOrigin = self.timeEntriesTableView.visibleRect.origin;
		if (self.timeEntriesTableView.isFlipped)
		{
			// scrollOrigin is top left, calculate unflipped coordinates
			scrollOrigin.y = self.timeEntriesTableView.bounds.size.height - scrollOrigin.y;
		}
	}
	long delta = (cmd.timeEntries.count + 1 - viewitems.count) * 56;
	@synchronized(viewitems)
	{
		[viewitems removeAllObjects];
		[viewitems addObjectsFromArray:cmd.timeEntries];

		// Add Load more button

		if (cmd.show_load_more)
		{
			TimeEntryViewItem *it = [TimeEntryViewItem alloc];
			[it setLoadMore:YES];
			[viewitems addObject:it];
		}
	}

	[self.timeEntriesTableView reloadData];
	if (cmd.open)
	{
		if (self.timeEntrypopover.shown)
		{
			[self.timeEntrypopover close];
			[self setDefaultPopupSize];
		}
		[self focusListing:nil];
	}

	BOOL noItems = self.timeEntriesTableView.numberOfRows == 0;
	[self.emptyLabel setEnabled:noItems];
	[self.timeEntryListScrollView setHidden:noItems];
	// This seems to work for hiding the list when there are no items
	if (noItems)
	{
		[self.timeEntryListScrollView setHidden:noItems];
	}

	// Restore scroll position after list reload
	if (adjustScroll)
	{
		// restore scroll position from the bottom
		if (self.timeEntriesTableView.isFlipped)
		{
			// calculate new flipped coordinates, height includes the new row
			scrollOrigin.y = self.timeEntriesTableView.bounds.size.height - scrollOrigin.y - delta;
		}
		[self.timeEntriesTableView scrollPoint:scrollOrigin];
	}
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
		self.runningEdit = (cmd.timeEntry.duration_in_seconds < 0);

		[self.timeEntrypopover showRelativeToRect:positionRect
										   ofView:self.view
									preferredEdge:NSMaxXEdge];
		BOOL onLeft = (self.view.window.frame.origin.x > self.timeEntryPopupEditView.window.frame.origin.x);
		[self.timeEntryEditViewController setDragHandle:onLeft];
		[self.timeEntryEditViewController setInsertionPointColor];
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

- (BOOL)  tableView:(NSTableView *)aTableView
	shouldSelectRow:(NSInteger)rowIndex
{
	[self clearLastSelectedEntry];
	self.lastSelectedRowIndex = rowIndex;
	TimeEntryCell *cell = [self getSelectedEntryCell:rowIndex];
	if (cell != nil)
	{
		[cell setFocused];
	}
	return YES;
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

	if (item.loadMore == YES)
	{
		LoadMoreCell *cell = [tableView makeViewWithIdentifier:@"LoadMoreCell"
														 owner:self];
		[cell initCell];
		return cell;
	}

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
	[self clearLastSelectedEntry];
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

	TimeEntryCell *cell = [self getSelectedEntryCell:row];

	// Group header clicked, toggle group open/closed
	if (cell.Group)
	{
		[[NSNotificationCenter defaultCenter] postNotificationName:kToggleGroup object:cell.GroupName];
		return;
	}

	// Load more cell clicked
	if ([cell isKindOfClass:[LoadMoreCell class]])
	{
		return;
	}

	if (cell != nil)
	{
		[cell focusFieldName];
	}
}

- (TimeEntryCell *)getSelectedEntryCell:(NSInteger)row
{
	NSView *latestView = [self.timeEntriesTableView rowViewAtRow:row
												 makeIfNecessary  :YES];

	if (latestView == nil)
	{
		return nil;
	}

	self.selectedEntryCell = nil;

	for (NSView *subview in [latestView subviews])
	{
		if ([subview isKindOfClass:[TimeEntryCell class]] || [subview isKindOfClass:[TimeEntryCellWithHeader class]])
		{
			self.selectedEntryCell = (TimeEntryCell *)subview;
			return self.selectedEntryCell;
		}
	}
	return nil;
}

- (void)clearLastSelectedEntry
{
	[self.selectedEntryCell setupGroupMode];
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
													  makeIfNecessary  :NO];
	[rowView setEmphasized:NO];
	[rowView setSelected:NO];
}

- (void)resizing:(NSSize)n
{
	[self.timeEntrypopover setContentSize:n];
	NSRect f = [self.timeEntryEditViewController.view frame];
	NSRect r = NSMakeRect(f.origin.x,
						  f.origin.y,
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
		if (self.runningEdit)
		{
			[self.timeEntryEditViewController closeEdit];
			self.runningEdit = false;
		}
		else
		{
			[self.selectedEntryCell openEdit];
		}

		[self setDefaultPopupSize];
	}
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

- (void)focusListing:(NSNotification *)notification
{
	if (self.timeEntriesTableView.numberOfRows == 0)
	{
		return;
	}

	// If list is focused with keyboard shortcut
	if (notification != nil && !self.timeEntrypopover.shown)
	{
		[self clearLastSelectedEntry];
		self.lastSelectedRowIndex = 0;
	}

	if ([self.timeEntriesTableView numberOfRows] < self.lastSelectedRowIndex)
	{
		return;
	}

	NSIndexSet *indexSet = [NSIndexSet indexSetWithIndex:self.lastSelectedRowIndex];

	[[self.timeEntriesTableView window] makeFirstResponder:self.timeEntriesTableView];
	[self.timeEntriesTableView selectRowIndexes:indexSet byExtendingSelection:NO];

	TimeEntryCell *cell = [self getSelectedEntryCell:self.lastSelectedRowIndex];
	if (cell != nil)
	{
		[self clearLastSelectedEntry];
		[cell setFocused];
		[self.timeEntriesTableView scrollRowToVisible:self.lastSelectedRowIndex];
	}
}

- (void)escapeListing:(NSNotification *)notification
{
	if (self.timeEntrypopover.shown)
	{
		[self closeEditPopup:nil];
		return;
	}
	[[NSNotificationCenter defaultCenter] postNotificationName:kFocusTimer
														object:nil
													  userInfo:nil];
	[self clearLastSelectedEntry];
	self.selectedEntryCell = nil;
}

#pragma mark Drag & Drop Delegates

- (BOOL)       tableView:(NSTableView *)aTableView
	writeRowsWithIndexes:(NSIndexSet *)rowIndexes
			toPasteboard:(NSPasteboard *)pboard
{
	if (aTableView == self.timeEntriesTableView)
	{
		// Disable drag and drop for load more and group row
		TimeEntryViewItem *model = [viewitems objectAtIndex:[rowIndexes firstIndex]];
		if ([model loadMore] || model.Group)
		{
			return NO;
		}
		NSData *data = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes];
		[pboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:self];
		[pboard setData:data forType:NSStringPboardType];
		return YES;
	}
	else
	{
		return NO;
	}
}

- (NSDragOperation)tableView:(NSTableView *)tv
				validateDrop:(id )info
				 proposedRow:(NSInteger)row
	   proposedDropOperation:(NSTableViewDropOperation)op
{
	return NSDragOperationMove;
}

- (BOOL)tableView:(NSTableView *)tv
	   acceptDrop:(id )info
			  row:(NSInteger)row
	dropOperation:(NSTableViewDropOperation)op
{
	NSPasteboard *pboard = [info draggingPasteboard];
	NSData *rowData = [pboard dataForType:NSStringPboardType];
	NSIndexSet *rowIndexes =
		[NSKeyedUnarchiver unarchiveObjectWithData:rowData];
	NSInteger dragRow = [rowIndexes firstIndex];
	int dateIndex = (int)row - 1;

	if (([info draggingSource] == self.timeEntriesTableView) & (tv == self.timeEntriesTableView) && row != dragRow)
	{
		if (row == 0)
		{
			dateIndex = (int)row + 1;
		}

		// Updating the dropped item date
		TimeEntryViewItem *dateModel = [viewitems objectAtIndex:dateIndex];
		TimeEntryViewItem *currentModel = [viewitems objectAtIndex:dragRow];

		if ([dateModel loadMore])
		{
			dateModel = [viewitems objectAtIndex:dateIndex - 1];
		}

		NSCalendar *calendar = [NSCalendar currentCalendar];
		NSDateComponents *components = [calendar components:(NSCalendarUnitHour | NSCalendarUnitMinute) fromDate:currentModel.started];
		NSInteger hours = [components hour];
		NSInteger minutes = [components minute];
		NSInteger seconds = [components second];

		unsigned unitFlags = NSCalendarUnitYear | NSCalendarUnitMonth |  NSCalendarUnitDay;
		NSDateComponents *comps = [calendar components:unitFlags fromDate:dateModel.started];
		comps.hour   = hours;
		comps.minute = minutes;
		comps.second = seconds;
		NSDate *newDate = [calendar dateFromComponents:comps];

		toggl_set_time_entry_date(ctx,
								  [currentModel.GUID UTF8String],
								  [newDate timeIntervalSince1970]);
	}
	return YES;
}

- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes
{
	TimeEntryCell *cellView = [self.timeEntriesTableView viewAtColumn:0 row:rowIndexes.firstIndex makeIfNecessary:NO];

	if (cellView)
	{
		[session enumerateDraggingItemsWithOptions:NSDraggingItemEnumerationConcurrent
										   forView:tableView
										   classes:[NSArray arrayWithObject:[NSPasteboardItem class]]
									 searchOptions:nil
										usingBlock:^(NSDraggingItem *draggingItem, NSInteger idx, BOOL *stop)
		 {
		     // prepare context
			 NSGraphicsContext *theContext = [NSGraphicsContext currentContext];
			 [theContext saveGraphicsState];

		     // drag image needs to be larger than the content in order to encapsulate the drop shadow
			 CGFloat imageOffset = 5;

		     // supply a drag background image
			 NSSize contentSize = draggingItem.draggingFrame.size;
			 contentSize.height = 56;
			 NSSize imageSize = NSMakeSize(contentSize.width + imageOffset, contentSize.height + imageOffset);
			 NSImage *image = [[NSImage alloc] initWithSize:imageSize];
			 [image lockFocus];

		     // define a shadow
			 NSShadow *shadow = [NSShadow new];
			 shadow.shadowColor = [[NSColor lightGrayColor] colorWithAlphaComponent:0.2];
			 shadow.shadowOffset = NSMakeSize(imageOffset, -imageOffset);
			 shadow.shadowBlurRadius = 3;
			 [shadow set];

		     // define content frame
			 NSRect contentFrame = NSMakeRect(0, imageOffset, contentSize.width, contentSize.height);
			 NSBezierPath *contentPath = [NSBezierPath bezierPathWithRect:contentFrame];

		     // draw content border and shadow
			 [[[NSColor lightGrayColor] colorWithAlphaComponent:0.6] set];
			 [contentPath stroke];
			 [theContext restoreGraphicsState];

		     // fill content
			 [[NSColor whiteColor] set];
			 contentPath = [NSBezierPath bezierPathWithRect:NSInsetRect(contentFrame, 1, 1)];
			 [contentPath fill];

			 [image unlockFocus];

		     // update the dragging item frame to accomodate larger image
			 draggingItem.draggingFrame = NSMakeRect(draggingItem.draggingFrame.origin.x, draggingItem.draggingFrame.origin.y, imageSize.width, imageSize.height);

		     // define additional image component for drag
			 NSDraggingImageComponent *backgroundImageComponent = [NSDraggingImageComponent draggingImageComponentWithKey:@"background"];
			 backgroundImageComponent.contents = image;
			 backgroundImageComponent.frame = NSMakeRect(0, 0, imageSize.width, imageSize.height);

		     // we can provide custom content by overridding NSTableViewCell -draggingImageComponents
		     // which defaults to only including the image and text fields
			 draggingItem.imageComponentsProvider = ^NSArray *(void) {
				 NSMutableArray *components = [NSMutableArray arrayWithArray:@[backgroundImageComponent]];
				 NSArray *cellViewComponents = cellView.draggingImageComponents;
				 [cellViewComponents enumerateObjectsUsingBlock:^(NSDraggingImageComponent *component, NSUInteger idx, BOOL *stop) {
					  component.frame = NSMakeRect(component.frame.origin.x, component.frame.origin.y + imageOffset, component.frame.size.width, component.frame.size.height);
				  }];

				 [components addObjectsFromArray:cellViewComponents];
				 return components;
			 };
		 }];
	}
}

@end
