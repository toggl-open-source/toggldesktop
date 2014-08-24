//
//  IdleNotificationWindowController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 06/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "IdleNotificationWindowController.h"
#import "toggl_api.h"

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
}

- (IBAction)stopButtonClicked:(id)sender
{
	toggl_discard_time_at(ctx,
						   [self.idleEvent.guid UTF8String],
						   self.idleEvent.started);
	[self.window orderOut:nil];
}

- (IBAction)ignoreButtonClicked:(id)sender
{
	[self.window orderOut:nil];
}

@end
