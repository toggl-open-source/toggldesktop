//
//  TimeEntryListViewController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "TimeEntryListViewController.h"
#import "TimeEntryViewItem.h"
#import "TimeEntryCell.h"
#import "DisplayCommand.h"
#import "ConvertHexColor.h"
#import "TogglDesktop-Swift.h"
#import "TimeEntryCollectionView.h"
#import "BetterFocusAutoCompleteInput.h"

static void *XXContext = &XXContext;
static NSString *kFrameKey = @"frame";

@interface TimeEntryListViewController () <TimeEntryDatasourceDraggingDelegate, TimeEntryEmptyViewDelegate>
@property (weak) IBOutlet NSView *headerView;
@property (weak) IBOutlet NSScrollView *timeEntryListScrollView;
@property (weak) IBOutlet TimeEntryCollectionView *collectionView;
@property (weak) IBOutlet NSBox *emptyViewContainerView;

@property (nonatomic, strong) TimeEntryDatasource *dataSource;
@property (nonatomic, assign) NSInteger defaultPopupHeight;
@property (nonatomic, assign) NSInteger defaultPopupWidth;
@property (nonatomic, assign) NSInteger minimumEditFormWidth;
@property (nonatomic, assign) BOOL runningEdit;
@property (nonatomic, copy) NSString *lastSelectedGUID;
@property (nonatomic, strong) TimeEntryEmptyView *emptyView;
@property (nonatomic, strong) EditorPopover *timeEntrypopover;
@property (nonatomic, assign) BOOL isOpening;
@end

@implementation TimeEntryListViewController

extern void *ctx;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
	self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
	if (self)
	{
	}
	return self;
}

- (void)dealloc
{
	[self.collectionView removeObserver:self forKeyPath:kFrameKey];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)viewDidLoad
{
	[super viewDidLoad];

	[self initCommon];
	[self initEditorPopover];
	[self initCollectionView];
	[self initEmptyView];
	[self initNotifications];
}

- (void)viewDidAppear
{
	[super viewDidAppear];
	self.isOpening = YES;
	[self.collectionView reloadData];
}

- (void)viewWillDisappear
{
	[super viewWillDisappear];
	self.isOpening = NO;
}

- (void)initCommon
{
	self.isOpening = YES;
	self.runningEdit = NO;

	// Shadow for Header
    [self.headerView applyShadowWithColor:[NSColor blackColor] opacity:0.1 radius:6.0 offset:CGSizeMake(0, -2)];
}

- (void)initNotifications
{
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
											 selector:@selector(resizeEditPopupWidth:)
												 name:kResizeEditFormWidth
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(resetEditPopover:)
												 name:NSPopoverDidCloseNotification
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
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(effectiveAppearanceChangedNotification)
												 name:NSNotification.EffectiveAppearanceChanged
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(windowSizeDidChange)
												 name:NSWindowDidResizeNotification
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(deselectAllTimeEntryNotification)
												 name:kDeselectAllTimeEntryList
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(windowDidBecomeKeyNotification:)
												 name:NSWindowDidBecomeKeyNotification
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(touchBarSettingChangedNotification:)
												 name:kTouchBarSettingChanged
											   object:nil];
}

- (void)initCollectionView
{
	self.dataSource = [[TimeEntryDatasource alloc] initWithCollectionView:self.collectionView];
	self.dataSource.delegate = self;
	[self.collectionView addObserver:self
						  forKeyPath:kFrameKey
							 options:NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew
							 context:XXContext];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if (context == XXContext)
	{
		CGRect oldFrame = CGRectZero;
		CGRect newFrame = CGRectZero;
		id oldValue = change[NSKeyValueChangeOldKey];
		id newValue = change[NSKeyValueChangeNewKey];

		if ([oldValue respondsToSelector:@selector(CGRectValue)])
		{
			oldFrame = [oldValue CGRectValue];
		}
		if ([newValue respondsToSelector:@selector(CGRectValue)])
		{
			newFrame = [newValue CGRectValue];
		}

		if (oldFrame.size.width != newFrame.size.width)
		{
			// HACK
			// Relayout if the scrollbar is appear or disappear
			[self.collectionView.collectionViewLayout invalidateLayout];
		}
	}
	else
	{
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	}
}

- (void)initEmptyView
{
	self.emptyView = [TimeEntryEmptyView viewFromXIB];
	self.emptyView.delegate = self;
	[self.emptyViewContainerView addSubview:self.emptyView];
	[self.emptyView edgesToSuperView];
	self.emptyViewContainerView.hidden = YES;
}

- (void)initEditorPopover {
	self.timeEntrypopover = [[EditorPopover alloc] init];
	[self.timeEntrypopover prepareViewController];
}

- (void)startDisplayTimeEntryList:(NSNotification *)notification
{
	[self displayTimeEntryList:notification.object];
}

- (void)displayTimeEntryList:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSArray<TimeEntryViewItem *> *newTimeEntries = [cmd.timeEntries copy];

	// reload
	[self.dataSource process:newTimeEntries showLoadMore:cmd.show_load_more];

	// Handle Popover
	if (cmd.open)
	{
		if (self.timeEntrypopover.shown)
		{
			[self.timeEntrypopover performClose:self];
		}
	}

	// Adjust the popover position if we change the date
	[self adjustPositionOfPopover];

	[self handleEmptyView];
}

- (void)adjustPositionOfPopover {
	if (!self.timeEntrypopover.shown)
	{
		return;
	}

	if (self.lastSelectedGUID == nil)
	{
		return;
	}

	// Get Selecte Item from last GUID
	TimeEntryViewItem *item = [self.dataSource objectWith:self.lastSelectedGUID];
	if (item == nil)
	{
		return;
	}
	NSIndexPath *indexPath = [self.dataSource indexPathFor:item];
	if (indexPath == nil)
	{
		return;
	}

	// Adjus the position of arrow
	TimeEntryCell *cell = [self getTimeEntryCellAtIndexPath:indexPath];
	NSRect positionRect = [self positionRectForItem:cell];
	self.timeEntrypopover.positioningRect = positionRect;

	// Scroll to visible selected row
	if (!NSContainsRect(self.collectionView.visibleRect, positionRect))
	{
		[self.collectionView scrollToItemsAtIndexPaths:[NSSet setWithCollectionViewIndexPath:indexPath]
										scrollPosition:NSCollectionViewScrollPositionBottom];
	}

	// Hightlight selected cell
	[[self.collectionView getSelectedEntryCells].firstObject setFocused];
}

- (void)resetEditPopover:(NSNotification *)notification
{
	if (notification.object == self.timeEntrypopover)
	{
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kResetEditPopover
																	object:nil];
	}
}

- (void)popoverWillClose:(NSNotification *)notification
{
	NSLog(@"%@", notification.userInfo);
}

- (void)displayTimeEntryEditor:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSLog(@"✅ TimeEntryListViewController displayTimeEntryEditor, thread %@", [NSThread currentThread]);

    self.runningEdit = [cmd.timeEntry isRunning];

	// Skip render if need
	if (!self.isOpening && !self.runningEdit)
	{
		return;
	}

	TimeEntryViewItem *timeEntry = cmd.timeEntry;
	if (cmd.open)
	{
		NSView *ofView = self.view;
		TimeEntryCell *selectedCell = [self.collectionView getSelectedEntryCells].firstObject;
		CGRect positionRect = [self positionRectForItem:selectedCell];

		if (self.runningEdit)
		{
			ofView = [self.delegate containerViewForTimer];
			positionRect = [ofView bounds];
			self.lastSelectedGUID = nil;
		}
		else if (selectedCell != nil)
		{
			self.lastSelectedGUID = selectedCell.GUID;
			ofView = self.collectionView;
		}
		else
		{
			// It's for new Time Entry from Manual Timer
			NSCollectionViewItem *firstItem = [self.collectionView itemAtIndexPath:[NSIndexPath indexPathForItem:0 inSection:0]];
			if ([firstItem isKindOfClass:[TimeEntryCell class]])
			{
				TimeEntryCell *timeEntryCell = (TimeEntryCell *)firstItem;
				self.lastSelectedGUID = timeEntryCell.GUID;
				positionRect = [self positionRectForItem:timeEntryCell];
				ofView = self.collectionView;
			}
		}

		// Show popover
		[self.timeEntrypopover presentFrom:positionRect of:ofView preferredEdge:NSRectEdgeMaxX];
	}

	// Update time entry for editor
	[self.timeEntrypopover setTimeEntry:timeEntry];
}

- (CGRect)positionRectForItem:(TimeEntryCell *)timeEntry {
	if (timeEntry)
	{
		return [self.collectionView convertRect:timeEntry.view.bounds
									   fromView:timeEntry.view];
	}
	return self.view.bounds;;
}

- (void)startDisplayTimeEntryEditor:(NSNotification *)notification
{
	[self displayTimeEntryEditor:notification.object];
}

- (TimeEntryCell *)getTimeEntryCellAtIndexPath:(NSIndexPath *)indexPath
{
	if (indexPath.section < 0 ||  indexPath.section >= self.dataSource.count)
	{
		return nil;
	}

	id item = [self.collectionView itemAtIndexPath:indexPath];
	if ([item isKindOfClass:[TimeEntryCell class]])
	{
		return item;
	}
	return nil;
}

- (void)clearLastSelectedEntry
{
	[[self.collectionView getSelectedEntryCells].firstObject setupGroupMode];
}

- (void)resetEditPopoverSize:(NSNotification *)notification
{
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kResetEditPopover
																object:nil];
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
}

- (void)startDisplayLogin:(NSNotification *)notification
{
	[self displayLogin:notification.object];
}

- (void)displayLogin:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	if (cmd.open && self.timeEntrypopover.shown)
	{
		[self.timeEntrypopover performClose:self];
	}
}

- (void)focusListing:(NSNotification *)notification
{
	if (self.dataSource.count == 0)
	{
		return;
	}

	NSIndexPath *selectedIndexpath = [self.collectionView.selectionIndexPaths.allObjects firstObject];

	// Don't select the first item if the TE List is focusing with selected item
	if (selectedIndexpath != nil && self.collectionView.isFirstResponder)
	{
		return;
	}

	// Select first cell
	[self.dataSource selectFirstItem];
}

- (void)escapeListing:(NSNotification *)notification
{
	if (self.timeEntrypopover.shown)
	{
		return;
	}
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kFocusTimer
																object:nil];
	[self clearLastSelectedEntry];
	[self.collectionView deselectAll:nil];
}

#pragma mark Drag & Drop Delegates

- (void)effectiveAppearanceChangedNotification {
	// Re-draw hard-code color sheme for all cells in tableview
	[self.collectionView reloadData];
}

- (void)windowSizeDidChange {
	// We have to reload entire collection rather than calling [self.collectionView.collectionViewLayout invalidateLayout];
	// Because it's difficult to re-draw the mask for highlight state of TimeEntryCell
	// -invalidateLayout is more better in term of performance
	// User is rarely to resize the app, so I believe it's reasonable.
	[self.collectionView reloadData];
}

- (BOOL)collectionView:(NSCollectionView *_Nonnull)collectionView acceptDrop:(id<NSDraggingInfo> _Nonnull)draggingInfo indexPath:(NSIndexPath *_Nonnull)indexPath dropOperation:(enum NSCollectionViewDropOperation)dropOperation {
	NSPasteboard *pboard = [draggingInfo draggingPasteboard];
	NSData *rowData = [pboard dataForType:NSStringPboardType];
	id data = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];

	// Make sure it's an array of indexPath
	if ([data isKindOfClass:[NSArray<NSIndexPath *> class]])
	{
		NSArray *moveIndexPaths = (NSArray *)data;

		// Update items
		for (NSIndexPath *moveIndexPath in moveIndexPaths)
		{
			TimeEntryViewItem *dateModel = [self.dataSource objectAt:indexPath];
			TimeEntryViewItem *currentModel = [self.dataSource objectAt:moveIndexPath];

			if (dateModel != nil && currentModel != nil && !dateModel.loadMore && !currentModel.loadMore)
			{
				NSCalendar *calendar = [NSCalendar currentCalendar];
				NSDateComponents *components = [calendar components:(NSCalendarUnitHour | NSCalendarUnitMinute) fromDate:currentModel.started];
				NSInteger hours = [components hour];
				NSInteger minutes = [components minute];
				NSInteger seconds = [components second];

				unsigned unitFlags = NSCalendarUnitYear | NSCalendarUnitMonth |  NSCalendarUnitDay;
				NSDateComponents *comps = [calendar components:unitFlags fromDate:dateModel.started];
				comps.hour = hours;
				comps.minute = minutes;
				comps.second = seconds;
				NSDate *newDate = [calendar dateFromComponents:comps];

				toggl_set_time_entry_date(ctx,
										  [currentModel.GUID UTF8String],
										  [newDate timeIntervalSince1970]);
			}
		}
	}

	return YES;
}

- (void)collectionView:(NSCollectionView *_Nonnull)collectionView draggingSession:(NSDraggingSession *_Nonnull)session willBeginAt:(NSPoint)screenPoint forItemsAt:(NSSet<NSIndexPath *> *_Nonnull)indexPaths {
	NSIndexPath *indexPath = indexPaths.allObjects.firstObject;

	if (indexPath == nil)
	{
		return;
	}

	NSCollectionViewItem *item = [collectionView itemAtIndexPath:indexPath];
	if (![item isKindOfClass:[TimeEntryCell class]])
	{
		return;
	}

	TimeEntryCell *cellView = (TimeEntryCell *)item;
	if (cellView)
	{
		[session enumerateDraggingItemsWithOptions:NSDraggingItemEnumerationConcurrent
										   forView:collectionView
										   classes:[NSArray arrayWithObject:[NSPasteboardItem class]]
									 searchOptions:@{}
										usingBlock:^(NSDraggingItem *_Nonnull draggingItem, NSInteger idx, BOOL *_Nonnull stop)
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
			 shadow.shadowColor = [[NSColor lightGrayColor] colorWithAlphaComponent:0.1];
			 shadow.shadowOffset = NSMakeSize(imageOffset, -imageOffset);
			 shadow.shadowBlurRadius = 8;
			 [shadow set];

		     // define content frame
			 NSRect contentFrame = NSMakeRect(0, imageOffset, contentSize.width, contentSize.height);
			 [theContext restoreGraphicsState];

			 NSColor *backgroundColor;
			 if (@available(macOS 10.13, *))
			 {
				 backgroundColor = [NSColor colorNamed:@"white-background-hover-color"];
			 }
			 else
			 {
				 backgroundColor = [ConvertHexColor hexCodeToNSColor:@"#f9f9f9"];
			 }

		     // fill content
			 [backgroundColor set];
			 NSBezierPath *contentPath = [NSBezierPath bezierPathWithRect:NSInsetRect(contentFrame, 1, 1)];
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

- (void)handleEmptyView
{
	if (self.dataSource.count == 1 && self.dataSource.isShowLoadMore)
	{
		BOOL isSignUp = [[NSUserDefaults standardUserDefaults] boolForKey:kUserHasBeenSignup];
		if (isSignUp)
		{
			// show welcome
			[self layoutEmptyViewWithLayoutType:EmptyLayoutTypeWelcome];
		}
		else
		{
			// Show No entry
			[self layoutEmptyViewWithLayoutType:EmptyLayoutTypeNoEntry];
		}
	}
	else if (self.dataSource.count == 0)
	{
		// welcome
		[self layoutEmptyViewWithLayoutType:EmptyLayoutTypeWelcome];
	}
	else
	{
		// Has entry
		self.emptyViewContainerView.hidden = YES;
		self.timeEntryListScrollView.hidden = NO;
	}
}

- (void)layoutEmptyViewWithLayoutType:(EmptyLayoutType)type
{
	switch (type)
	{
		case EmptyLayoutTypeNoEntry :
			self.emptyViewContainerView.hidden = NO;
			self.timeEntryListScrollView.hidden = YES;
			[self.emptyView setLayoutType:EmptyLayoutTypeNoEntry];
			break;
		case EmptyLayoutTypeWelcome :
			self.emptyViewContainerView.hidden = NO;
			self.timeEntryListScrollView.hidden = YES;
			[self.emptyView setLayoutType:EmptyLayoutTypeWelcome];
			break;
	}
}

- (void)emptyViewDidTapOnLoadMore
{
	toggl_load_more(ctx);
}

- (BOOL)isEditorOpen
{
	return self.timeEntrypopover.shown;
}

- (void)deselectAllTimeEntryNotification
{
	[self.collectionView deselectAll:self];
}

- (void)windowDidBecomeKeyNotification:(NSNotification *)notification
{
    // Don't focus on Timer Bar if the Editor is presented
    if (self.timeEntrypopover.isShown)
    {
        return;
    }

    // Only focus if the window is main
    // Otherwise, shouldn't override the firstResponder
    if (notification.object != self.view.window)
    {
        return;
    }
    [self.delegate shouldFocusTimer];
}

- (void)loadMoreIfNeedAtDate:(NSDate *)date;
{
	[self.dataSource loadMoreTimeEntryIfNeedAt:date];
}

- (NSTouchBar *)makeTouchBar
{
	return [[TouchBarService shared] makeTouchBar];
}

- (void)touchBarSettingChangedNotification:(NSNotification *)noti
{
    if (@available(macOS 10.12.2, *))
    {
        self.touchBar = nil;
    }
}

-(NSView * __nullable)firstTimeEntryCellForOnboarding
{
    return [self.collectionView itemAtIndexPath:[NSIndexPath indexPathForItem:0 inSection:0]].view;
}
@end
