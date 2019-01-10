
//
//  TimerEditViewController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//
#import "TimerEditViewController.h"
#import "UIEvents.h"
#import "AutocompleteItem.h"
#import "LiteAutoCompleteDataSource.h"
#import "ConvertHexColor.h"
#import "NSComboBox_Expansion.h"
#import "TimeEntryViewItem.h"
#import "NSTextFieldClickable.h"
#import "NSCustomComboBoxCell.h"
#import "NSCustomComboBox.h"
#import "NSCustomTimerComboBox.h"
#import "DisplayCommand.h"
#import "TogglDesktop-Swift.h"

@interface TimerEditViewController ()
@property LiteAutoCompleteDataSource *liteAutocompleteDataSource;
@property TimeEntryViewItem *time_entry;
@property NSTimer *timer;
@property BOOL constraintsAdded;
@property BOOL disableChange;
@property BOOL focusNotSet;
@end

@implementation TimerEditViewController

extern void *ctx;

NSString *kTrackingColor = @"#d0d0d0";
NSString *kInactiveTimerColor = @"#999999";

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
	self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
	if (self)
	{
		self.focusNotSet = YES;
		self.liteAutocompleteDataSource = [[LiteAutoCompleteDataSource alloc] initWithNotificationName:kDisplayMinitimerAutocomplete];

		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimerState:)
													 name:kDisplayTimerState
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimeEntryList:)
													 name:kDisplayTimeEntryList
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimeEntryEditor:)
													 name:kDisplayTimeEntryEditor
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(focusTimer:)
													 name:kFocusTimer
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(toggleTimer:)
													 name:kToggleTimerMode
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(toggleManual:)
													 name:kToggleManualMode
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayLogin:)
													 name:kDisplayLogin
												   object:nil];


		self.time_entry = [[TimeEntryViewItem alloc] init];

		self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0
													  target:self
													selector:@selector(timerFired:)
													userInfo:nil
													 repeats:YES];
		self.constraintsAdded = NO;
		self.disableChange = NO;
	}

	return self;
}

- (void)viewDidLoad
{
	self.liteAutocompleteDataSource.input = self.autoCompleteInput;
	[self.liteAutocompleteDataSource setFilter:@""];

	NSFont *descriptionFont = [NSFont fontWithName:@"Lucida Grande" size:13.0];
	NSFont *durationFont = [NSFont fontWithName:@"Lucida Grande" size:14.0];
	NSColor *color = [ConvertHexColor hexCodeToNSColor:kTrackingColor];
	NSDictionary *descriptionDictionary = @{
			NSFontAttributeName : descriptionFont,
			NSForegroundColorAttributeName : color
	};
	NSDictionary *durationDictionary = @{
			NSFontAttributeName : durationFont,
			NSForegroundColorAttributeName : color
	};

	NSAttributedString *descriptionLightString =
		[[NSAttributedString alloc] initWithString:NSLocalizedString(@"What are you working on?", nil)
										attributes:descriptionDictionary];

	NSAttributedString *durationLightString =
		[[NSAttributedString alloc] initWithString:@"00:00:00"
										attributes:durationDictionary];

	[[self.durationTextField cell] setPlaceholderAttributedString:durationLightString];
	[[self.descriptionLabel cell] setPlaceholderAttributedString:descriptionLightString];
	[[self.autoCompleteInput cell] setPlaceholderAttributedString:descriptionLightString];

	[self.startButton setHoverAlpha:0.75];

	int osxMode = [[[NSUserDefaults standardUserDefaults] stringForKey:@"AppleAquaColorVariant"] intValue];
	int trail = 40;
	if (osxMode == 6)
	{
		trail = 60;
	}
	self.descriptionTrailing.constant = trail;

	[self.autoCompleteInput.autocompleteTableView setTarget:self];
	[self.autoCompleteInput.autocompleteTableView setAction:@selector(performClick:)];
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

- (void)viewDidAppear
{
	NSRect viewFrameInWindowCoords = [self.view convertRect:[self.view bounds] toView:nil];

	[self.autoCompleteInput setPos:(int)viewFrameInWindowCoords.origin.y];
	[self.autoCompleteInput.autocompleteTableView setDelegate:self];
}

- (void)startDisplayLogin:(NSNotification *)notification
{
	[self clear];
}

- (void)focusTimer:(NSNotification *)notification
{
	if (self.time_entry.duration < 0 || ![self.manualBox isHidden])
	{
		[self.view.window makeFirstResponder:self.startButton];
	}
	else
	{
		[self.autoCompleteInput.window makeFirstResponder:self.autoCompleteInput];
	}
}

- (void)startDisplayTimeEntryList:(NSNotification *)notification
{
	[self displayTimeEntryList:notification.object];
}

- (void)displayTimeEntryList:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	if (cmd.open && self.time_entry && self.time_entry.duration_in_seconds >= 0
		&& self.focusNotSet)
	{
		[self.autoCompleteInput.window makeFirstResponder:self.autoCompleteInput];
		self.focusNotSet = NO;
	}
}

- (void)startDisplayTimerState:(NSNotification *)notification
{
	[self displayTimerState:notification.object];
}

- (void)displayTimerState:(TimeEntryViewItem *)te
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	if (!te)
	{
		if ([self.autoCompleteInput currentEditor] != nil)
		{
			return;
		}
		te = [[TimeEntryViewItem alloc] init];
	}
	self.time_entry = te;

	// Description and duration cannot be edited
	// while time entry is running
	if (self.time_entry.duration_in_seconds < 0)
	{
		// Start/stop button title and color depend on
		// whether time entry is running
		self.startButton.toolTip = @"Stop";
		[self.startButton setImage:[NSImage imageNamed:@"stop_button.pdf"]];
		toggl_set_settings_manual_mode(ctx, NO);

		[self.durationTextField setDelegate:self];
		// Time entry has a description
		if (self.time_entry.Description && [self.time_entry.Description length] > 0)
		{
			self.descriptionLabel.stringValue = self.time_entry.Description;
			self.descriptionLabel.toolTip = self.time_entry.Description;
		}
		else
		{
			self.descriptionLabel.stringValue = @"(no description)";
			self.descriptionLabel.toolTip = @"(no description)";
		}
		[self.autoCompleteInput hide];
		[self.descriptionLabel setHidden:NO];
		[self.durationTextField setEditable:NO];
		[self.durationTextField setSelectable:NO];
		[self.durationTextField setHidden:NO];
		[self.descriptionLabel setTextColor:[ConvertHexColor hexCodeToNSColor:kTrackingColor]];

		[self.durationTextField setTextColor:[ConvertHexColor hexCodeToNSColor:kTrackingColor]];
		[self.billableFlag setHidden:!self.time_entry.billable];

		// Time entry tags icon
		if ([self.time_entry.tags count] && [[self.time_entry.tags componentsJoinedByString:@", "] length])
		{
			[self.tagFlag setHidden:NO];
			self.tagFlag.toolTip = [self.time_entry.tags componentsJoinedByString:@", "];
		}
		else
		{
			[self.tagFlag setHidden:YES];
			self.tagFlag.toolTip = nil;
		}

		self.durationTextField.toolTip = [NSString stringWithFormat:@"Started: %@", self.time_entry.startTimeString];
	}
	else
	{
		[self showDefaultTimer];
	}

	[self checkProjectConstraints];

	// Display project name
	if (self.time_entry.ProjectAndTaskLabel != nil)
	{
		[self.projectTextField setAttributedStringValue:[self setProjectClientLabel:self.time_entry]];
		self.projectTextField.toolTip = self.time_entry.ProjectAndTaskLabel;
	}
	else
	{
		self.projectTextField.stringValue = @"";
		self.projectTextField.toolTip = nil;
	}
	self.projectTextField.backgroundColor =
		[ConvertHexColor hexCodeToNSColor:self.time_entry.ProjectColor];

	// Display duration
	if (self.time_entry.duration != nil)
	{
		self.durationTextField.stringValue = self.time_entry.duration;
	}
	else
	{
		self.durationTextField.stringValue = @"";
	}
}

- (void)startDisplayTimeEntryEditor:(NSNotification *)notification
{
	[self displayTimeEntryEditor:notification.object];
}

- (void)displayTimeEntryEditor:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSLog(@"TimeEntryListViewController displayTimeEntryEditor, thread %@", [NSThread currentThread]);
}

- (void)checkProjectConstraints
{
	// If a project is assigned, then project name
	// is visible.
	if (self.time_entry.ProjectID || self.time_entry.ProjectGUID || [self.time_entry.ProjectAndTaskLabel length])
	{
		if (![self.projectComboConstraint count])
		{
			[self createConstraints];
		}
		if (!self.constraintsAdded)
		{
			[self.view addConstraints:self.projectComboConstraint];
			[self.view addConstraints:self.projectLabelConstraint];
			self.constraintsAdded = YES;
		}

		[self.projectTextField setHidden:NO];
	}
	else
	{
		[self.projectTextField setHidden:YES];
		if (self.constraintsAdded)
		{
			[self.view removeConstraints:self.projectComboConstraint];
			[self.view removeConstraints:self.projectLabelConstraint];
			self.constraintsAdded = NO;
		}
	}
}

- (void)showDefaultTimer
{
	// Start/stop button title and color depend on
	// whether time entry is running
	self.startButton.toolTip = @"Start";
	[self.startButton setImage:[NSImage imageNamed:@"start_button.pdf"]];
	if ([self.autoCompleteInput currentEditor] == nil)
	{
		self.autoCompleteInput.stringValue = @"";
	}
	[self.autoCompleteInput setHidden:NO];
	[self.descriptionLabel setHidden:YES];
	[self.durationTextField setEditable:YES];
	[self.durationTextField setSelectable:YES];
	[self.durationTextField setHidden:YES];
	[self.descriptionLabel setTextColor:[ConvertHexColor hexCodeToNSColor:kInactiveTimerColor]];

	[self.durationTextField setTextColor:[ConvertHexColor hexCodeToNSColor:kInactiveTimerColor]];
	[self.tagFlag setHidden:YES];
	[self.billableFlag setHidden:YES];
	[self.view.window makeFirstResponder:self.autoCompleteInput];

	self.time_entry = [[TimeEntryViewItem alloc] init];
}

- (NSMutableAttributedString *)setProjectClientLabel:(TimeEntryViewItem *)view_item
{
	NSMutableAttributedString *clientName = [[NSMutableAttributedString alloc] initWithString:view_item.ClientLabel];
	NSColor *color = [ConvertHexColor hexCodeToNSColor:@"#666666"];

	[clientName setAttributes:
	 @{
		 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
		 NSForegroundColorAttributeName:color
	 }
						range:NSMakeRange(0, [clientName length])];
	NSMutableAttributedString *string;
	if (view_item.TaskID != 0)
	{
		string = [[NSMutableAttributedString alloc] initWithString:[view_item.TaskLabel stringByAppendingString:@". "]];

		[string setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
			 NSForegroundColorAttributeName:color
		 }
						range:NSMakeRange(0, [string length])];

		NSMutableAttributedString *projectName = [[NSMutableAttributedString alloc] initWithString:[view_item.ProjectLabel stringByAppendingString:@" "]];

		[string appendAttributedString:projectName];
	}
	else
	{
		string = [[NSMutableAttributedString alloc] initWithString:[view_item.ProjectLabel stringByAppendingString:@" "]];
	}

	[string appendAttributedString:clientName];
	return string;
}

- (void)textFieldClicked:(id)sender
{
	[self.autoCompleteInput.window makeFirstResponder:self.autoCompleteInput];

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kResetEditPopoverSize
																object:nil];

	if (nil == self.time_entry || nil == self.time_entry.GUID)
	{
		if (sender == self.addEntryLabel)
		{
			[self addButtonClicked];
		}
		return;
	}

	const char *focusField = kFocusedFieldNameDescription;
	if (sender == self.projectTextField)
	{
		focusField = kFocusedFieldNameProject;
	}
	else if (sender == self.durationTextField)
	{
		focusField = kFocusedFieldNameDuration;
	}

	toggl_edit(ctx, [self.time_entry.GUID UTF8String], false, focusField);
}

- (void)createConstraints
{
	NSDictionary *viewsDict = NSDictionaryOfVariableBindings(_autoCompleteInput, _projectTextField);

	self.projectComboConstraint = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[_autoCompleteInput]-2@1000-[_projectTextField]"
																		  options:0
																		  metrics:nil
																			views:viewsDict];

	NSDictionary *viewsDict_ = NSDictionaryOfVariableBindings(_descriptionLabel, _projectTextField);
	self.projectLabelConstraint = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[_descriptionLabel]-6@1000-[_projectTextField]"
																		  options:0
																		  metrics:nil
																			views:viewsDict_];
}

- (void)clear
{
	self.durationTextField.stringValue = @"";
	self.autoCompleteInput.stringValue = @"";
	[self.autoCompleteInput resetTable];
	[self.liteAutocompleteDataSource clearFilter];
	self.projectTextField.stringValue = @"";
	[self.projectTextField setHidden:YES];
}

- (IBAction)startButtonClicked:(id)sender
{
	if (![self.manualBox isHidden])
	{
		[self addButtonClicked];
		return;
	}
	if (self.time_entry.duration_in_seconds < 0)
	{
		[self clear];
		[self showDefaultTimer];
		[self.projectTextField setHidden:YES];
		if (self.constraintsAdded)
		{
			[self.view removeConstraints:self.projectComboConstraint];
			[self.view removeConstraints:self.projectLabelConstraint];
			self.constraintsAdded = NO;
		}
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kCommandStop
																	object:nil];
		return;
	}

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kForceCloseEditPopover
																object:nil];

	self.disableChange = YES;
	// resign current firstResponder
	[self.durationTextField.window makeFirstResponder:[self.durationTextField superview]];
	self.disableChange = NO;
	self.time_entry.duration = self.durationTextField.stringValue;
	self.time_entry.Description = self.autoCompleteInput.stringValue;
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kCommandNew
																object:self.time_entry];

	// Reset autocomplete
	[self.autoCompleteInput resetTable];
	[self.liteAutocompleteDataSource clearFilter];
}

- (IBAction)durationFieldChanged:(id)sender
{
	if (![self.durationTextField.stringValue length])
	{
		return;
	}

	// Parse text into seconds
	const char *duration_string = [self.durationTextField.stringValue UTF8String];
	int64_t seconds = toggl_parse_duration_string_into_seconds(duration_string);

	// Format seconds as text again
	char *str = toggl_format_tracking_time_duration(seconds);
	NSString *newValue = [NSString stringWithUTF8String:str];
	free(str);
	[self.durationTextField setStringValue:newValue];
}

- (IBAction)autoCompleteChanged:(id)sender
{
	if (self.disableChange == YES)
	{
		return;
	}

	self.time_entry.Description = [self.autoCompleteInput stringValue];
}

- (void)fillEntryFromAutoComplete:(AutocompleteItem *)item
{
	// User has selected a autocomplete item.
	// It could be a time entry, a task or a project.
	self.time_entry.WorkspaceID = item.WorkspaceID;
	self.time_entry.ProjectID = item.ProjectID;
	self.time_entry.TaskID = item.TaskID;
	self.time_entry.ProjectAndTaskLabel = item.ProjectAndTaskLabel;
	self.time_entry.TaskLabel = item.TaskLabel;
	self.time_entry.ProjectLabel = item.ProjectLabel;
	self.time_entry.ClientLabel = item.ClientLabel;
	self.time_entry.ProjectColor = item.ProjectColor;
	self.time_entry.tags = [[NSMutableArray alloc] initWithArray:item.tags copyItems:YES];
	if (item.Description != nil)
	{
		self.time_entry.Description = ([item.Description length] != 0) ? item.Description : item.TaskLabel;
	}

	self.autoCompleteInput.stringValue = self.time_entry.Description;
	if (item.ProjectID > 0)
	{
		[self.projectTextField setAttributedStringValue:[self setProjectClientLabel:self.time_entry]];
		self.projectTextField.toolTip = self.time_entry.ProjectAndTaskLabel;
	}
	else
	{
		self.projectTextField.stringValue = @"";
		self.projectTextField.toolTip = nil;
	}
	[self checkProjectConstraints];

	self.time_entry.billable = item.Billable;
}

- (void)controlTextDidChange:(NSNotification *)aNotification
{
	if ([[aNotification object] isKindOfClass:[NSTextFieldDuration class]])
	{
		return;
	}

	if ([[aNotification object] isKindOfClass:[AutoCompleteInput class]])
	{
		AutoCompleteInput *field = [aNotification object];
		[self.liteAutocompleteDataSource setFilter:[field stringValue]];
		[field.autocompleteTableView resetSelected];
		// NSLog(@"Filter: %@", [field stringValue]);
		return;
	}
}

- (void)timerFired:(NSTimer *)timer
{
	if (self.time_entry == nil || self.time_entry.duration_in_seconds >= 0)
	{
		return;
	}

	char *str = toggl_format_tracking_time_duration(self.time_entry.duration_in_seconds);
	NSString *newValue = [NSString stringWithUTF8String:str];
	free(str);

	[self.durationTextField setStringValue:newValue];
}

- (void)toggleTimer:(NSNotification *)notification
{
	[self.mainBox setHidden:NO];
	[self.manualBox setHidden:YES];
}

- (void)toggleManual:(NSNotification *)notification
{
	[self.manualBox setHidden:NO];
	[self.mainBox setHidden:YES];
	[self.view.window makeFirstResponder:self.startButton];
}

- (void)addButtonClicked
{
	const char *tag_list = [[self.time_entry.tags componentsJoinedByString:@"\t"] UTF8String];
	char *guid = toggl_start(ctx,
							 [self.autoCompleteInput.stringValue UTF8String],
							 "0",
							 self.time_entry.TaskID,
							 self.time_entry.ProjectID,
							 0,
							 tag_list,
							 false);

	[self clear];
	self.time_entry = [[TimeEntryViewItem alloc] init];
	NSString *GUID = [NSString stringWithUTF8String:guid];
	free(guid);

	toggl_edit(ctx, [GUID UTF8String], false, kFocusedFieldNameDescription);
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
	if (row < 0 || row >= self.liteAutocompleteDataSource.filteredOrderedKeys.count)
	{
		return nil;
	}

	AutocompleteItem *item = nil;
	AutoCompleteTable *tb = (AutoCompleteTable *)tableView;

	@synchronized(self)
	{
		item = [self.liteAutocompleteDataSource.filteredOrderedKeys objectAtIndex:row];
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

- (IBAction)performClick:(id)sender
{
	NSInteger row = [self.autoCompleteInput.autocompleteTableView clickedRow];

	if (row < 0)
	{
		return;
	}

	AutocompleteItem *item = [self.liteAutocompleteDataSource itemAtIndex:row];
	// Category clicked
	if (item == nil || item.Type < 0)
	{
		return;
	}
	[self fillEntryFromAutoComplete:item];
	[self.autoCompleteInput.window makeFirstResponder:self.autoCompleteInput];
	NSRange tRange = [[self.autoCompleteInput currentEditor] selectedRange];
	[[self.autoCompleteInput currentEditor] setSelectedRange:NSMakeRange(tRange.length, 0)];
	[self.autoCompleteInput resetTable];
	[self.liteAutocompleteDataSource clearFilter];
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)fieldEditor doCommandBySelector:(SEL)commandSelector
{
	BOOL retval = NO;

	if ([self.autoCompleteInput currentEditor] != nil)
	{
		if (commandSelector == @selector(moveDown:))
		{
			[self.autoCompleteInput.autocompleteTableView nextItem];
		}
		if (commandSelector == @selector(moveUp:))
		{
			[self.autoCompleteInput.autocompleteTableView previousItem];
		}
		if (commandSelector == @selector(insertTab:))
		{
			// Set data according to selected item
			if (self.autoCompleteInput.autocompleteTableView.lastSelected >= 0)
			{
				AutocompleteItem *item = [self.liteAutocompleteDataSource itemAtIndex:self.autoCompleteInput.autocompleteTableView.lastSelected];
				if (item == nil)
				{
					return retval;
				}
				[self fillEntryFromAutoComplete:item];
			}
			[self.autoCompleteInput resetTable];
			[self.liteAutocompleteDataSource clearFilter];
		}
		if (commandSelector == @selector(insertNewline:))
		{
			// avoid firing default Enter actions
			retval = YES;

			// Set data according to selected item
			if (self.autoCompleteInput.autocompleteTableView.lastSelected >= 0)
			{
				AutocompleteItem *item = [self.liteAutocompleteDataSource itemAtIndex:self.autoCompleteInput.autocompleteTableView.lastSelected];
				if (item == nil)
				{
					return retval;
				}
				[self fillEntryFromAutoComplete:item];
			}

			// Start entry
			[self startButtonClicked:nil];

			[self.autoCompleteInput resetTable];
			[self.liteAutocompleteDataSource clearFilter];
		}
	}
	// NSLog(@"Selector = %@", NSStringFromSelector( commandSelector ) );
	return retval;
}

@end
