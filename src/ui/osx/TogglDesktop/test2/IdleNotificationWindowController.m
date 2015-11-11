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

@implementation IdleNotificationWindowController

extern void *ctx;

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
	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayApp
														object:cmd];
}

- (IBAction)discardIdleAndContinue:(id)sender
{
	toggl_discard_time_and_continue(ctx,
									[self.idleEvent.guid UTF8String],
									self.idleEvent.started);
	[self.window orderOut:nil];
}

@end
