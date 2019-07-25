//
//  TimeEntryEditViewController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//
#import "TimeEntryEditViewController.h"
#import "UIEvents.h"
#import "TimeEntryViewItem.h"
#import "AutocompleteDataSource.h"
#import "LiteAutoCompleteDataSource.h"
#import "NSComboBox_Expansion.h"
#import "ViewItem.h"
#import "toggl_api.h"
#import "DisplayCommand.h"
#import "Utils.h"
#import "TogglDesktop-Swift.h"
#import "UndoTextField.h"
#import "AutoCompleteTable.h"
#import "AutoCompleteTableContainer.h"

@interface TimeEntryEditViewController ()
@property LiteAutoCompleteDataSource *liteDescriptionAutocompleteDataSource;
@property LiteAutoCompleteDataSource *liteProjectAutocompleteDataSource;
@property NSTimer *timerMenubarTimer;
@property (strong, nonatomic) TimeEntryViewItem *timeEntry; // Time entry being edited
@property NSMutableArray *tagsList;
@property NSMutableArray *fullClientList;
@property NSMutableArray *workspaceClientList;
@property NSMutableArray *filteredClients;
@property NSMutableArray *workspaceList;
@property NSArray *topConstraint;
@property NSLayoutConstraint *addProjectBoxHeight;
@property NSDateFormatter *format;
@property BOOL willTerminate;
@property BOOL resizeOnOpen;
@property BOOL startTimeChanged;
@property BOOL endTimeChanged;
@property BOOL popupOnLeft;
@property NSString *descriptionComboboxPreviousStringValue;
@property NSString *projectSelectPreviousStringValue;
@property NSMutableAttributedString *clientColorTitle;
@property NSMutableAttributedString *clientColorTitleCancel;
@property NSString *GUID;
@property (strong, nonatomic) DisplayCommand *cmd;
@property (assign, nonatomic) BOOL isOpen;
@end

@implementation TimeEntryEditViewController

extern void *ctx;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
	self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
	if (self)
	{
		self.willTerminate = NO;
		self.startTimeChanged = NO;
		self.endTimeChanged = NO;
		self.popupOnLeft = NO;
		self.isOpen = NO;

		self.liteDescriptionAutocompleteDataSource = [[LiteAutoCompleteDataSource alloc] initWithNotificationName:kDisplayTimeEntryAutocomplete];

		self.liteProjectAutocompleteDataSource = [[LiteAutoCompleteDataSource alloc] initWithNotificationName:kDisplayProjectAutocomplete];

		self.timerMenubarTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
																  target:self
																selector:@selector(timerFired:)
																userInfo:nil
																 repeats:YES];

		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimeEntryEditor:)
													 name:kDisplayTimeEntryEditor
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayClientSelect:)
													 name:kDisplayClientSelect
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayWorkspaceSelect:)
													 name:kDisplayWorkspaceSelect
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTags:)
													 name:kDisplayTags
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(appWillTerminate:)
													 name:NSApplicationWillTerminateNotification
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(resetPopover:)
													 name:kResetEditPopover
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(setFocus:)
													 name:NSPopoverWillShowNotification
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(setProjectColors:)
													 name:kSetProjectColors
												   object:nil];
	}
	return self;
}

- (void)viewDidLoad
{
	self.liteDescriptionAutocompleteDataSource.input = self.descriptionAutoCompleteInput;
	[self.liteDescriptionAutocompleteDataSource setFilter:@""];

	self.liteProjectAutocompleteDataSource.input = self.projectAutoCompleteInput;
	[self.liteProjectAutocompleteDataSource setFilter:@""];

	// Setting "Add project" link color to blue
	NSColor *color = [NSColor alternateSelectedControlColor];
	NSMutableAttributedString *colorTitle =
		[[NSMutableAttributedString alloc] initWithAttributedString:[self.addProjectButton attributedTitle]];

	NSRange titleRange = NSMakeRange(0, [colorTitle length]);

	[colorTitle addAttribute:NSForegroundColorAttributeName
					   value:color
					   range:titleRange];

	self.clientColorTitle =
		[[NSMutableAttributedString alloc] initWithAttributedString:[self.addClientButton attributedTitle]];

	NSRange clientTitleRange = NSMakeRange(0, [self.clientColorTitle length]);

	[self.clientColorTitle addAttribute:NSForegroundColorAttributeName
								  value:color
								  range:clientTitleRange];

	self.clientColorTitleCancel =
		[[NSMutableAttributedString alloc] initWithString:@"cancel"];

	NSRange clientTitleRangeCancel = NSMakeRange(0, [self.clientColorTitleCancel length]);

	[self.clientColorTitleCancel addAttribute:NSForegroundColorAttributeName
										value:color
										range:clientTitleRangeCancel];

	NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc]init];
	[paragraphStyle setAlignment:NSRightTextAlignment];

	[self.clientColorTitleCancel addAttribute:NSParagraphStyleAttributeName value:paragraphStyle range:clientTitleRangeCancel];

	[self.addProjectButton setAttributedTitle:colorTitle];
	[self.addClientButton setAttributedTitle:self.clientColorTitle];
	[self.resizeHandle setCursor:[NSCursor resizeLeftRightCursor]];
	[self.resizeHandleLeft setCursor:[NSCursor resizeLeftRightCursor]];
	toggl_get_project_colors(ctx);

	// Setup autocomplete table row click event
	[self.descriptionAutoCompleteInput.autocompleteTableView setTarget:self];
	[self.descriptionAutoCompleteInput.autocompleteTableView setAction:@selector(performDescriptionTableClick:)];
	[self.projectAutoCompleteInput.autocompleteTableView setTarget:self];
	[self.projectAutoCompleteInput.autocompleteTableView setAction:@selector(performProjectTableClick:)];

	[self.projectAutoCompleteInput setButton:self.projectOpenButton];
}

- (void)viewWillAppear
{
	[super viewWillAppear];
	self.isOpen = YES;

	// Populate all data with TimeEntry
	[self populateData];
}

- (void)viewDidAppear
{
	[super viewDidAppear];

	NSRect descriptionViewFrameInWindowCoords = [self.descriptionAutoCompleteInput convertRect:[self.descriptionAutoCompleteInput bounds] toView:nil];
	NSRect projectViewFrameInWindowCoords = [self.projectAutoCompleteInput convertRect:[self.projectAutoCompleteInput bounds] toView:nil];

	[self.descriptionAutoCompleteInput setPos:(int)descriptionViewFrameInWindowCoords.origin.y];
	[self.projectAutoCompleteInput setPos:(int)projectViewFrameInWindowCoords.origin.y];

	[self.descriptionAutoCompleteInput.autocompleteTableView setDelegate:self];
	[self.projectAutoCompleteInput.autocompleteTableView setDelegate:self];
}

- (void)viewDidDisappear {
	[super viewDidDisappear];
	self.isOpen = NO;
}

- (void)loadView
{
	[super loadView];
	NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
	if (version.minorVersion < 11)
	{
		[self viewDidLoad];
		[self viewDidAppear];
	}
}

- (void)appWillTerminate:(NSNotification *)notification
{
	self.willTerminate = YES;
}

- (void)setFocus:(NSNotification *)notification
{
	if ([self.timeEntry.focusedFieldName isEqualToString:[NSString stringWithUTF8String:kFocusedFieldNameDuration]])
	{
		[self.view.window setInitialFirstResponder:self.durationTextField];
		return;
	}
	if ([self.timeEntry.focusedFieldName isEqualToString:[NSString stringWithUTF8String:kFocusedFieldNameProject]])
	{
		[self.view.window setInitialFirstResponder:self.projectAutoCompleteInput];
		return;
	}
	[self.view.window setInitialFirstResponder:self.descriptionAutoCompleteInput];
}

- (void)setProjectColors:(NSNotification *)notification
{
	[self.colorPicker setProjectColors:notification.object];
}

- (void)resetPopover:(NSNotification *)notification
{
	[self applyAddProject];

	[self.addProjectBox setHidden:YES];

	[self.projectSelectBox setHidden:NO];
	[self.projectAutoCompleteInput setHidden:NO];
	[self.projectPublicCheckbox setState:NSOffState];

	[self removeCustomConstraints];
	[self.descriptionAutoCompleteInput setNextKeyView:self.projectAutoCompleteInput];
	[self toggleAddClient:YES];
	[self.addProjectButton setNextKeyView:self.durationTextField];

	// reset autocompletes
	self.descriptionAutoCompleteInput.stringValue = @"";
	self.projectAutoCompleteInput.stringValue = @"";
	[self.descriptionAutoCompleteInput resetTable];
	[self.projectAutoCompleteInput resetTable];
	self.liteDescriptionAutocompleteDataSource.currentFilter = nil;
	self.liteProjectAutocompleteDataSource.currentFilter = nil;
}

- (IBAction)addProjectButtonClicked:(id)sender
{
	self.projectNameTextField.stringValue = @"";
	self.clientSelect.stringValue = @"";
	[self.descriptionAutoCompleteInput setNextKeyView:self.projectNameTextField];

	if (!self.addProjectBoxHeight)
	{
		self.addProjectBoxHeight = [NSLayoutConstraint constraintWithItem:self.addProjectBox
																attribute:NSLayoutAttributeHeight
																relatedBy:NSLayoutRelationEqual
																   toItem:nil
																attribute:NSLayoutAttributeNotAnAttribute
															   multiplier:1
																 constant:139];
		[self.view addConstraint:self.addProjectBoxHeight];
	}

	// If user has only one workspace, do not show the workspace combobox at all.
	BOOL singleWorkspace = YES;
	NSNumber *addedHeight;
	if (self.workspaceList.count > 1)
	{
		singleWorkspace = NO;
		self.addProjectBoxHeight.constant = 139;
		addedHeight = [NSNumber numberWithInt:115];
	}
	else
	{
		self.addProjectBoxHeight.constant = 106;
		addedHeight = [NSNumber numberWithInt:80];
	}
	[self.workspaceLabel setHidden:singleWorkspace];
	[self.workspaceSelect setHidden:singleWorkspace];

	NSDictionary *userInfo = [NSDictionary dictionaryWithObject:addedHeight forKey:@"height"];

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kResizeEditForm
																object:nil
															  userInfo:userInfo];
	[self.projectNameTextField.window makeFirstResponder:self.projectNameTextField];
	[self.addProjectBox setHidden:NO];
	[self.projectSelectBox setHidden:YES];
	[self.addProjectButton setHidden:YES];

	// Updating TAB order

	// skip colorpicker as keyboard handling is not implemented
	// [self.projectNameTextField setNextKeyView:self.colorPicker];
	// [self.colorPicker setNextKeyView:self.projectPublicCheckbox];

	[self.projectNameTextField setNextKeyView:self.projectPublicCheckbox];
	if (self.workspaceList.count > 1)
	{
		[self.projectPublicCheckbox setNextKeyView:self.workspaceSelect];
	}
	else
	{
		[self.projectPublicCheckbox setNextKeyView:self.clientSelect];
	}
	[self.clientSelect setNextKeyView:self.addClientButton];
	[self.addClientButton setNextKeyView:self.durationTextField];
	[self.workspaceSelect setNextKeyView:self.clientSelect];
}

- (void)removeCustomConstraints
{
	if (self.topConstraint)
	{
		[self.view removeConstraints:self.topConstraint];
		self.topConstraint = nil;
	}
	if (self.addProjectBoxHeight)
	{
		[self.view removeConstraint:self.addProjectBoxHeight];
		self.addProjectBoxHeight = nil;
	}
}

- (IBAction)backButtonClicked:(id)sender
{
	// This is not a good place for this (on Done button!)
	if ([self applyAddProject])
	{
		toggl_view_time_entry_list(ctx);
	}
}

// Checks if any autocompletes are focused so we don't close the popup
- (BOOL)autcompleteFocused
{
	if ([self.descriptionAutoCompleteInput currentEditor] != nil
		&& !self.descriptionAutoCompleteInput.autocompleteTableContainer.isHidden)
	{
		return YES;
	}

	if ([self.projectAutoCompleteInput currentEditor] != nil
		&& !self.projectAutoCompleteInput.autocompleteTableContainer.isHidden)
	{
		return YES;
	}
	return NO;
}

// Returns NO if there's an error and UI should not go out of the add project
// mode.
- (BOOL)applyAddProject
{
	if (self.addProjectBox.isHidden)
	{
		return YES;
	}
	NSString *projectName = self.projectNameTextField.stringValue;
	if (!projectName || !projectName.length)
	{
		return YES;
	}
	bool_t is_public = [Utils stateToBool:[self.projectPublicCheckbox state]];
	uint64_t workspaceID = [self selectedWorkspaceID];
	if (!workspaceID)
	{
		[self.workspaceSelect.window makeFirstResponder:self.workspaceSelect];
		return NO;
	}

	uint64_t clientID = 0;
	NSString *clientGUID = 0;

	ViewItem *client = [self selectedClient];
	if (client != nil)
	{
		clientID = client.ID;
		clientGUID = client.GUID;
	}

	bool_t isBillable = self.timeEntry.billable;

	char *color = (char *)[[self.colorPicker getSelectedColor] UTF8String];

	// A new project is being added!
	BOOL projectAdded = NO;
	char_t *project_guid = toggl_add_project(ctx,
											 [self.timeEntry.GUID UTF8String],
											 workspaceID,
											 clientID,
											 [clientGUID UTF8String],
											 [projectName UTF8String],
											 !is_public,
											 color);

	if (project_guid)
	{
		projectAdded = YES;
	}
	free(project_guid);

	if (projectAdded)
	{
		if (isBillable)
		{
			toggl_set_time_entry_billable(ctx, [self.timeEntry.GUID UTF8String], isBillable);
		}

		[self.addProjectBox setHidden:YES];
	}

	return projectAdded;
}

- (NSString *)comboBox:(NSComboBox *)comboBox completedString:(NSString *)partialString
{
	if (comboBox == self.clientSelect)
	{
		return @"";
	}
	NSAssert(false, @"Invalid combo box");
	return nil;
}

- (void)startDisplayTimeEntryEditor:(NSNotification *)notification
{
	[self displayTimeEntryEditor:notification.object];
}

- (void)displayTimeEntryEditor:(DisplayCommand *)cmd
{
	self.cmd = cmd;
	self.timeEntry = cmd.timeEntry;
	self.GUID = cmd.timeEntry.GUID;

	// Only populate if this popover is opening
	// If not - it will populate in viewWillAppear
	if (self.isOpen)
	{
		[self populateData];
	}
}

- (void)populateData
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	NSLog(@"TimeEntryEditViewController render, %@", self.timeEntry);

	if (self.timeEntry == nil)
	{
		return;
	}

	if (nil == self.startDate.listener)
	{
		self.startDate.listener = self;
	}

	if (self.cmd.open)
	{
		[self.liteDescriptionAutocompleteDataSource setFilter:@""];
		[self.liteProjectAutocompleteDataSource setFilter:@""];
	}

	[self.billableCheckbox setHidden:!self.timeEntry.CanSeeBillable];

	[self.billableCheckbox setState:[Utils boolToState:self.timeEntry.billable]];

	if (!self.timeEntry.CanSeeBillable)
	{
		[self.tagsTokenField setNextKeyView:self.deleteButton];
	}
	else
	{
		[self.tagsTokenField setNextKeyView:self.billableCheckbox];
	}

	// Fixes reverse tabing
	[self.addProjectButton setNextKeyView:self.durationTextField];

	if (!self.timeEntry.CanAddProjects)
	{
		[self.addProjectButton setHidden:YES];
	}
	else if ([self.addProjectBox isHidden])
	{
		[self.addProjectButton setHidden:NO];
	}

	// Overwrite description only if user is not editing it:
	if (self.cmd.open || [self.descriptionAutoCompleteInput currentEditor] == nil)
	{
		[[self.descriptionAutoCompleteInput currentEditor] setSelectedRange:NSMakeRange(0, 0)];
		[[self.descriptionAutoCompleteInput currentEditor] moveToEndOfLine:nil];
		self.descriptionAutoCompleteInput.stringValue = self.timeEntry.Description;
		self.descriptionComboboxPreviousStringValue = self.timeEntry.Description;
	}

	// Overwrite project only if user is not editing it
	if (self.cmd.open || [self.projectAutoCompleteInput currentEditor] == nil)
	{
		[[self.projectAutoCompleteInput currentEditor] setSelectedRange:NSMakeRange(0, 0)];
		[[self.projectAutoCompleteInput currentEditor] moveToEndOfLine:nil];
		if (self.timeEntry.ProjectAndTaskLabel != nil)
		{
			self.projectAutoCompleteInput.stringValue = self.timeEntry.ProjectAndTaskLabel;
			self.projectSelectPreviousStringValue = self.timeEntry.ProjectAndTaskLabel;
		}
		else
		{
			self.projectAutoCompleteInput.stringValue = @"";
			self.projectSelectPreviousStringValue = @"";
		}
	}

	// Overwrite duration only if user is not editing it:
	if (self.cmd.open || [self.durationTextField currentEditor] == nil)
	{
		[[self.durationTextField currentEditor] setSelectedRange:NSMakeRange(0, 0)];
		[[self.durationTextField currentEditor] moveToEndOfLine:nil];
		[self.durationTextField setStringValue:self.timeEntry.duration];
	}

	if (self.cmd.open || [self.startTime currentEditor] == nil || self.startTimeChanged == YES)
	{
		[self.startTime setStringValue:self.timeEntry.startTimeString];
		self.startTimeChanged = NO;
	}
	if (self.cmd.open || [self.endTime currentEditor] == nil || self.endTimeChanged == YES)
	{
		[self.endTime setStringValue:self.timeEntry.endTimeString];
		self.endTimeChanged = NO;
	}
	BOOL running = (self.timeEntry.duration_in_seconds >= 0);
	[self.startDate setDateValue:self.timeEntry.started];
	[self.startDate setEnabled:running];
	[self.startDate setDrawsBackground:running];

	[self.endTime setHidden:!running];

	// Overwrite tags only if user is not editing them right now
	if (self.cmd.open || [self.tagsTokenField currentEditor] == nil)
	{
		if ([self.timeEntry.tags count] == 0)
		{
			[self.tagsTokenField setObjectValue:nil];
		}
		else
		{
			[self.tagsTokenField setObjectValue:self.timeEntry.tags];
		}
	}

	self.currentWorkspaceLabel.stringValue = self.timeEntry.WorkspaceName;

	if (self.timeEntry.updatedAt != nil)
	{
		NSDateFormatter *df_local = [[NSDateFormatter alloc] init];
		[df_local setTimeZone:[NSTimeZone defaultTimeZone]];
		[df_local setDateFormat:@"yyyy.MM.dd 'at' HH:mm:ss"];
		NSString *localDate = [df_local stringFromDate:self.timeEntry.updatedAt];
		NSString *updatedAt = [NSLocalizedString(@"Last update ", nil) stringByAppendingString:localDate];
		[self.lastUpdateTextField setStringValue:updatedAt];
		[self.lastUpdateTextField setHidden:NO];
	}
	else
	{
		[self.lastUpdateTextField setHidden:YES];
	}

	if (self.cmd.open)
	{
		[self setFocus:nil];
	}
}

- (NSArray *)    tokenField:(NSTokenField *)tokenField
	completionsForSubstring:(NSString *)substring
			   indexOfToken:(NSInteger)tokenIndex
		indexOfSelectedItem:(NSInteger *)selectedIndex
{
	NSMutableArray *filteredCompletions = [NSMutableArray array];

	[self.tagsList enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
		 ViewItem *tag = obj;
		 if ([tag.Name rangeOfString:substring options:NSCaseInsensitiveSearch].location != NSNotFound)
		 {
			 [filteredCompletions addObject:tag.Name];
		 }
	 }];

	*selectedIndex = -1;

	return filteredCompletions;
}

- (void)applyTags
{
	NSAssert(self.timeEntry != nil, @"Cannot edit nil time entry");
	NSAssert(self.tagsTokenField != nil, @"tags field cant be nil");

	NSArray *tag_names = [self.tagsTokenField objectValue];
	const char *value = [[tag_names componentsJoinedByString:@"\t"] UTF8String];
	toggl_set_time_entry_tags(ctx,
							  [self.timeEntry.GUID UTF8String],
							  value);
}

- (void)startDisplayTags:(NSNotification *)notification
{
	[self displayTags:notification.object];
}

- (void)displayTags:(NSMutableArray *)tags
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	@synchronized(self)
	{
		self.tagsList = tags;
	}
}

- (void)startDisplayWorkspaceSelect:(NSNotification *)notification
{
	[self displayWorkspaceSelect:notification.object];
}

- (void)displayWorkspaceSelect:(NSMutableArray *)workspaces
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	@synchronized(self)
	{
		self.workspaceList = workspaces;
		NSMutableArray *ws = [[NSMutableArray alloc] init];

		for (int i = 0; i < self.workspaceList.count; i++)
		{
			ViewItem *workspace = self.workspaceList[i];
			[ws addObject:workspace.Name];
		}
		[self.workspaceSelect removeAllItems];
		[self.workspaceSelect addItemsWithTitles:[ws copy]];
	}

	uint64_t wid = [self selectedWorkspaceID];

	if (!wid && self.timeEntry)
	{
		wid = self.timeEntry.WorkspaceID;
	}

	if (!wid && self.timeEntry)
	{
		wid = self.timeEntry.DefaultWID;
	}

	if (!wid && self.workspaceList.count)
	{
		ViewItem *view = self.workspaceList[0];
		wid = view.ID;
	}

	for (int i = 0; i < self.workspaceList.count; i++)
	{
		ViewItem *workspace = self.workspaceList[i];
		if (workspace.ID == wid)
		{
			[self.workspaceSelect selectItemAtIndex:i];
			return;
		}
	}
}

- (void)startDisplayClientSelect:(NSNotification *)notification
{
	[self displayClientSelect:notification.object];
}

- (void)displayClientSelect:(NSMutableArray *)clients
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	@synchronized(self)
	{
		self.fullClientList = clients;
		self.workspaceClientList = [self findWorkspaceClientList];
		self.filteredClients = [NSMutableArray arrayWithArray:self.workspaceClientList];
	}
	[self.clientSelect reloadData];
}

- (NSMutableArray *)findWorkspaceClientList
{
	NSMutableArray *result = [[NSMutableArray alloc] init];
	uint64_t wid = [self selectedWorkspaceID];

	if (!wid)
	{
		return result;
	}
	for (ViewItem *n in self.fullClientList)
	{
		if (n.WID == wid)
		{
			[result addObject:n];
		}
	}
	return result;
}

- (uint64_t)selectedWorkspaceID
{
	if (self.workspaceList.count == 1)
	{
		ViewItem *workspace = self.workspaceList[0];
		return workspace.ID;
	}
	for (int i = 0; i < self.workspaceList.count; i++)
	{
		ViewItem *workspace = self.workspaceList[i];
		if ([workspace.Name isEqualToString:self.workspaceSelect.titleOfSelectedItem])
		{
			return workspace.ID;
		}
	}
	return 0;
}

- (ViewItem *)selectedClient
{
	for (int i = 0; i < self.workspaceClientList.count; i++)
	{
		ViewItem *client = self.workspaceClientList[i];
		if ([client.Name isEqualToString:self.clientSelect.stringValue])
		{
			return client;
		}
	}
	return nil;
}

- (void)setDragHandle:(BOOL)onLeft
{
	self.popupOnLeft = onLeft;
	[self.resizeHandle setHidden:onLeft];
	[self.resizeHandleLeft setHidden:!onLeft];
}

- (void)draggingResizeStart:(id)sender
{
	self.lastPosition = [NSEvent mouseLocation];
}

- (void)draggingResize:(id)sender
{
	NSPoint mouseLoc = [NSEvent mouseLocation];
	NSNumber *addedWidth;

	if (self.popupOnLeft)
	{
		addedWidth = [NSNumber numberWithInt:-(mouseLoc.x - self.lastPosition.x)];
	}
	else
	{
		addedWidth = [NSNumber numberWithInt:(mouseLoc.x - self.lastPosition.x)];
	}

	self.lastPosition = mouseLoc;
	NSDictionary *userInfo = [NSDictionary dictionaryWithObject:addedWidth forKey:@"width"];
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kResizeEditFormWidth
																object:nil
															  userInfo:userInfo];
}

- (IBAction)durationTextFieldChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}

	if (nil == self.timeEntry)
	{
		NSLog(@"Cannot apply duration text field changes, self.GUID is nil");
		return;
	}

	if ([[self.durationTextField stringValue] isEqualToString:self.timeEntry.duration])
	{
		NSLog(@"Duration has not changed");
		return;
	}

	const char *value = [[self.durationTextField stringValue] UTF8String];
	toggl_set_time_entry_duration(ctx, [self.timeEntry.GUID UTF8String], value);
}

- (IBAction)startTimeChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}

	if (nil == self.timeEntry)
	{
		NSLog(@"Cannot apply start time change, self.timeEntry is nil");
		return;
	}
	self.startTimeChanged = YES;

	toggl_set_time_entry_start(ctx,
							   [self.timeEntry.GUID UTF8String],
							   [self.startTime.stringValue UTF8String]);
}

- (IBAction)endTimeChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}

	self.endTimeChanged = YES;

	NSAssert(self.timeEntry != nil, @"Time entry expected");

	toggl_set_time_entry_end(ctx,
							 [self.timeEntry.GUID UTF8String],
							 [self.endTime.stringValue UTF8String]);
}

- (IBAction)dateChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}

	toggl_set_time_entry_date(ctx,
							  [self.timeEntry.GUID UTF8String],
							  [self.startDate.dateValue timeIntervalSince1970]);
}

- (IBAction)tagsChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}

	[self applyTags];
}

- (IBAction)billableCheckBoxClicked:(id)sender
{
	NSAssert(self.timeEntry != nil, @"Time entry expected");

	bool_t value = [Utils stateToBool:[self.billableCheckbox state]];
	toggl_set_time_entry_billable(ctx, [self.timeEntry.GUID UTF8String], value);
}

- (IBAction)descriptionAutoCompleteChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}

	if (self.descriptionAutoCompleteInput.stringValue != nil &&
		[self.descriptionAutoCompleteInput.stringValue isEqualToString:self.descriptionComboboxPreviousStringValue])
	{
		return;
	}

	NSAssert(self.timeEntry != nil, @"Time entry expected");

	NSString *key = [self.descriptionAutoCompleteInput stringValue];

	[self.descriptionAutoCompleteInput resetTable];
	if ([self.descriptionAutoCompleteInput.autocompleteTableView lastSavedSelected] == -1)
	{
		@synchronized(self)
		{
			dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
				toggl_set_time_entry_description(ctx,
												 [self.GUID UTF8String],
												 [key UTF8String]);
			});
		}
		[self.descriptionAutoCompleteInput.window makeFirstResponder:self.descriptionAutoCompleteInput];
		self.liteDescriptionAutocompleteDataSource.currentFilter = nil;
	}
}

- (void)updateWithSelectedDescription:(AutocompleteItem *)autocomplete withKey:(NSString *)key
{
	if (!autocomplete)
	{
		return;
	}
	self.descriptionAutoCompleteInput.stringValue = autocomplete.Description;

	@synchronized(self)
	{
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			if (![self.timeEntry.Description isEqualToString:key] &&
				!toggl_set_time_entry_project(ctx,
											  [self.GUID UTF8String],
											  autocomplete.TaskID,
											  autocomplete.ProjectID,
											  0))
			{
				[self.descriptionAutoCompleteInput.window makeFirstResponder:self.descriptionAutoCompleteInput];
				[self.descriptionAutoCompleteInput resetTable];
				self.liteDescriptionAutocompleteDataSource.currentFilter = nil;
				return;
			}

			toggl_set_time_entry_description(ctx, [self.GUID UTF8String], [autocomplete.Description UTF8String]);

			const char *value = [[autocomplete.tags componentsJoinedByString:@"\t"] UTF8String];
			toggl_set_time_entry_tags(ctx, [self.GUID UTF8String], value);

			bool_t isBillable = autocomplete.Billable;

			if (isBillable)
			{
				toggl_set_time_entry_billable(ctx, [self.GUID UTF8String], isBillable);
			}
		});
	}
	[self.descriptionAutoCompleteInput.window makeFirstResponder:self.descriptionAutoCompleteInput];
	[self.descriptionAutoCompleteInput resetTable];
	self.liteDescriptionAutocompleteDataSource.currentFilter = nil;
}

- (IBAction)projectOpenButtonClicked:(id)sender
{
	if (self.projectAutoCompleteInput.autocompleteTableContainer.isHidden)
	{
		[self.projectAutoCompleteInput toggleTableViewWithNumberOfItem:self.projectAutoCompleteInput.autocompleteTableView.numberOfRows];
		if ([self.projectAutoCompleteInput currentEditor] == nil)
		{
			[self.projectAutoCompleteInput.window makeFirstResponder:self.projectAutoCompleteInput];
		}
	}
	else
	{
		[self.projectAutoCompleteInput showAutoComplete:NO];
	}
}

- (IBAction)projectAutoCompleteChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}

	if (self.projectSelectPreviousStringValue != nil &&
		[self.projectSelectPreviousStringValue isEqualToString:self.projectAutoCompleteInput.stringValue])
	{
		return;
	}

	NSAssert(self.timeEntry != nil, @"Expected time entry");
}

- (void)updateWithSelectedProject:(AutocompleteItem *)autocomplete withKey:(NSString *)key
{
	uint64_t task_id = 0;
	uint64_t project_id = 0;

	// User inserted free text, revert field value
	if (autocomplete == nil)
	{
		self.projectAutoCompleteInput.stringValue = self.timeEntry.ProjectAndTaskLabel;
		self.projectSelectPreviousStringValue = self.timeEntry.ProjectAndTaskLabel;
		return;
	}

	task_id = autocomplete.TaskID;
	project_id = autocomplete.ProjectID;
	self.projectAutoCompleteInput.stringValue = autocomplete.ProjectAndTaskLabel;
	self.projectSelectPreviousStringValue = autocomplete.ProjectAndTaskLabel;

	@synchronized(self)
	{
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			toggl_set_time_entry_project(ctx, [self.timeEntry.GUID UTF8String], task_id, project_id, 0);
		});
	}
	[self.projectAutoCompleteInput.window makeFirstResponder:self.projectAutoCompleteInput];
	[self.projectAutoCompleteInput resetTable];
	self.liteProjectAutocompleteDataSource.currentFilter = nil;
}

- (IBAction)deleteButtonClicked:(id)sender
{
	NSAssert(self.timeEntry != nil, @"Time entry expected");

	// If description is empty and duration is less than 15 seconds delete without confirmation
	if ([self.timeEntry confirmlessDelete])
	{
		toggl_delete_time_entry(ctx, [self.timeEntry.GUID UTF8String]);
		return;
	}

	NSAlert *alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"OK"];
	[alert addButtonWithTitle:@"Cancel"];
	[alert setMessageText:@"Delete the time entry?"];
	[alert setInformativeText:@"Deleted time entries cannot be restored."];
	[alert setAlertStyle:NSWarningAlertStyle];
	if ([alert runModal] != NSAlertFirstButtonReturn)
	{
		return;
	}

	NSLog(@"Deleting time entry %@", self.timeEntry);

	toggl_delete_time_entry(ctx, [self.timeEntry.GUID UTF8String]);
}

- (NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox
{
	if (self.clientSelect == aComboBox)
	{
		return [self.filteredClients count];
	}
	NSAssert(false, @"Invalid combo box");
	return 0;
}

- (id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)row
{
	if (self.clientSelect == aComboBox)
	{
		if (row >= self.filteredClients.count)
		{
			return nil;
		}
		ViewItem *client = [self.filteredClients objectAtIndex:row];
		return client.Name;
	}

	NSAssert(false, @"Invalid combo box");
	return nil;
}

- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)aString
{
	if (self.clientSelect == aComboBox)
	{
		for (int i = 0; i < self.workspaceClientList.count; i++)
		{
			ViewItem *client = [self.workspaceClientList objectAtIndex:i];
			if ([client.Name isEqualToString:aString])
			{
				return i;
			}
		}
		return NSNotFound;
	}

	NSAssert(false, @"Invalid combo box");
	return NSNotFound;
}

/*
 * - (void)comboBoxWillPopUp:(NSNotification *)notification
 * {
 *      [self resultsInComboForString:((NSComboBox *)[notification object]).stringValue];
 * }
 */

- (NSArray *)resultsInComboForString:(NSString *)string
{
	[self.filteredClients removeAllObjects];

	if (string.length == 0 || [string isEqualToString:@""] || [string isEqualToString:@" "])
	{
		[self.filteredClients addObjectsFromArray:self.workspaceClientList];
	}
	else
	{
		for (int i = 0; i < self.workspaceClientList.count; i++)
		{
			ViewItem *client = self.workspaceClientList[i];
			if ([client.Name rangeOfString:string options:NSCaseInsensitiveSearch].location != NSNotFound)
			{
				[self.filteredClients addObject:self.workspaceClientList[i]];
			}
		}
	}

	[self.clientSelect reloadData];

	return self.filteredClients;
}

- (void)controlTextDidEndEditing:(NSNotification *)aNotification
{
	if (self.willTerminate)
	{
		return;
	}

	if ([[aNotification object] isKindOfClass:[AutoCompleteInput class]])
	{
		AutoCompleteInput *input = [aNotification object];
		input.autocompleteTableView.lastSavedSelected = [input.autocompleteTableView selectedRow];
		[input resetTable];
	}

	if (![[aNotification object] isKindOfClass:[NSTokenField class]])
	{
		// If enter was pressed then close editpopup
		if ([[[aNotification userInfo] objectForKey:@"NSTextMovement"] intValue] == NSReturnTextMovement &&
			((![[aNotification object] isKindOfClass:[NSCustomComboBox class]] && ![[aNotification object] isKindOfClass:[NSComboBox class]] ) ||
			 ![[aNotification object] isExpanded]))
		{
			[self closeEdit];
		}

		if ([[aNotification object] isKindOfClass:[NSComboBox class]])
		{
			NSCustomComboBox *comboBox = [aNotification object];
			// Reset client if entered text doesn't match any clients
			if (comboBox == self.clientSelect && !self.filteredClients.count)
			{
				[self workspaceSelectChanged:nil];
			}
		}
		return;
	}


	[self applyTags];
}

- (void)controlTextDidChange:(NSNotification *)aNotification
{
	if (self.willTerminate)
	{
		return;
	}

	if ([[aNotification object] isKindOfClass:[AutoCompleteInput class]])
	{
		AutoCompleteInput *field = [aNotification object];
		if (field == self.descriptionAutoCompleteInput)
		{
			// NSLog(@"Filter DESCRIPTION: %@", [field stringValue]);
			[self.liteDescriptionAutocompleteDataSource setFilter:[field stringValue]];
		}

		if (field == self.projectAutoCompleteInput)
		{
			// NSLog(@"Filter PROJECTS: %@", [field stringValue]);
			[self.liteProjectAutocompleteDataSource setFilter:[field stringValue]];
		}

		[field.autocompleteTableView resetSelected];
		return;
	}

	// Don't trigger combobox autocomplete when inside tags field
	if (![[aNotification object] isKindOfClass:[NSComboBox class]])
	{
		return;
	}

	NSCustomComboBox *comboBox = [aNotification object];
	NSString *filter = [comboBox stringValue];

	AutocompleteDataSource *dataSource = nil;
	if (comboBox == self.clientSelect)
	{
		[self resultsInComboForString:comboBox.stringValue];
	}

	if (dataSource != nil)
	{
		[dataSource setFilter:filter];
	}

	if (!filter || ![filter length] || (dataSource != nil && !dataSource.count))
	{
		if ([comboBox isExpanded] == YES)
		{
			[comboBox setExpanded:NO];
		}
		return;
	}

	if ([comboBox isExpanded] == NO)
	{
		[comboBox setExpanded:YES];
	}
}

// If duration field is not focused, render ticking time
// into duration field
- (void)timerFired:(NSTimer *)timer
{
	// if time entry is not running, ignore
	if (self.timeEntry == nil || self.timeEntry.duration_in_seconds >= 0)
	{
		return;
	}
	// if duration field is focussed by user, don't mess with it
	if ([self.durationTextField currentEditor] != nil)
	{
		return;
	}
	char *str = toggl_format_tracking_time_duration(self.timeEntry.duration_in_seconds);
	NSString *newValue = [NSString stringWithUTF8String:str];
	free(str);
	[self.durationTextField setStringValue:newValue];
}

- (IBAction)workspaceSelectChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}

	NSLog(@"workspaceSelectChanged");
	// Changing workspace should render the clients
	// of the selected workspace in the client select combobox.
	self.workspaceClientList = [self findWorkspaceClientList];
	self.filteredClients = [NSMutableArray arrayWithArray:self.workspaceClientList];
	self.clientSelect.stringValue = @"";
}

- (IBAction)clientSelectChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}

	NSLog(@"clientSelectChanged");
	// Changing client does not change anything in new project view.
}

- (IBAction)addClientButtonClicked:(id)sender
{
	bool openClientAdd = [self.addClientButton.title isEqualToString:@"cancel"];

	[self toggleAddClient:openClientAdd];
	if (openClientAdd)
	{
		[self.clientSelect.window makeFirstResponder:self.clientSelect];
	}
	else
	{
		[self.clientNameTextField.window makeFirstResponder:self.clientNameTextField];
	}

	[self.saveNewClientButton setNextKeyView:self.addClientButton];

	if (self.workspaceList.count > 1)
	{
		[self.projectPublicCheckbox setNextKeyView:self.workspaceSelect];
		if (openClientAdd)
		{
			[self.workspaceSelect setNextKeyView:self.clientSelect];
		}
		else
		{
			[self.workspaceSelect setNextKeyView:self.clientNameTextField];
		}
	}
	else
	{
		[self.projectPublicCheckbox setNextKeyView:self.clientNameTextField];
	}

	NSLog(@"addClientButtonClicked");
}

- (IBAction)saveAddClientButtonClicked:(id)sender;
{
	NSString *clientName = self.clientNameTextField.stringValue;

	if (!clientName || !clientName.length)
	{
		return;
	}

	uint64_t workspaceID = [self selectedWorkspaceID];
	if (!workspaceID)
	{
		[self.workspaceSelect.window makeFirstResponder:self.workspaceSelect];
		return;
	}

	char *clientGUID = toggl_create_client(ctx,
										   workspaceID,
										   [clientName UTF8String]);
	if (clientGUID)
	{
		[self toggleAddClient:YES];
		self.clientSelect.stringValue = clientName;
	}
	free(clientGUID);
}

- (void)toggleAddClient:(BOOL)showAddClient
{
	if (showAddClient)
	{
		[self.addClientButton setAttributedTitle:self.clientColorTitle];
		[self.clientNameTextField setStringValue:@""];
	}
	else
	{
		[self.addClientButton setAttributedTitle:self.clientColorTitleCancel];
	}
	[self.clientNameTextField setHidden:showAddClient];
	[self.saveNewClientButton setHidden:showAddClient];
	[self.clientSelect setHidden:!showAddClient];
}

- (void)closeEdit
{
	toggl_edit(ctx, [self.timeEntry.GUID UTF8String], false, "");
}

#pragma AutocompleteTableView Delegate

- (BOOL)  tableView:(NSTableView *)aTableView
	shouldSelectRow:(NSInteger)rowIndex
{
	AutoCompleteTable *table = (AutoCompleteTable *)aTableView;

	[table setCurrentSelected:rowIndex next:YES];
	return YES;
}

- (NSView *) tableView:(NSTableView *)tableView
	viewForTableColumn:(NSTableColumn *)tableColumn
				   row:(NSInteger)row
{
	if (row < 0)
	{
		return nil;
	}

	AutocompleteItem *item = nil;
	LiteAutoCompleteDataSource *dataSource = nil;
	AutoCompleteTable *tb = (AutoCompleteTable *)tableView;

	if (tableView == self.descriptionAutoCompleteInput.autocompleteTableView)
	{
		dataSource = self.liteDescriptionAutocompleteDataSource;
	}

	if (tableView == self.projectAutoCompleteInput.autocompleteTableView)
	{
		dataSource = self.liteProjectAutocompleteDataSource;
	}

	if (dataSource == nil || row >= dataSource.filteredOrderedKeys.count)
	{
		return nil;
	}

	@synchronized(self)
	{
		item = [dataSource.filteredOrderedKeys objectAtIndex:row];
	}
	// NSLog(@"%@", item);
	NSAssert(item != nil, @"view item from viewitems array is nil");

	AutoCompleteTableCell *cell = [tableView makeViewWithIdentifier:@"AutoCompleteTableCell"
															  owner:self];

	[cell render:item selected:(tb.lastSelected != -1 && tb.lastSelected == row)];
	return cell;
}

- (CGFloat)tableView:(NSTableView *)tableView
		 heightOfRow:(NSInteger)row
{
	return 25;
}

- (IBAction)performDescriptionTableClick:(id)sender
{
	AutoCompleteInput *input = self.descriptionAutoCompleteInput;
	LiteAutoCompleteDataSource *dataSource = self.liteDescriptionAutocompleteDataSource;
	NSInteger row = input.autocompleteTableView.lastClicked;

	if (row == -1)
	{
		row = input.autocompleteTableView.lastSavedSelected;
	}

	if (row < 0)
	{
		return;
	}
	[input.autocompleteTableView resetSelected];

	AutocompleteItem *item = [dataSource itemAtIndex:row];
	// Category clicked
	if (item == nil)
	{
		return;
	}
	[self updateWithSelectedDescription:item withKey:item.Text];
}

- (IBAction)performProjectTableClick:(id)sender
{
	AutoCompleteInput *input = self.projectAutoCompleteInput;
	LiteAutoCompleteDataSource *dataSource = self.liteProjectAutocompleteDataSource;
	NSInteger row = input.autocompleteTableView.lastClicked;

	if (row == -1)
	{
		row = input.autocompleteTableView.lastSavedSelected;
	}

	if (row < 0)
	{
		return;
	}
	[input.autocompleteTableView resetSelected];

	AutocompleteItem *item = [dataSource itemAtIndex:row];
	// Category clicked
	if (item == nil || item.Type < 0)
	{
		return;
	}
	[self updateWithSelectedProject:item withKey:item.Text];
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)fieldEditor doCommandBySelector:(SEL)commandSelector
{
	//  NSLog(@"Selector = %@", NSStringFromSelector(commandSelector) );

	BOOL retval = NO;
	AutoCompleteInput *input = nil;
	LiteAutoCompleteDataSource *dataSource = nil;
	NSInteger lastSelected = -1;

	if ([self.descriptionAutoCompleteInput currentEditor] != nil)
	{
		input = self.descriptionAutoCompleteInput;
		lastSelected = input.autocompleteTableView.lastSelected;
		dataSource = self.liteDescriptionAutocompleteDataSource;
	}
	if ([self.projectAutoCompleteInput currentEditor] != nil)
	{
		input = self.projectAutoCompleteInput;
		lastSelected = input.autocompleteTableView.lastSelected;
		dataSource = self.liteProjectAutocompleteDataSource;
	}
	if (input != nil)
	{
		if (commandSelector == @selector(moveDown:))
		{
			[input.autocompleteTableView nextItem];
			retval = YES;
		}
		if (commandSelector == @selector(moveUp:))
		{
			[input.autocompleteTableView previousItem];
			retval = YES;
		}
		if (commandSelector == @selector(insertTab:) || commandSelector == @selector(insertBacktab:))
		{
			if (input == self.projectAutoCompleteInput)
			{
				self.projectAutoCompleteInput.stringValue = self.timeEntry.ProjectAndTaskLabel;
			}
			[input resetTable];
			dataSource.currentFilter = nil;
		}
		if (commandSelector == @selector(insertNewline:))
		{
			// allow default action when autocomplete is closed
			if (input.autocompleteTableView.isHidden)
			{
				return NO;
			}

			// avoid firing default Enter actions
			retval = YES;

			// Set data according to selected item
			if (lastSelected >= 0)
			{
				AutocompleteItem *item = [dataSource itemAtIndex:lastSelected];

				if (item == nil)
				{
					return NO;
				}

				[input resetTable];
				dataSource.currentFilter = nil;

				if (input == self.descriptionAutoCompleteInput)
				{
					[self updateWithSelectedDescription:item withKey:item.Text];
				}
				else if (input == self.projectAutoCompleteInput)
				{
					[self updateWithSelectedProject:item withKey:item.Text];
				}
			}
		}
	}

	return retval;
}

- (void)registerUndoForAllFields
{
	if (self.timeEntry == nil)
	{
		return;
	}

	TimeEntrySnapshot *snapshot = [[UndoManager shared] getSnapshotFor:self.timeEntry];
	NSLog(@"%@", snapshot.debugDescription);
	if (snapshot != nil)
	{
		[self.descriptionAutoCompleteInput registerUndoWithValue:snapshot.descriptionUndoValue];
		[self.projectAutoCompleteInput registerUndoWithValue:snapshot.projectLableUndoValue.projectAndTaskLabel];
		[self.durationTextField registerUndoWithValue:snapshot.durationUndoValue];
		[self.startTime registerUndoWithValue:snapshot.startTimeUndoValue];
		[self.endTime registerUndoWithValue:snapshot.endTimeUndoValue];
		[self registerUndoForProjectWithSnapshot:snapshot.projectLableUndoValue];
	}
	else
	{
		[self.descriptionAutoCompleteInput.undoManager removeAllActions];
		[self.projectAutoCompleteInput.undoManager removeAllActions];
		[self.durationTextField.undoManager removeAllActions];
		[self.startTime.undoManager removeAllActions];
		[self.endTime.undoManager removeAllActions];
		[self.projectAutoCompleteInput.undoManager removeAllActions];
	}
}

- (void)setTimeEntry:(TimeEntryViewItem *)timeEntry
{
	_timeEntry = timeEntry;
	[self registerUndoForAllFields];
}

- (void)registerUndoForProjectWithSnapshot:(ProjectSnapshot *)snapshot
{
	if (snapshot == nil)
	{
		return;
	}

	[self.projectAutoCompleteInput.undoManager removeAllActions];
	[self.projectAutoCompleteInput.undoManager registerUndoWithTarget:self
															 selector:@selector(updateProjectValueForProjectAutoCompleteInput:)
															   object:[[AutocompleteItem alloc] initWithSnapshot:snapshot]];
}

- (void)updateProjectValueForProjectAutoCompleteInput:(AutocompleteItem *)item
{
	[self updateWithSelectedProject:item withKey:nil];
}

@end

// http://stackoverflow.com/questions/4499262/how-to-programmatically-open-an-nscomboboxs-list

@implementation NSComboBox (ExpansionAPI)

- (BOOL)isExpanded
{
	return self.accessibilityExpanded;
}

- (void)setExpanded:(BOOL)expanded
{
	self.accessibilityExpanded = expanded;
}

@end
