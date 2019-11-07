//
//  IdleNotificationWindowController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 06/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "IdleNotificationWindowController.h"
#import "DisplayCommand.h"
#import "TogglDesktop-Swift.h"
#import "IdleEvent.h"
#import "UserNotificationCenter.h"

@interface IdleNotificationWindowController () <IdleNotificationTouchBarDelegate>

@property (weak) IBOutlet NSTextField *idleSinceTextField;
@property (weak) IBOutlet NSTextField *idleAmountTextField;
@property (weak) IBOutlet NSTextField *timeentryDescriptionTextField;
@property (weak) IBOutlet FlatButton *addIdleTimeButton;
@property (weak) IBOutlet NSButton *cancelButton;
@property (weak) IBOutlet FlatButton *discardAndContinueButton;
@property (weak) IBOutlet FlatButton *keepIdleTimeButton;
@property (assign, nonatomic) BOOL isWaiting;

- (IBAction)stopButtonClicked:(id)sender;
- (IBAction)ignoreButtonClicked:(id)sender;
- (IBAction)addIdleTimeAsNewTimeEntry:(id)sender;
- (IBAction)discardAndConitnueButtonClicked:(id)sender;

@end

@implementation IdleNotificationWindowController

extern void *ctx;

- (void)awakeFromNib
{
	[super awakeFromNib];
	[self.window setLevel:NSFloatingWindowLevel];
	[self initCommon];
}

- (void)initCommon
{
	// Style buttons
	[self styleTransparentButton:self.addIdleTimeButton];
	[self styleTransparentButton:self.discardAndContinueButton];
	[self styleTransparentButton:self.keepIdleTimeButton];
	[self styleCancelButton];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(windowDidBecomeActiveNotification)
												 name:NSWindowDidBecomeKeyNotification
											   object:nil];
}

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)windowDidBecomeActiveNotification
{
	if (self.isWaiting && ![UserNotificationCenter share].isDoNotDisturbEnabled)
	{
		[self.window makeKeyAndOrderFront:nil];
		[NSApp activateIgnoringOtherApps:YES];
		self.isWaiting = NO;
	}
}

- (void)styleTransparentButton:(FlatButton *)button
{
	button.wantsLayer = YES;
	button.layer.borderWidth = 1;
	if (@available(macOS 10.13, *))
	{
		button.layer.borderColor = [NSColor colorNamed:@"upload-border-color"].CGColor;
		button.bgColor = [NSColor colorNamed:@"upload-background-color"];
	}
	else
	{
		button.layer.borderColor = [ConvertHexColor hexCodeToNSColor:@"#acacac"].CGColor;
		button.bgColor = NSColor.whiteColor;
	}
}

- (void)styleCancelButton
{
	// Font
	NSFont *font = self.cancelButton.font;

	if (font == nil)
	{
		font = [NSFont systemFontOfSize:12 weight:NSFontWeightMedium];
	}

	// Color
	NSColor *color = [ConvertHexColor hexCodeToNSColor:@"#555555"];
	if (@available(macOS 10.13, *))
	{
		color = [NSColor colorNamed:@"grey-text-color"];
	}

	NSDictionary<NSAttributedStringKey, id> *attributes = @{ NSFontAttributeName: font,
															 NSForegroundColorAttributeName: color,
															 NSUnderlineStyleAttributeName: @(NSUnderlineStyleSingle), };
	NSAttributedString *underlineString = [[NSAttributedString alloc] initWithString:@"Cancel" attributes:attributes];
	self.cancelButton.attributedTitle = underlineString;
}

- (void)displayIdleEvent:(IdleEvent *)idleEvent
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.idleEvent = idleEvent;

	self.idleSinceTextField.stringValue = self.idleEvent.since;
	[self.idleSinceTextField setHidden:NO];

	self.idleAmountTextField.stringValue = self.idleEvent.duration;
	[self.idleAmountTextField setHidden:NO];

	self.timeentryDescriptionTextField.stringValue = self.idleEvent.timeEntryDescription;

	if ([[UserNotificationCenter share] isDoNotDisturbEnabled])
	{
		self.isWaiting = YES;
	}
	else
	{
		[self.window makeKeyAndOrderFront:nil];
		[NSApp activateIgnoringOtherApps:YES];
	}
}

- (IBAction)stopButtonClicked:(id)sender
{
	toggl_discard_time_at(ctx,
						  [self.idleEvent.guid UTF8String],
						  self.idleEvent.started,
						  NO);
	[self.window orderOut:nil];
}

- (IBAction)ignoreButtonClicked:(id)sender
{
	[self.window orderOut:nil];
}

- (IBAction)addIdleTimeAsNewTimeEntry:(id)sender
{
	toggl_discard_time_at(ctx,
						  [self.idleEvent.guid UTF8String],
						  self.idleEvent.started,
						  YES);
	[self.window orderOut:nil];

	// Open the main app to add Time Entry details
	DisplayCommand *cmd = [[DisplayCommand alloc] init];

	cmd.open = YES;
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayApp
																object:cmd];
}

- (IBAction)discardAndConitnueButtonClicked:(id)sender {
	toggl_discard_time_and_continue(ctx,
									[self.idleEvent.guid UTF8String],
									self.idleEvent.started);
	[self.window orderOut:nil];
}

- (NSTouchBar *)makeTouchBar {
	IdleNotificationTouchBar *touchbar = [[IdleNotificationTouchBar alloc] init];

	touchbar.delegate = self;
	return [touchbar makeTouchBar];
}

- (void)idleTouchBarDidTapFor:(enum Action)action {
	switch (action)
	{
		case ActionDiscard :
			[self stopButtonClicked:self];
			break;
		case ActionDiscardAndContinue :
			[self discardAndConitnueButtonClicked:self];
			break;
		case ActionKeep :
			[self ignoreButtonClicked:self];
			break;
		case ActionAdd :
			[self addIdleTimeAsNewTimeEntry:self];
			break;
		default :
			break;
	}
}

@end
