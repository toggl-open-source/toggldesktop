
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

@interface TimerEditViewController ()
@property (weak) IBOutlet NSButton *addEntryBtn;
@property (strong, nonatomic) TimerViewController *timerViewController;
@property (assign, nonatomic) DisplayMode displayMode;

- (IBAction)addEntryBtnOnTap:(id)sender;

@end

@implementation TimerEditViewController

extern void *ctx;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
	self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
	if (self)
	{
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(toggleTimer:)
													 name:kToggleTimerMode
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(toggleManual:)
													 name:kToggleManualMode
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(touchBarSettingChangedNotification:)
													 name:kTouchBarSettingChanged
												   object:nil];
	}

	return self;
}

- (void)viewDidLoad
{
	[super viewDidLoad];

	[self initCommon];
}

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)initCommon
{
	self.displayMode = DisplayModeInput;
    [self embeddTimerViewController];
}

- (void)embeddTimerViewController {
    self.timerViewController = [[TimerViewController alloc] initWithNibName:@"TimerViewController" bundle:nil];
    NSView *timerView = [self.timerViewController view];
    timerView.translatesAutoresizingMaskIntoConstraints = NO;

    [self.mainBox.contentView addSubview:timerView];

    [timerView.leadingAnchor constraintEqualToAnchor:self.mainBox.contentView.leadingAnchor].active = YES;
    [timerView.trailingAnchor constraintEqualToAnchor:self.mainBox.contentView.trailingAnchor].active = YES;
    [timerView.topAnchor constraintEqualToAnchor:self.mainBox.contentView.topAnchor].active = YES;
    [timerView.bottomAnchor constraintEqualToAnchor:self.mainBox.contentView.bottomAnchor].active = YES;
}

- (void)focusTimer
{
	switch (self.displayMode)
	{
		case DisplayModeManual:
			return;
		case DisplayModeInput:
            [self.timerViewController focusDescriptionField];
			break;
		case DisplayModeTimer:
			break;
	}
}

- (void)startButtonClicked
{
	if (![self.manualBox isHidden])
	{
		[self addButtonClicked];
    }
    else
    {
        [self.timerViewController triggerStartStopAction];
    }
}

- (IBAction)addEntryBtnOnTap:(id)sender
{
    [self addButtonClicked];
}

- (void)toggleTimer:(NSNotification *)notification
{
	[self.mainBox setHidden:NO];
	[self.manualBox setHidden:YES];
    [self.timerViewController focusDescriptionField];
}

- (void)toggleManual:(NSNotification *)notification
{
	[self.manualBox setHidden:NO];
	[self.mainBox setHidden:YES];
}

- (void)addButtonClicked
{
	char *guid = toggl_start(ctx,
							 "",
							 "0",
							 0,
							 0,
							 0,
							 "",
							 false,
							 0,
							 0
							 );

	NSString *GUID = [NSString stringWithUTF8String:guid];
	free(guid);

	toggl_edit(ctx, [GUID UTF8String], false, kFocusedFieldNameDescription);

	// Focus on Timeline if need
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
		[[NSNotificationCenter defaultCenter] postNotificationName:kDidAdddManualTimeNotification object:nil];
	});
}

- (void)setDisplayMode:(DisplayMode)displayMode
{
	_displayMode = displayMode;
	switch (displayMode)
	{
		case DisplayModeManual:
			self.manualBox.hidden = NO;
			self.mainBox.hidden = YES;
			break;
		case DisplayModeTimer:
			self.manualBox.hidden = YES;
			self.mainBox.hidden = NO;
			break;
		case DisplayModeInput:
			break;
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
