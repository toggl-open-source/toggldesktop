//
//  IdleNotificationWindowController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 06/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "IdleNotificationWindowController.h"
#import "toggl_api.h"
#import "DisplayCommand.h"
#import "UIEvents.h"
#import "TogglDesktop-Swift.h"

@interface IdleNotificationWindowController ()

@property (weak) IBOutlet NSTextField *idleSinceTextField;
@property (weak) IBOutlet NSTextField *idleAmountTextField;
@property (weak) IBOutlet NSTextField *timeentryDescriptionTextField;
@property (weak) IBOutlet FlatButton *addIdleTimeButton;
@property (weak) IBOutlet NSButton *cancelButton;

- (IBAction)stopButtonClicked:(id)sender;
- (IBAction)ignoreButtonClicked:(id)sender;
- (IBAction)addIdleTimeAsNewTimeEntry:(id)sender;

@end

@implementation IdleNotificationWindowController

extern void *ctx;

- (void)awakeFromNib
{
	[super awakeFromNib];

	[self initCommon];
}

- (void)initCommon
{
	// Style buttons
	[self styleAddIdleButton];
	[self styleCancelButton];
}

- (void)styleAddIdleButton
{
	self.addIdleTimeButton.wantsLayer = YES;
	self.addIdleTimeButton.layer.borderWidth = 1;
	if (@available(macOS 10.13, *))
	{
		self.addIdleTimeButton.layer.borderColor = [NSColor colorNamed:@"upload-border-color"].CGColor;
		self.addIdleTimeButton.bgColor = [NSColor colorNamed:@"upload-background-color"];
	}
	else
	{
		self.addIdleTimeButton.layer.borderColor = [ConvertHexColor hexCodeToNSColor:@"#acacac"].CGColor;
		self.addIdleTimeButton.bgColor = NSColor.whiteColor;
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

	[self.window makeKeyAndOrderFront:nil];

	self.idleSinceTextField.stringValue = self.idleEvent.since;
	[self.idleSinceTextField setHidden:NO];

	self.idleAmountTextField.stringValue = self.idleEvent.duration;
	[self.idleAmountTextField setHidden:NO];

	self.timeentryDescriptionTextField.stringValue = self.idleEvent.timeEntryDescription;
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

@end
