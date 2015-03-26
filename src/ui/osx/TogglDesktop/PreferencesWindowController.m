//
//  PreferencesWindowController.m
//
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import "PreferencesWindowController.h"
#import "toggl_api.h"
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

	[self.idleMinutesTextField setDelegate:self];
	[self.reminderMinutesTextField setDelegate:self];
}

- (IBAction)useProxyButtonChanged:(id)sender
{
	[self saveProxySettings];
}

- (IBAction)useIdleDetectionButtonChanged:(id)sender
{
	toggl_set_settings_use_idle_detection(ctx,
										  [Utils stateToBool:[self.useIdleDetectionButton state]]);

	self.idleMinutesTextField.enabled = [Utils stateToBool:self.useIdleDetectionButton.state];
}

- (IBAction)ontopCheckboxChanged:(id)sender
{
	toggl_set_settings_on_top(ctx,
							  [Utils stateToBool:[self.ontopCheckbox state]]);
}

- (IBAction)reminderCheckboxChanged:(id)sender
{
	toggl_set_settings_reminder(ctx,
								[Utils stateToBool:[self.reminderCheckbox state]]);
}

- (IBAction)focusOnShortcutCheckboxChanged:(id)sender
{
	toggl_set_settings_focus_on_shortcut(ctx,
										 [Utils stateToBool:[self.focusOnShortcutCheckbox state]]);
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

- (void)saveProxySettings
{
	NSLog(@"saveProxySettings");

	NSString *host = [self.hostTextField stringValue];
	NSInteger port = [self.portTextField integerValue];
	NSString *username = [self.usernameTextField stringValue];
	NSString *password = [self.passwordTextField stringValue];

	toggl_set_proxy_settings(ctx,
							 [Utils stateToBool:[self.useProxyButton state]],
							 [host UTF8String],
							 (unsigned int)port,
							 [username UTF8String],
							 [password UTF8String]);
}

- (IBAction)recordTimelineCheckboxChanged:(id)sender
{
	BOOL record_timeline = [Utils stateToBool:[self.recordTimelineCheckbox state]];

	toggl_timeline_toggle_recording(ctx, record_timeline);
}

- (IBAction)menubarTimerCheckboxChanged:(id)sender
{
	toggl_set_settings_menubar_timer(ctx,
									 [Utils stateToBool:[self.menubarTimerCheckbox state]]);
}

- (IBAction)menubarProjectCheckboxChanged:(id)sender
{
	toggl_set_settings_menubar_project(ctx,
									   [Utils stateToBool:[self.menubarProjectCheckbox state]]);
}

- (IBAction)dockIconCheckboxChanged:(id)sender
{
	toggl_set_settings_dock_icon(ctx,
								 [Utils stateToBool:[self.dockIconCheckbox state]]);
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
	[self.menubarProjectCheckbox setState:[Utils boolToState:settings.menubar_project]];
	[self.dockIconCheckbox setState:[Utils boolToState:settings.dock_icon]];
	[self.ontopCheckbox setState:[Utils boolToState:settings.on_top]];
	[self.reminderCheckbox setState:[Utils boolToState:settings.reminder]];
	[self.focusOnShortcutCheckbox setState:[Utils boolToState:settings.focus_on_shortcut]];

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

	self.idleMinutesTextField.intValue = settings.idle_minutes;
	self.idleMinutesTextField.enabled = settings.idle_detection;

	self.reminderMinutesTextField.intValue = settings.reminder_minutes;
	self.reminderMinutesTextField.enabled = settings.reminder;

	[self.autodetectProxyCheckbox setState:[Utils boolToState:settings.autodetect_proxy]];
}

- (IBAction)idleMinutesChange:(id)sender
{
	toggl_set_settings_idle_minutes(ctx,
									[self.idleMinutesTextField.stringValue intValue]);
}

- (IBAction)reminderMinutesChanged:(id)sender
{
	toggl_set_settings_reminder_minutes(ctx,
										[self.reminderMinutesTextField.stringValue intValue]);
}

- (void)controlTextDidChange:(NSNotification *)aNotification
{
	if ([aNotification object] == self.idleMinutesTextField)
	{
		toggl_set_settings_idle_minutes(ctx,
										[self.idleMinutesTextField.stringValue intValue]);
	}

	if ([aNotification object] == self.reminderMinutesTextField)
	{
		toggl_set_settings_reminder_minutes(ctx,
											[self.reminderMinutesTextField.stringValue intValue]);
	}
}

- (IBAction)autodetectProxyCheckboxChanged:(id)sender
{
	toggl_set_settings_autodetect_proxy(ctx,
										[Utils stateToBool:[self.autodetectProxyCheckbox state]]);
}

@end
