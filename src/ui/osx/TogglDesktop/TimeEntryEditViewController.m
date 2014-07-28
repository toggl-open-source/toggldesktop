//
//  TimeEntryEditViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//
#import "TimeEntryEditViewController.h"
#import "UIEvents.h"
#import "TimeEntryViewItem.h"
#import "AutocompleteItem.h"
#import "AutocompleteDataSource.h"
#import "NSComboBox_Expansion.h"
#import "ViewItem.h"
#import "NSCustomComboBoxCell.h"
#import "NSCustomComboBox.h"
#import "kopsik_api.h"
#import "DisplayCommand.h"
#import "Utils.h"

@interface TimeEntryEditViewController ()
@property AutocompleteDataSource *projectAutocompleteDataSource;
@property AutocompleteDataSource *descriptionComboboxDataSource;
@property NSTimer *timerMenubarTimer;
@property TimeEntryViewItem *timeEntry; // Time entry being edited
@property NSMutableArray *tagsList;
@property NSMutableArray *fullClientList;
@property NSMutableArray *workspaceClientList;
@property NSMutableArray *workspaceList;
@property NSArray *topConstraint;
@property NSLayoutConstraint *addProjectBoxHeight;
@property NSDateFormatter *format;
@property BOOL willTerminate;
@property BOOL resizeOnOpen;
@property BOOL startTimeChanged;
@property BOOL endTimeChanged;
@end

@implementation TimeEntryEditViewController

extern void *ctx;
extern int kDurationStringLength;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
	self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
	if (self)
	{
		self.willTerminate = NO;
		self.startTimeChanged = NO;
		self.endTimeChanged = NO;

		self.projectAutocompleteDataSource = [[AutocompleteDataSource alloc] initWithNotificationName:kDisplayProjectAutocomplete];
		self.descriptionComboboxDataSource = [[AutocompleteDataSource alloc] initWithNotificationName:kDisplayTimeEntryAutocomplete];

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
												 selector:@selector(checkResize:)
													 name:NSPopoverDidShowNotification
												   object:nil];
	}
	return self;
}

- (void)viewDidLoad
{
	self.projectAutocompleteDataSource.combobox = self.projectSelect;
	self.descriptionComboboxDataSource.combobox = self.descriptionCombobox;

	[self.projectAutocompleteDataSource setFilter:@""];
	[self.descriptionComboboxDataSource setFilter:@""];

	// Setting "Add project" link color to blue
	NSColor *color = [NSColor alternateSelectedControlColor];
	NSMutableAttributedString *colorTitle =
		[[NSMutableAttributedString alloc] initWithAttributedString:[self.addProjectButton attributedTitle]];

	NSRange titleRange = NSMakeRange(0, [colorTitle length]);

	[colorTitle addAttribute:NSForegroundColorAttributeName
					   value:color
					   range:titleRange];

	[self.addProjectButton setAttributedTitle:colorTitle];
}

- (void)loadView
{
	[super loadView];
	[self viewDidLoad];
}

- (void)appWillTerminate:(NSNotification *)notification
{
	self.willTerminate = YES;
}

- (void)checkResize:(NSNotification *)notification
{
	if (self.resizeOnOpen)
	{
		[self toggleTimeForm:YES];
		[self.durationTextField becomeFirstResponder];
	}
	self.resizeOnOpen = NO;
}

- (void)resetPopover:(NSNotification *)notification
{
	[self applyAddProject];

	[self.addProjectBox setHidden:YES];

	[self.projectSelectBox setHidden:NO];
	[self.projectPublicCheckbox setState:NSOffState];

	[self removeCustomConstraints];
	[self.descriptionCombobox setNextKeyView:self.projectSelect];
	[self toggleTimeForm:NO];
}

- (IBAction)addProjectButtonClicked:(id)sender
{
	self.projectNameTextField.stringValue = @"";
	self.clientSelect.stringValue = @"";
	[self.descriptionCombobox setNextKeyView:self.projectNameTextField];

	if (!self.addProjectBoxHeight)
	{
		self.addProjectBoxHeight = [NSLayoutConstraint constraintWithItem:self.addProjectBox
																attribute:NSLayoutAttributeHeight
																relatedBy:NSLayoutRelationEqual
																   toItem:nil
																attribute:NSLayoutAttributeNotAnAttribute
															   multiplier:1
																 constant:129];
		[self.view addConstraint:self.addProjectBoxHeight];
	}

	// If user has only one workspace, do not show the workspace combobox at all.
	BOOL singleWorkspace = YES;
	NSNumber *addedHeight;
	if (self.workspaceList.count > 1)
	{
		singleWorkspace = NO;
		self.addProjectBoxHeight.constant = 129;
		addedHeight = [NSNumber numberWithInt:100];
	}
	else
	{
		self.addProjectBoxHeight.constant = 96;
		addedHeight = [NSNumber numberWithInt:70];
	}
	[self.workspaceLabel setHidden:singleWorkspace];
	[self.workspaceSelect setHidden:singleWorkspace];

	NSDictionary *userInfo = [NSDictionary dictionaryWithObject:addedHeight forKey:@"height"];

	[[NSNotificationCenter defaultCenter] postNotificationName:kResizeEditForm
														object:nil
													  userInfo:userInfo];
	[self.projectNameTextField becomeFirstResponder];
	[self.addProjectBox setHidden:NO];
	[self.projectSelectBox setHidden:YES];
	[self.addProjectButton setHidden:YES];
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
		kopsik_view_time_entry_list(ctx);
	}
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
	_Bool is_public = [Utils stateToBool:[self.projectPublicCheckbox state]];
	uint64_t workspaceID = [self selectedWorkspaceID];
	if (!workspaceID)
	{
		[self.workspaceSelect becomeFirstResponder];
		return NO;
	}
	uint64_t clientID = [self selectedClientID];
	_Bool isBillable = self.timeEntry.billable;
	// A new project is being added!
	_Bool projectAdded = kopsik_add_project(ctx,
											[self.timeEntry.GUID UTF8String],
											workspaceID,
											clientID,
											[projectName UTF8String],
											!is_public);

	if (projectAdded && isBillable)
	{
		kopsik_set_time_entry_billable(ctx, [self.timeEntry.GUID UTF8String], isBillable);
	}

	if (projectAdded)
	{
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
	if (comboBox == self.workspaceSelect)
	{
		return @"";
	}
	NSAssert(false, @"Invalid combo box");
	return nil;
}

- (void)startDisplayTimeEntryEditor:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayTimeEntryEditor:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayTimeEntryEditor:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.timeEntry = cmd.timeEntry;

	NSLog(@"TimeEntryEditViewController render, %@", self.timeEntry);

	if (nil == self.startDate.listener)
	{
		self.startDate.listener = self;
	}

	if (cmd.open)
	{
		[self.projectAutocompleteDataSource setFilter:@""];
		[self.descriptionComboboxDataSource setFilter:@""];
	}

	[self.billableCheckbox setHidden:!self.timeEntry.CanSeeBillable];

	[self.billableCheckbox setState:[Utils boolToState:self.timeEntry.billable]];

	if (!self.timeEntry.CanAddProjects)
	{
		[self.addProjectButton setHidden:YES];
	}
	else if ([self.addProjectBox isHidden])
	{
		[self.addProjectButton setHidden:NO];
	}

	// Overwrite description only if user is not editing it:
	if ([self.descriptionCombobox currentEditor] == nil)
	{
		[self.descriptionCombobox setStringValue:self.timeEntry.Description];
	}

	// Overwrite project only if user is not editing it
	if ([self.projectSelect currentEditor] == nil)
	{
		if (self.timeEntry.ProjectAndTaskLabel != nil)
		{
			[self.projectSelect setStringValue:self.timeEntry.ProjectAndTaskLabel];
		}
		else
		{
			[self.projectSelect setStringValue:@""];
		}
	}

	// Overwrite duration only if user is not editing it:
	if ([self.durationTextField currentEditor] == nil)
	{
		[self.durationTextField setStringValue:self.timeEntry.duration];
	}

	// Set TimeDateTextBox value
	NSString *dateString = [NSString stringWithFormat:@"%@ ", self.timeEntry.formattedDate];
	NSString *timeString;
	if (self.timeEntry.durOnly)
	{
		timeString = [NSString stringWithFormat:@"for %@ ", self.timeEntry.duration];

		NSDictionary *viewsDict = NSDictionaryOfVariableBindings(_durationBox, _dateBox);
		self.topConstraint = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[_durationBox]-0@1000-[_dateBox]"
																	 options:0
																	 metrics:nil
																	   views:viewsDict];
		[self.view addConstraints:self.topConstraint];
	}
	else
	{
		if (self.topConstraint != nil)
		{
			[self.view removeConstraints:self.topConstraint];
			self.topConstraint = nil;
		}
		if (self.timeEntry.endTimeString.length)
		{
			timeString = [NSString stringWithFormat:@"from %@ to %@", self.timeEntry.startTimeString, self.timeEntry.endTimeString];
		}
		else
		{
			timeString = [NSString stringWithFormat:@"from %@", self.timeEntry.startTimeString];
		}
	}
	NSString *dateTimeString = [dateString stringByAppendingString:timeString];
	[self.dateTimeTextField setStringValue:dateTimeString];

	if ([self.startTime currentEditor] == nil || self.startTimeChanged == YES)
	{
		[self.startTime setStringValue:self.timeEntry.startTimeString];
		self.startTimeChanged = NO;
	}
	if ([self.endTime currentEditor] == nil || self.endTimeChanged == YES)
	{
		[self.endTime setStringValue:self.timeEntry.endTimeString];
		self.endTimeChanged = NO;
	}
	BOOL running = (self.timeEntry.duration_in_seconds >= 0);
	[self.startDate setDateValue:self.timeEntry.started];
	[self.startDate setEnabled:running];
	[self.startDate setDrawsBackground:running];

	[self.endTime setHidden:!running];

	[self.startEndTimeBox setHidden:self.timeEntry.durOnly];

	// Overwrite tags only if user is not editing them right now
	if ([self.tagsTokenField currentEditor] == nil)
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

	if (self.timeEntry.updatedAt != nil)
	{
		NSDateFormatter *df_local = [[NSDateFormatter alloc] init];
		[df_local setTimeZone:[NSTimeZone defaultTimeZone]];
		[df_local setDateFormat:@"yyyy.MM.dd 'at' HH:mm:ss"];
		NSString *localDate = [df_local stringFromDate:self.timeEntry.updatedAt];
		NSString *updatedAt = [@"Last update " stringByAppendingString : localDate];
		[self.lastUpdateTextField setStringValue:updatedAt];
		[self.lastUpdateTextField setHidden:NO];
	}
	else
	{
		[self.lastUpdateTextField setHidden:YES];
	}

	if (cmd.open)
	{
		if ([self.timeEntry.focusedFieldName isEqualToString:[NSString stringWithUTF8String:kFocusedFieldNameDuration]])
		{
			self.resizeOnOpen = YES;
			[self.durationTextField becomeFirstResponder];
		}
		if ([self.timeEntry.focusedFieldName isEqualToString:[NSString stringWithUTF8String:kFocusedFieldNameDescription]])
		{
			[self.descriptionCombobox becomeFirstResponder];
		}
		if ([self.timeEntry.focusedFieldName isEqualToString:[NSString stringWithUTF8String:kFocusedFieldNameProject]])
		{
			[self.projectSelectBox becomeFirstResponder];
		}
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
		 if ([[tag.Name lowercaseString] hasPrefix:[substring lowercaseString]])
		 {
			 [filteredCompletions addObject:tag.Name];
		 }
	 }];

	return filteredCompletions;
}

- (void)applyTags
{
	NSAssert(self.timeEntry != nil, @"Cannot edit nil time entry");
	NSAssert(self.tagsTokenField != nil, @"tags field cant be nil");

	NSArray *tag_names = [self.tagsTokenField objectValue];
	const char *value = [[tag_names componentsJoinedByString:@"|"] UTF8String];
	kopsik_set_time_entry_tags(ctx,
							   [self.timeEntry.GUID UTF8String],
							   value);
}

- (void)startDisplayTags:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayTags:)
						   withObject:notification.object
						waitUntilDone:NO];
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
	[self performSelectorOnMainThread:@selector(displayWorkspaceSelect:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayWorkspaceSelect:(NSMutableArray *)workspaces
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	@synchronized(self)
	{
		self.workspaceList = workspaces;
	}

	uint64_t wid = [self selectedWorkspaceID];

	self.workspaceSelect.usesDataSource = YES;
	if (self.workspaceSelect.dataSource == nil)
	{
		self.workspaceSelect.dataSource = self;
	}
	[self.workspaceSelect reloadData];

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
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	[self performSelectorOnMainThread:@selector(displayClientSelect:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayClientSelect:(NSMutableArray *)clients
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	@synchronized(self)
	{
		self.fullClientList = clients;
		self.workspaceClientList = [self findWorkspaceClientList];
	}

	self.clientSelect.usesDataSource = YES;
	if (self.clientSelect.dataSource == nil)
	{
		self.clientSelect.dataSource = self;
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
	for (int i = 0; i < self.workspaceList.count; i++)
	{
		ViewItem *workspace = self.workspaceList[i];
		if ([workspace.Name isEqualToString:self.workspaceSelect.stringValue])
		{
			return workspace.ID;
		}
	}
	return 0;
}

- (uint64_t)selectedClientID
{
	for (int i = 0; i < self.workspaceClientList.count; i++)
	{
		ViewItem *client = self.workspaceClientList[i];
		if ([client.Name isEqualToString:self.clientSelect.stringValue])
		{
			return client.ID;
		}
	}
	return 0;
}

- (void)textFieldClicked:(id)sender
{
	if (sender == self.dateTimeTextField)
	{
		[self toggleTimeForm:YES];
	}
}

- (void)toggleTimeForm:(BOOL)open
{
	if (open)
	{
		NSNumber *addedHeight;
		if (self.timeEntry.durOnly)
		{
			addedHeight = [NSNumber numberWithInt:60];
		}
		else
		{
			addedHeight = [NSNumber numberWithInt:100];
		}
		NSDictionary *userInfo = [NSDictionary dictionaryWithObject:addedHeight forKey:@"height"];

		[[NSNotificationCenter defaultCenter] postNotificationName:kResizeEditForm
															object:nil
														  userInfo:userInfo];
	}
	[self.timeEditBox setHidden:!open];
	[self.timeTextBox setHidden:open];
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
	kopsik_set_time_entry_duration(ctx, [self.timeEntry.GUID UTF8String], value);
}

- (IBAction)projectSelectChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}

	NSAssert(self.timeEntry != nil, @"Expected time entry");

	[self.projectSelect.cell setCalculatedMaxWidth:0];

	NSString *key = [self.projectSelect stringValue];
	AutocompleteItem *autocomplete = [self.projectAutocompleteDataSource get:key];
	uint64_t task_id = 0;
	uint64_t project_id = 0;
	if (autocomplete != nil)
	{
		task_id = autocomplete.TaskID;
		project_id = autocomplete.ProjectID;
	}
	if ([key length] && project_id == 0)
	{
		return;
	}
	kopsik_set_time_entry_project(ctx, [self.timeEntry.GUID UTF8String], task_id, project_id, 0);
}

- (NSDateComponents *)parseTime:(NSTextField *)field current:(NSDateComponents *)component
{
	int hours = 0;
	int minutes = 0;

	if (!kopsik_parse_time([[field stringValue] UTF8String], &hours, &minutes))
	{
		if (field == self.startTime)
		{
			[field setStringValue:self.timeEntry.startTimeString];
		}
		else if (field == self.endTime)
		{
			[field setStringValue:self.timeEntry.endTimeString];
		}
		return component;
	}

	[component setHour:hours];
	[component setMinute:minutes];

	return component;
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
	[self applyStartTime];
}

- (IBAction)applyStartTime
{
	NSAssert(self.timeEntry != nil, @"Time entry expected");

	NSDate *startDate = [self.startDate dateValue];

	unsigned unitFlags = NSYearCalendarUnit | NSMonthCalendarUnit |  NSDayCalendarUnit;
	NSDateComponents *comps = [[NSCalendar currentCalendar] components:unitFlags fromDate:startDate];
	NSDate *combined = [[NSCalendar currentCalendar] dateFromComponents:comps];

	unitFlags = NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit;
	comps = [[NSCalendar currentCalendar] components:unitFlags fromDate:self.timeEntry.started];
	comps = [self parseTime:self.startTime current:comps];

	combined = [[NSCalendar currentCalendar] dateByAddingComponents:comps toDate:combined options:0];

	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	NSLocale *enUSPOSIXLocale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
	[dateFormatter setLocale:enUSPOSIXLocale];
	[dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ssZ"];
	NSString *iso8601String = [dateFormatter stringFromDate:combined];

	kopsik_set_time_entry_start_iso_8601(ctx, [self.timeEntry.GUID UTF8String], [iso8601String UTF8String]);
}

- (IBAction)endTimeChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}
	self.endTimeChanged = YES;
	[self applyEndTime];
}

- (IBAction)applyEndTime
{
	NSAssert(self.timeEntry != nil, @"Time entry expected");

	NSDate *startDate = [self.startDate dateValue];

	unsigned unitFlags = NSYearCalendarUnit | NSMonthCalendarUnit |  NSDayCalendarUnit;
	NSDateComponents *comps = [[NSCalendar currentCalendar] components:unitFlags fromDate:startDate];
	NSDate *combined = [[NSCalendar currentCalendar] dateFromComponents:comps];

	unitFlags = NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit;
	comps = [[NSCalendar currentCalendar] components:unitFlags fromDate:self.timeEntry.ended];
	comps = [self parseTime:self.endTime current:comps];
	combined = [[NSCalendar currentCalendar] dateByAddingComponents:comps toDate:combined options:0];

	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	NSLocale *enUSPOSIXLocale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
	[dateFormatter setLocale:enUSPOSIXLocale];
	[dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ssZ"];
	NSString *iso8601String = [dateFormatter stringFromDate:combined];

	kopsik_set_time_entry_end_iso_8601(ctx, [self.timeEntry.GUID UTF8String], [iso8601String UTF8String]);
}

- (IBAction)dateChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}

	[self applyStartTime];
	if (!self.endTime.isHidden)
	{
		[self applyEndTime];
	}
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

	_Bool value = [Utils stateToBool:[self.billableCheckbox state]];
	kopsik_set_time_entry_billable(ctx, [self.timeEntry.GUID UTF8String], value);
}

- (IBAction)descriptionComboboxChanged:(id)sender
{
	if (self.willTerminate)
	{
		return;
	}

	NSAssert(self.timeEntry != nil, @"Time entry expected");

	NSString *key = [self.descriptionCombobox stringValue];

	NSLog(@"descriptionComboboxChanged, stringValue = %@", key);

	[self.descriptionCombobox.cell setCalculatedMaxWidth:0];

	AutocompleteItem *autocomplete =
		[self.descriptionComboboxDataSource get:key];

	const char *GUID = [self.timeEntry.GUID UTF8String];

	if (!autocomplete)
	{
		kopsik_set_time_entry_description(ctx,
										  GUID,
										  [key UTF8String]);
		return;
	}

	if (![self.timeEntry.Description isEqualToString:key] &&
		!kopsik_set_time_entry_project(ctx,
									   GUID,
									   autocomplete.TaskID,
									   autocomplete.ProjectID,
									   0))
	{
		return;
	}

	self.descriptionCombobox.stringValue = autocomplete.Description;
	kopsik_set_time_entry_description(ctx, GUID, [autocomplete.Description UTF8String]);
}

- (IBAction)deleteButtonClicked:(id)sender
{
	NSAssert(self.timeEntry != nil, @"Time entry expected");

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

	kopsik_delete_time_entry(ctx, [self.timeEntry.GUID UTF8String]);
}

- (NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox
{
	if (self.clientSelect == aComboBox)
	{
		return [self.workspaceClientList count];
	}
	if (self.workspaceSelect == aComboBox)
	{
		return [self.workspaceList count];
	}
	NSAssert(false, @"Invalid combo box");
	return 0;
}

- (id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)row
{
	if (self.clientSelect == aComboBox)
	{
		if (row >= self.workspaceClientList.count)
		{
			return nil;
		}
		ViewItem *client = [self.workspaceClientList objectAtIndex:row];
		return client.Name;
	}
	if (self.workspaceSelect == aComboBox)
	{
		if (row >= self.workspaceList.count)
		{
			return nil;
		}
		ViewItem *workspace = [self.workspaceList objectAtIndex:row];
		return workspace.Name;
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
	if (self.workspaceSelect == aComboBox)
	{
		for (int i = 0; i < self.workspaceList.count; i++)
		{
			ViewItem *workspace = [self.workspaceList objectAtIndex:i];
			if ([workspace.Name isEqualToString:aString])
			{
				return i;
			}
		}
		return NSNotFound;
	}
	NSAssert(false, @"Invalid combo box");
	return NSNotFound;
}

- (void)controlTextDidEndEditing:(NSNotification *)aNotification
{
	if (self.willTerminate)
	{
		return;
	}

	if (![[aNotification object] isKindOfClass:[NSTokenField class]])
	{
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

	// Don't trigger combobox autocomplete when inside tags field
	if (![[aNotification object] isKindOfClass:[NSComboBox class]])
	{
		return;
	}

	NSCustomComboBox *comboBox = [aNotification object];
	NSString *filter = [comboBox stringValue];

	AutocompleteDataSource *dataSource = nil;
	if (comboBox == self.projectSelect)
	{
		dataSource = self.projectAutocompleteDataSource;
	}
	if (comboBox == self.descriptionCombobox)
	{
		dataSource = self.descriptionComboboxDataSource;
	}

	[dataSource setFilter:filter];

	if (!filter || ![filter length] || !dataSource.count)
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
	char str[kDurationStringLength];
	kopsik_format_duration_in_seconds_hhmmss(self.timeEntry.duration_in_seconds,
											 str,
											 kDurationStringLength);
	NSString *newValue = [NSString stringWithUTF8String:str];
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
	self.clientSelect.stringValue = @"";
	self.workspaceClientList = [self findWorkspaceClientList];
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

@end

// http://stackoverflow.com/questions/4499262/how-to-programmatically-open-an-nscomboboxs-list

@implementation NSComboBox (ExpansionAPI)

- (BOOL)isExpanded
{
	id ax = NSAccessibilityUnignoredDescendant(self);

	return [[ax accessibilityAttributeValue:
			 NSAccessibilityExpandedAttribute] boolValue];
}

- (void)setExpanded:(BOOL)expanded
{
	id ax = NSAccessibilityUnignoredDescendant(self);

	[ax accessibilitySetValue:[NSNumber numberWithBool:expanded]
				 forAttribute:NSAccessibilityExpandedAttribute];
}

@end
