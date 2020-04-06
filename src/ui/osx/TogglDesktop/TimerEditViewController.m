
//
//  TimerEditViewController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//
#import "TimerEditViewController.h"
#import "AutocompleteItem.h"
#import "LiteAutoCompleteDataSource.h"
#import "ConvertHexColor.h"
#import "TimeEntryViewItem.h"
#import "DisplayCommand.h"
#import "TogglDesktop-Swift.h"
#import "ProjectTextField.h"
#import "NSTextFieldDuration.h"
#import "NSHoverButton.h"
#import "NSBoxClickable.h"
#import "AutoCompleteInput.h"
#import "BetterFocusAutoCompleteInput.h"
#import "AutoCompleteTable.h"
#import <Carbon/Carbon.h>
#import "Utils.h"
#import "ClickableImageView.h"

typedef enum : NSUInteger
{
	DisplayModeManual,
	DisplayModeTimer,
	DisplayModeInput,
} DisplayMode;

static void *XXContext = &XXContext;
static NSString *kStateKey = @"state";

@interface TimerEditViewController () <ClickableImageViewDelegate>
@property (weak) IBOutlet NSTextFieldDuration *durationTextField;
@property (weak) IBOutlet NSHoverButton *startButton;
@property (weak) IBOutlet ProjectTextField *projectTextField;
@property (weak) IBOutlet AutoCompleteInput *descriptionLabel;
@property (weak) IBOutlet NSImageView *billableFlag;
@property (weak) IBOutlet ClickableImageView *tagFlag;
@property (weak) IBOutlet NSButton *addEntryBtn;
@property (weak) IBOutlet NSView *contentContainerView;
@property (weak) IBOutlet TimerContainerBox *autocompleteContainerView;
@property (weak) IBOutlet DotImageView *dotImageView;
@property (weak) IBOutlet NSLayoutConstraint *projectTextFieldLeading;
@property (weak) IBOutlet NSButton *cancelBtn;

- (IBAction)durationFieldChanged:(id)sender;
- (IBAction)autoCompleteChanged:(id)sender;
- (IBAction)addEntryBtnOnTap:(id)sender;

@property (strong, nonatomic) LiteAutoCompleteDataSource *liteAutocompleteDataSource;
@property (strong, nonatomic) TimeEntryViewItem *time_entry;
@property (strong, nonatomic) NSTimer *timer;
@property (assign, nonatomic) BOOL disableChange;
@property (assign, nonatomic) BOOL focusNotSet;
@property (assign, nonatomic) DisplayMode displayMode;

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
		self.liteAutocompleteDataSource = [[LiteAutoCompleteDataSource alloc] initWithNotificationName:kDisplayMinitimerAutocomplete];

		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimerState:)
													 name:kDisplayTimerState
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(focusTimer)
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
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(stop:)
													 name:kCommandStop
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startTimerNotification:)
													 name:kStartTimer
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(touchBarSettingChangedNotification:)
													 name:kTouchBarSettingChanged
												   object:nil];

		self.time_entry = [[TimeEntryViewItem alloc] init];

		self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0
													  target:self
													selector:@selector(timerFired:)
													userInfo:nil
													 repeats:YES];
		self.disableChange = NO;
	}

	return self;
}

- (void)viewDidLoad
{
	[super viewDidLoad];

	[self initCommon];
	[self initKVO];
}

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[self.startButton removeObserver:self forKeyPath:kStateKey];
}

- (void)initCommon
{
	self.displayMode = DisplayModeInput;
	self.projectTextField.isInTimerBar = YES;
	self.autoCompleteInput.displayMode = AutoCompleteDisplayModeFullscreen;
	self.liteAutocompleteDataSource.input = self.autoCompleteInput;
	[self.liteAutocompleteDataSource setFilter:@""];

	[self.startButton setHoverAlpha:0.75];
	self.autoCompleteInput.responderDelegate = self.autocompleteContainerView;

	self.descriptionLabel.delegate = self;
	self.tagFlag.delegate = self;
}

- (void)initKVO
{
	[self.startButton addObserver:self
					   forKeyPath:kStateKey
						  options:NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew
						  context:XXContext];
}

- (void)viewDidAppear
{
	[super viewDidAppear];
	NSRect viewFrameInWindowCoords = [self.view convertRect:[self.view bounds] toView:nil];

	[self.autoCompleteInput setPos:(int)viewFrameInWindowCoords.origin.y];
	[self.autoCompleteInput.autocompleteTableView setDelegate:self];
}

- (void)startDisplayLogin:(NSNotification *)notification
{
	[self clear];
}

- (void)focusTimer
{
	switch (self.displayMode)
	{
		case DisplayModeManual :
			[self.view.window makeFirstResponder:self.startButton];
			return;

		case DisplayModeInput :
			[self.autoCompleteInput.window makeFirstResponder:self.autoCompleteInput];
			break;
		case DisplayModeTimer :
			if (self.time_entry.isRunning)
			{
				[self.view.window makeFirstResponder:self.view];

				// Deselect all selected in TE list
				// Because we're focusing on the Timer bar since the TE is running
				[[NSNotificationCenter defaultCenter] postNotificationName:kDeselectAllTimeEntryList
																	object:nil];
			}
			break;
	}
}

- (void)startDisplayTimerState:(NSNotification *)notification
{
	[self displayTimerState:notification.object];
}

- (void)stop:(NSNotification *)notification
{
	self.descriptionLabel.editable = NO;
	[self clear];
	[self showDefaultTimer];
	[self focusTimer];
}

- (void)displayTimerState:(TimeEntryViewItem *)te
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	if (!te)
	{
		if ([self.autoCompleteInput currentEditor] != nil || self.descriptionLabel.currentEditor != nil)
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
		if (self.displayMode == DisplayModeManual)
		{
			toggl_set_settings_manual_mode(ctx, NO);
		}
		self.displayMode = DisplayModeTimer;
		self.startButton.toolTip = @"Stop";
		self.startButton.state = NSOnState;

		[self.durationTextField setDelegate:self];
		// Time entry has a description
		if (self.time_entry.descriptionName && [self.time_entry.descriptionName length] > 0)
		{
			self.descriptionLabel.stringValue = self.time_entry.descriptionName;
			self.descriptionLabel.toolTip = self.time_entry.descriptionName;
		}
		else
		{
			self.descriptionLabel.stringValue = @"";
			self.descriptionLabel.toolTip = @"(no description)";
		}
		[self.autoCompleteInput hide];
		[self.durationTextField setEditable:NO];
		[self.durationTextField setSelectable:NO];
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
		self.descriptionLabel.editable = NO;

        // Switch to timer mode in setting
        toggl_set_settings_manual_mode(ctx, false);
	}
	else
	{
		[self showDefaultTimer];
		self.descriptionLabel.editable = YES;
	}

	// Display project name
	if (self.time_entry.GUID != nil)
	{
		[self renderProjectLabelWithViewItem:self.time_entry];
	}

	// Display duration
	if (self.time_entry.duration != nil)
	{
		self.durationTextField.hidden = NO;
		self.durationTextField.stringValue = self.time_entry.duration;
	}
	else
	{
		self.durationTextField.hidden = YES;
		self.durationTextField.stringValue = @"";
	}

	// Hide
	self.cancelBtn.hidden = YES;

    if (@available(macOS 10.12.2, *))
    {
        if (self.time_entry != nil && self.time_entry.isRunning)
        {
            [[TouchBarService shared] updateRunningItem:self.time_entry];
        }
    }

}

- (void)showDefaultTimer
{
	if (!self.descriptionLabel.isEditable)
	{
		self.displayMode = DisplayModeInput;
	}

	// Start/stop button title and color depend on
	// whether time entry is running
	self.startButton.toolTip = @"Start";
	self.startButton.state = NSOffState;
	if ([self.autoCompleteInput currentEditor] == nil)
	{
		self.autoCompleteInput.stringValue = @"";
	}
	self.cancelBtn.hidden = YES;
	self.time_entry = [[TimeEntryViewItem alloc] init];
}

- (void)textFieldClicked:(id)sender
{
	if (self.time_entry.GUID == nil)
	{
		return;
	}

	[self.autoCompleteInput.window makeFirstResponder:self.autoCompleteInput];

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kResetEditPopoverSize
																object:nil];

	const char *focusField = kFocusedFieldNameDescription;
	if (sender == self.projectTextField)
	{
		focusField = kFocusedFieldNameProject;
	}
	else if (sender == self.durationTextField)
	{
		focusField = kFocusedFieldNameDuration;
	}
	else if (sender == self.tagFlag)
	{
		focusField = kFocusedFieldNameTag;
	}

	toggl_edit(ctx, [self.time_entry.GUID UTF8String], false, focusField);
}

- (void)clear
{
	self.durationTextField.hidden = YES;
	self.durationTextField.stringValue = @"";
	self.autoCompleteInput.stringValue = @"";
	[self.autoCompleteInput resetTable];
	[self.liteAutocompleteDataSource clearFilter];
	self.projectTextField.stringValue = @"";
	[self.contentContainerView setHidden:YES];
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
	self.durationTextField.hidden = newValue.length == 0;
}

- (IBAction)autoCompleteChanged:(id)sender
{
	if (self.disableChange == YES)
	{
		return;
	}

	self.time_entry.Description = [self.autoCompleteInput stringValue];
}

- (IBAction)addEntryBtnOnTap:(id)sender
{
	if (nil == self.time_entry || nil == self.time_entry.GUID)
	{
		[self addButtonClicked];
	}
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

	// Make descriptionLabel is editable and focus
	self.descriptionLabel.editable = YES;
	self.descriptionLabel.stringValue = self.time_entry.Description;
	self.displayMode = DisplayModeTimer;
	[self.view.window makeFirstResponder:self.descriptionLabel];

	// Display project name
	[self renderProjectLabelWithViewItem:self.time_entry];

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

		if (aNotification.object == self.descriptionLabel)
		{
			self.autoCompleteInput.stringValue = self.descriptionLabel.stringValue;
		}

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
	self.durationTextField.hidden = newValue.length == 0;

	// Update
	[[NSNotificationCenter defaultCenter] postNotificationName:@"TimerForRunningTimeEntryOnTicket" object:nil];
}

- (void)toggleTimer:(NSNotification *)notification
{
	[self.mainBox setHidden:NO];
	[self.manualBox setHidden:YES];

    // Don't focus if the timer is running
    if (![self.time_entry isRunning]) {
        [self.view.window makeFirstResponder:self.autoCompleteInput];
    }
}

- (void)toggleManual:(NSNotification *)notification
{
	[self.manualBox setHidden:NO];
	[self.mainBox setHidden:YES];
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
							 false,
							 0,
							 0
							 );

	[self clear];
	self.time_entry = [[TimeEntryViewItem alloc] init];
	NSString *GUID = [NSString stringWithUTF8String:guid];
	free(guid);

	toggl_edit(ctx, [GUID UTF8String], false, kFocusedFieldNameDescription);

	// Focus on Timeline if need
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
		[[NSNotificationCenter defaultCenter] postNotificationName:kDidAdddManualTimeNotification object:nil];
	});
}

#pragma AutocompleteTableView Delegate

- (BOOL)  tableView:(NSTableView *)aTableView
	shouldSelectRow:(NSInteger)rowIndex
{
	AutoCompleteTable *table = (AutoCompleteTable *)aTableView;

	[table setCurrentSelected:rowIndex next:YES];
	return YES;
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
	NSInteger row = [self.autoCompleteInput.autocompleteTableView selectedRow];

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
	NSRange tRange = [[self.descriptionLabel currentEditor] selectedRange];
	[[self.descriptionLabel currentEditor] setSelectedRange:NSMakeRange(tRange.length, 0)];
	[self.autoCompleteInput resetTable];
	[self.liteAutocompleteDataSource clearFilter];

	// Show cancel btn
	self.cancelBtn.hidden = NO;
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
	AutocompleteItem *item = nil;

	@synchronized(self)
	{
		item = [self.liteAutocompleteDataSource.filteredOrderedKeys objectAtIndex:row];
	}
	AutoCompleteCellType cellType = [AutoCompleteTableCell cellTypeFrom:item];

	// Big workspace
	if (cellType == AutoCompleteCellTypeWorkspace)
	{
		return self.autoCompleteInput.worksapceItemHeight;
	}

	// Other cells
	return self.autoCompleteInput.itemHeight;
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)fieldEditor doCommandBySelector:(SEL)commandSelector
{
	BOOL retval = NO;

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

		// Disable editable
		self.descriptionLabel.editable = NO;

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

	return retval;
}

- (void)setDisplayMode:(DisplayMode)displayMode
{
	_displayMode = displayMode;
	switch (displayMode)
	{
		case DisplayModeManual :
			self.manualBox.hidden = NO;
			self.mainBox.hidden = YES;
			break;
		case DisplayModeTimer :
			self.manualBox.hidden = YES;
			self.mainBox.hidden = NO;
			self.contentContainerView.hidden = NO;
			self.autocompleteContainerView.hidden = YES;
			break;
		case DisplayModeInput :
			self.contentContainerView.hidden = YES;
			self.autocompleteContainerView.hidden = NO;
			self.autoCompleteInput.hidden = NO;
			break;
	}
}

- (void)renderProjectLabelWithViewItem:(TimeEntryViewItem *)item
{
	NSString *text = item.ProjectAndTaskLabel;

	if (text != nil && text.length > 0)
	{
		NSColor *projectColor = [ConvertHexColor hexCodeToNSColor:item.ProjectColor];
		[self.dotImageView fillWith:projectColor];
		self.dotImageView.hidden = NO;
		[self.projectTextField setTitleWithTimeEntry:item];
		self.projectTextField.toolTip = text;
		self.projectTextFieldLeading.constant = 16.0;
	}
	else
	{
		self.dotImageView.hidden = YES;
		self.projectTextField.stringValue = @"";
		self.projectTextField.toolTip = nil;
		self.projectTextField.placeholderString = @"+ Add project";
		self.projectTextFieldLeading.constant = 0;
	}
}

- (void)startTimerNotification:(NSNotification *)notification
{
	[self startButtonClicked:self];
}

- (void)keyDown:(NSEvent *)event
{
	[super keyDown:event];

	if (self.time_entry.GUID != nil && self.time_entry.isRunning)
	{
		if ((event.keyCode == kVK_Return) || (event.keyCode == kVK_ANSI_KeypadEnter))
		{
			toggl_edit(ctx, [self.time_entry.GUID UTF8String], false, "");
		}
		else if (event.keyCode == kVK_Delete)
		{
            [[DesktopLibraryBridge shared] deleteTimeEntryItem:self.time_entry undoManager:self.undoManager];
		}
		else if (event.keyCode == kVK_Space)
		{
			[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kCommandStop
																		object:nil];
		}
	}
}

- (void)imageViewOnClick:(id)sender
{
	[self textFieldClicked:sender];
}

- (IBAction)cancelBtnOnTap:(id)sender
{
	NSString *description = self.time_entry.Description;

	self.time_entry = [[TimeEntryViewItem alloc] init];
	self.time_entry.Description = description;
	self.tagFlag.hidden = YES;
	self.billableFlag.hidden = YES;
	self.cancelBtn.hidden = YES;
	[self renderProjectLabelWithViewItem:nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if (context == XXContext)
	{
		[[NSNotificationCenter defaultCenter] postNotificationName:kStartButtonStateChange
															object:[NSNumber numberWithInteger:self.startButton.state]];
	}
	else
	{
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	}
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

@end
