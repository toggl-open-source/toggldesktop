//
//  IdleNotificationWindowController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 06/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "IdleNotificationWindowController.h"
#import "UIEvents.h"
#import "IdleEvent.h"
#import "kopsik_api.h"
#import "TimeEntryViewItem.h"

@interface IdleNotificationWindowController ()
@property IdleEvent *idleEvent;
@end

@implementation IdleNotificationWindowController

extern void *ctx;

- (id)initWithWindow:(NSWindow *)window
{
	self = [super initWithWindow:window];
	if (self)
	{
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayIdleNotification:)
													 name:kDisplayIdleNotification
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayTimerState:)
													 name:kDisplayTimerState
												   object:nil];
	}
	return self;
}

- (void)windowDidLoad
{
	[super windowDidLoad];

	[self renderIdle];
}

- (void)renderIdle
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSDateFormatter *format = [[NSDateFormatter alloc] init];
	[format setDateFormat:@"HH:mm:ss"];
	NSString *dateString = [format stringFromDate:self.idleEvent.started];

	NSString *idleSince = [NSString stringWithFormat:@"You have been idle since %@", dateString];
	[self.idleSinceTextField setStringValue:idleSince];

	NSString *idleAmount = [NSString stringWithFormat:@" (%ld minutes)", self.idleEvent.seconds / 60];
	[self.idleAmountTextField setStringValue:idleAmount];
}

- (IBAction)stopButtonClicked:(id)sender
{
	NSTimeInterval startedAt = [self.idleEvent.started timeIntervalSince1970];

	kopsik_stop_running_time_entry_at(ctx, startedAt);
	[self.window orderOut:nil];
}

- (IBAction)ignoreButtonClicked:(id)sender
{
	[self.window orderOut:nil];
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
		[self.window orderOut:nil];
	}
}

- (void)startDisplayIdleNotification:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayIdleNotification:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayIdleNotification:(IdleEvent *)evt
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.idleEvent = evt;
	[self.window makeKeyAndOrderFront:nil];
	[self renderIdle];
}

@end
