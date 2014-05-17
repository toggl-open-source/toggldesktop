//
//  PreferencesWindowController.m
//  Kopsik
//
//  Created by Tanel Lebedev on 22/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "PreferencesWindowController.h"
#import "kopsik_api.h"
#import "UIEvents.h"
#import "Settings.h"
#import "MASShortcutView+UserDefaults.h"
#import "DisplayCommand.h"
#import "Utils.h"

NSString *const kPreferenceGlobalShortcutShowHide = @"TogglDesktopGlobalShortcutShowHide";
NSString *const kPreferenceGlobalShortcutStartStop = @"TogglDesktopGlobalShortcutStartStop";

@implementation PreferencesWindowController

extern void *ctx;

- (void)windowDidLoad
{
	[super windowDidLoad];

	self.showHideShortcutView.associatedUserDefaultsKey = kPreferenceGlobalShortcutShowHide;
	self.startStopShortcutView.associatedUserDefaultsKey = kPreferenceGlobalShortcutStartStop;

	NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
	[formatter setUsesGroupingSeparator:NO];
	[formatter setNumberStyle:NSNumberFormatterDecimalStyle];
	[self.portTextField setFormatter:formatter];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplaySettings:)
												 name:kDisplaySettings
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplayLogin:)
												 name:kDisplayLogin
											   object:nil];
  
  [self.recordTimelineCheckbox setEnabled:self.user_id != 0];

	[self displaySettings:self.originalCmd];
}

- (IBAction)useProxyButtonChanged:(id)sender
{
	[self saveProxySettings];
}

- (IBAction)useIdleDetectionButtonChanged:(id)sender
{
	[self saveSettings];
}

- (IBAction)ontopCheckboxChanged:(id)sender
{
	[self saveSettings];
}

- (IBAction)reminderCheckboxChanged:(id)sender
{
	[self saveSettings];
}

- (IBAction)hostTextFieldChanged:(id)sender
{
	[self saveProxySettings];
}

- (IBAction)portTextFieldChanged:(id)sender
{
	[self saveProxySettings];
}

- (IBAction)usernameTextFieldChanged:(id)sender
{
	[self saveProxySettings];
}

- (IBAction)passwordTextFieldChanged:(id)sender
{
	[self saveProxySettings];
}

- (void)saveSettings
{
	NSLog(@"saveSettings");

	kopsik_set_settings(ctx,
						[Utils stateToBool:[self.useIdleDetectionButton state]],
						[Utils stateToBool:[self.menubarTimerCheckbox state]],
						[Utils stateToBool:[self.dockIconCheckbox state]],
						[Utils stateToBool:[self.ontopCheckbox state]],
						[Utils stateToBool:[self.reminderCheckbox state]]);
}

- (void)saveProxySettings
{
	NSLog(@"saveProxySettings");

	NSString *host = [self.hostTextField stringValue];
	NSInteger port = [self.portTextField integerValue];
	NSString *username = [self.usernameTextField stringValue];
	NSString *password = [self.passwordTextField stringValue];

	kopsik_set_proxy_settings(ctx,
							  [Utils stateToBool:[self.useProxyButton state]],
							  [host UTF8String],
							  (unsigned int)port,
							  [username UTF8String],
							  [password UTF8String]);
}

- (IBAction)recordTimelineCheckboxChanged:(id)sender
{
	kopsik_timeline_toggle_recording(ctx);
}

- (IBAction)menubarTimerCheckboxChanged:(id)sender
{
	[self saveSettings];
}

- (IBAction)dockIconCheckboxChanged:(id)sender
{
	[self saveSettings];
}

- (void)startDisplayLogin:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayLogin:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayLogin:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	self.user_id = cmd.user_id;
	[self.recordTimelineCheckbox setEnabled:self.user_id != 0];
}

- (void)startDisplaySettings:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displaySettings:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displaySettings:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	Settings *settings = cmd.settings;

	[self.useIdleDetectionButton setState:[Utils boolToState:settings.idle_detection]];
	[self.menubarTimerCheckbox setState:[Utils boolToState:settings.menubar_timer]];
	[self.dockIconCheckbox setState:[Utils boolToState:settings.dock_icon]];
	[self.ontopCheckbox setState:[Utils boolToState:settings.on_top]];
	[self.reminderCheckbox setState:[Utils boolToState:settings.reminder]];
	[self.recordTimelineCheckbox setEnabled:self.user_id != 0];
	[self.recordTimelineCheckbox setState:[Utils boolToState:settings.timeline_recording_enabled]];

	[self.useProxyButton setState:[Utils boolToState:settings.use_proxy]];
	[self.hostTextField setStringValue:settings.proxy_host];
	[self.portTextField setIntegerValue:settings.proxy_port];
	[self.usernameTextField setStringValue:settings.proxy_username];
	[self.passwordTextField setStringValue:settings.proxy_password];

	bool use_proxy = [self.useProxyButton state] == NSOnState;
	[self.hostTextField setEnabled:use_proxy];
	[self.portTextField setEnabled:use_proxy];
	[self.usernameTextField setEnabled:use_proxy];
	[self.passwordTextField setEnabled:use_proxy];
}

@end
