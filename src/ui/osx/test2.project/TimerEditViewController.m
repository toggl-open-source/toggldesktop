
//
//  TimerEditViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//
#import "TimerEditViewController.h"
#import "UIEvents.h"
#import "AutocompleteItem.h"
#import "AutocompleteDataSource.h"
#import "ConvertHexColor.h"
#import "NSComboBox_Expansion.h"
#import "TimeEntryViewItem.h"
#import "NSTextFieldClickable.h"
#import "NSCustomComboBoxCell.h"
#import "NSCustomComboBox.h"
#import "DisplayCommand.h"

@interface TimerEditViewController ()
@property AutocompleteDataSource *autocompleteDataSource;
@property TimeEntryViewItem *time_entry;
@property NSTimer *timer;
@property BOOL constraintsAdded;
@end

@implementation TimerEditViewController

extern int kDurationStringLength;
extern void *ctx;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
	self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
	if (self)
	{
		self.autocompleteDataSource = [[AutocompleteDataSource alloc] init];

		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimerState:)
													 name:kDisplayTimerState
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimeEntryList:)
													 name:kDisplayTimeEntryList
												   object:nil];

		self.time_entry = [[TimeEntryViewItem alloc] init];

		self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0
													  target:self
													selector:@selector(timerFired:)
													userInfo:nil
													 repeats:YES];
		self.constraintsAdded = NO;
	}

	return self;
}

- (void)viewDidLoad
{
	self.autocompleteDataSource.combobox = self.descriptionComboBox;

	[self.autocompleteDataSource setFilter:@""];

	NSDictionary *lightDict = [NSDictionary dictionaryWithObject:[ConvertHexColor hexCodeToNSColor:@"#d0d0d0"]
														  forKey:NSForegroundColorAttributeName];
	NSAttributedString *lightString = [[NSAttributedString alloc] initWithString:@"What are you doing?"
																	  attributes:lightDict];
	[[self.descriptionLabel cell] setPlaceholderAttributedString:lightString];
}

- (void)loadView
{
	[super loadView];
	[self viewDidLoad];
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

	if (cmd.open && self.time_entry && self.time_entry.duration_in_seconds >= 0)
	{
		[self.descriptionComboBox becomeFirstResponder];
	}
}

- (void)startDisplayTimerState:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayTimerState:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayTimerState:(TimeEntryViewItem *)te
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	if (!te)
	{
		te = [[TimeEntryViewItem alloc] init];
	}
	self.time_entry = te;

	// Start/stop button title and color depend on
	// whether time entry is running
	if (self.time_entry.duration_in_seconds < 0)
	{
		[self.startButton setImage:[NSImage imageNamed:@"icon-stop-red@2x.png"]];
		self.startButton.toolTip = @"Stop";
		self.startButtonBox.borderColor = [ConvertHexColor hexCodeToNSColor:@"#ec0000"];
		self.startButtonBox.fillColor = [ConvertHexColor hexCodeToNSColor:@"#ec0000"];
	}
	else
	{
		self.startButton.toolTip = @"Start";
		[self.startButton setImage:[NSImage imageNamed:@"icon-start-green@2x.png"]];
		self.startButtonBox.borderColor = [ConvertHexColor hexCodeToNSColor:@"#4bc800"];
		self.startButtonBox.fillColor = [ConvertHexColor hexCodeToNSColor:@"#4bc800"];
	}

	// Description and duration cannot be edited
	// while time entry is running
	if (self.time_entry.duration_in_seconds < 0)
	{
		[self.durationTextField setDelegate:self];
		self.descriptionLabel.stringValue = self.time_entry.Description;
		self.descriptionLabel.toolTip = self.time_entry.Description;
		[self.descriptionComboBox setHidden:YES];
		[self.descriptionLabel setHidden:NO];
		[self.durationTextField setEditable:NO];
		[self.durationTextField setSelectable:NO];
		[self.durationTextField setTextColor:[ConvertHexColor hexCodeToNSColor:@"#d0d0d0"]];
		[self.descriptionLabel setTextColor:[ConvertHexColor hexCodeToNSColor:@"#d0d0d0"]];
	}
	else
	{
		[self.descriptionComboBox setHidden:NO];
		[self.descriptionLabel setHidden:YES];
		[self.durationTextField setEditable:YES];
		[self.durationTextField setSelectable:YES];
		[self.durationTextField setDelegate:self.durationTextField];
		[self.durationTextField setTextColor:[ConvertHexColor hexCodeToNSColor:@"#999999"]];
		[self.descriptionLabel setTextColor:[ConvertHexColor hexCodeToNSColor:@"#999999"]];
	}

	[self checkProjectConstraints];

	// Display project name
	if (self.time_entry.ProjectAndTaskLabel != nil)
	{
		self.projectTextField.stringValue = self.time_entry.ProjectAndTaskLabel;
		self.projectTextField.toolTip = self.time_entry.ProjectAndTaskLabel;
		[self setClient:self.projectTextField];
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

- (void)setClient:(NSTextField *)inTextField
{
	NSArray *chunks = [[inTextField stringValue] componentsSeparatedByString:@"."];

	if ([chunks count] == 1)
	{
		return;
	}
	NSString *client;
	NSString *project;
	if (self.time_entry.duration_in_seconds < 0)
	{
		client = chunks[1];
		project = chunks[0];
	}
	else
	{
		client = chunks[0];
		project = [chunks[1] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
	}

	NSMutableAttributedString *clientName = [[NSMutableAttributedString alloc] initWithString:client];
	[clientName setAttributes:
	 @{
		 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
		 NSForegroundColorAttributeName:[NSColor disabledControlTextColor]
	 }
						range:NSMakeRange(0, [clientName length])];
	NSMutableAttributedString *string = [[NSMutableAttributedString alloc] initWithString:[project stringByAppendingString:@" "]];
	[string appendAttributedString:clientName];
	// set the attributed string to the NSTextField
	[inTextField setAttributedStringValue:string];
}

- (void)textFieldClicked:(id)sender
{
	if (nil == self.time_entry)
	{
		return;
	}
	if (nil == self.time_entry.GUID)
	{
		return;
	}

	if (sender == self.durationTextField)
	{
		kopsik_edit(ctx, [self.time_entry.GUID UTF8String], false, kFocusedFieldNameDuration);
		return;
	}

	if (sender == self.descriptionLabel)
	{
		kopsik_edit(ctx, [self.time_entry.GUID UTF8String], false, kFocusedFieldNameDescription);
		return;
	}
  
  if (sender == self.projectTextField) {
    kopsik_edit(ctx, [self.time_entry.GUID UTF8String], false, kFocusedFieldNameProject);
    return;
  }
}

- (void)createConstraints
{
	NSDictionary *viewsDict = NSDictionaryOfVariableBindings(_descriptionComboBox, _projectTextField);

	self.projectComboConstraint = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[_descriptionComboBox]-2@1000-[_projectTextField]"
																		  options:0
																		  metrics:nil
																			views:viewsDict];

	NSDictionary *viewsDict_ = NSDictionaryOfVariableBindings(_descriptionLabel, _projectTextField);
	self.projectLabelConstraint = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[_descriptionLabel]-0@1000-[_projectTextField]"
																		  options:0
																		  metrics:nil
																			views:viewsDict_];
}

- (void)clear
{
	self.durationTextField.stringValue = @"";
	self.descriptionComboBox.stringValue = @"";
	self.projectTextField.stringValue = @"";
	[self.projectTextField setHidden:YES];
}

- (IBAction)startButtonClicked:(id)sender
{
	if (self.time_entry.duration_in_seconds < 0)
	{
		[self clear];
		[[NSNotificationCenter defaultCenter] postNotificationName:kCommandStop
															object:nil];
		return;
	}

	// resign current FirstResponder
	[self.durationTextField.window makeFirstResponder:[self.durationTextField superview]];
	self.time_entry.Duration = self.durationTextField.stringValue;
	self.time_entry.Description = self.descriptionComboBox.stringValue;
	[[NSNotificationCenter defaultCenter] postNotificationName:kCommandNew
														object:self.time_entry];

	// Reset autocomplete filter
	[self.autocompleteDataSource setFilter:@""];

	if (self.time_entry.duration_in_seconds >= 0)
	{
		[self clear];
		self.time_entry = [[TimeEntryViewItem alloc] init];
	}
}

- (IBAction)durationFieldChanged:(id)sender
{
	if (![self.durationTextField.stringValue length])
	{
		return;
	}

	// Parse text into seconds
	const char *duration_string = [self.durationTextField.stringValue UTF8String];
	int64_t seconds = kopsik_parse_duration_string_into_seconds(duration_string);

	// Format seconds as text again
	char str[kDurationStringLength];
	kopsik_format_duration_in_seconds_hhmmss(seconds,
											 str,
											 kDurationStringLength);
	NSString *newValue = [NSString stringWithUTF8String:str];
	[self.durationTextField setStringValue:newValue];
}

- (IBAction)descriptionComboBoxChanged:(id)sender
{
	NSString *key = [self.descriptionComboBox stringValue];
	AutocompleteItem *item = [self.autocompleteDataSource get:key];

	// User has entered free text
	if (item == nil)
	{
		self.time_entry.Description = [self.descriptionComboBox stringValue];
		return;
	}

	// User has selected a autocomplete item.
	// It could be a time entry, a task or a project.
	self.time_entry.ProjectID = item.ProjectID;
	self.time_entry.TaskID = item.TaskID;
	self.time_entry.ProjectAndTaskLabel = item.ProjectAndTaskLabel;
	self.time_entry.ProjectColor = item.ProjectColor;
	self.time_entry.Description = item.Description;

	self.descriptionComboBox.stringValue = self.time_entry.Description;
	if (item.ProjectID)
	{
		self.projectTextField.stringValue = self.time_entry.ProjectAndTaskLabel;
		self.projectTextField.toolTip = self.time_entry.ProjectAndTaskLabel;
		[self setClient:self.projectTextField];
	}
	[self checkProjectConstraints];
}

- (void)controlTextDidChange:(NSNotification *)aNotification
{
	NSComboBox *box = [aNotification object];
	NSString *filter = [box stringValue];

	[self.autocompleteDataSource setFilter:filter];

	// Hide dropdown if filter is empty or nothing was found
	if (!filter || ![filter length] || !self.autocompleteDataSource.count)
	{
		if ([box isExpanded] == YES)
		{
			[box setExpanded:NO];
		}
		return;
	}

	if ([box isExpanded] == NO)
	{
		[box setExpanded:YES];
	}
}

- (void)timerFired:(NSTimer *)timer
{
	if (self.time_entry == nil || self.time_entry.duration_in_seconds >= 0)
	{
		return;
	}
	char str[kDurationStringLength];
	kopsik_format_duration_in_seconds_hhmmss(self.time_entry.duration_in_seconds,
											 str,
											 kDurationStringLength);
	NSString *newValue = [NSString stringWithUTF8String:str];
	[self.durationTextField setStringValue:newValue];
}

@end
