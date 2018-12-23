//
//  SystemService.m
//  TogglDesktop
//
//  Created by Nghia Tran on 12/21/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "SystemService.h"
#import "toggl_api.h"

extern void *ctx;

static NSString *kScreenIsLockedNotification = @"com.apple.screenIsLocked";
static NSString *kScreenIsUnlockedNotification = @"com.apple.screenIsUnlocked";

@implementation SystemService

- (void)dealloc
{
	[[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
	[[NSDistributedNotificationCenter defaultCenter] removeObserver:self];
}

- (void)registerSystemNotification
{
	[[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
														   selector:@selector(receiveShutdownNotification:)
															   name:NSWorkspaceWillPowerOffNotification
															 object:nil];

	NSDistributedNotificationCenter *distCenter = [NSDistributedNotificationCenter defaultCenter];
	[distCenter addObserver:self
				   selector:@selector(onScreenLockedNotification:)
					   name:kScreenIsLockedNotification
					 object:nil];
	[distCenter addObserver:self
				   selector:@selector(onScreenUnlockedNotification:)
					   name:kScreenIsUnlockedNotification
					 object:nil];
}

- (void)onScreenLockedNotification:(NSNotification *)note
{
	NSLog(@"onScreenLockedNotification: %@", [note name]);
	toggl_set_sleep(ctx);
}

- (void)onScreenUnlockedNotification:(NSNotification *)note
{
	NSLog(@"onScreenUnlockedNotification: %@", [note name]);
	toggl_set_wake(ctx);
}

- (void)receiveShutdownNotification:(NSNotification *)notification
{
	toggl_os_shutdown(ctx);
}

@end
