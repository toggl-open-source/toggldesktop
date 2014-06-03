//
//  AppDelegate.m
//  test2
//
//  Created by Alari on 9/15/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "AppDelegate.h"
#import "kopsik_api.h"
#import "MainWindowController.h"
#import "PreferencesWindowController.h"
#import "Bugsnag.h"
#import "UIEvents.h"
#import "TimeEntryViewItem.h"
#import "AboutWindowController.h"
#import "MenuItemTags.h"
#import "Update.h"
#import "idler.h"
#import "IdleEvent.h"
#import "IdleNotificationWindowController.h"
#import "CrashReporter.h"
#import "FeedbackWindowController.h"
#import "AutocompleteItem.h"
#import "const.h"
#import "MASShortcut+UserDefaults.h"
#import "ViewItem.h"
#import "Utils.h"
#import "Settings.h"
#import "DisplayCommand.h"

@interface AppDelegate ()
@property (nonatomic, strong) IBOutlet MainWindowController *mainWindowController;
@property (nonatomic, strong) IBOutlet PreferencesWindowController *preferencesWindowController;
@property (nonatomic, strong) IBOutlet AboutWindowController *aboutWindowController;
@property (nonatomic, strong) IBOutlet IdleNotificationWindowController *idleNotificationWindowController;
@property (nonatomic, strong) IBOutlet FeedbackWindowController *feedbackWindowController;
@property TimeEntryViewItem *lastKnownRunningTimeEntry;
@property NSTimer *menubarTimer;
@property NSTimer *idleTimer;
@property uint64_t user_id;
@property long lastIdleSecondsReading;
@property NSDate *lastIdleStarted;

// we'll be updating running TE as a menu item, too
@property (strong) IBOutlet NSMenuItem *runningTimeEntryMenuItem;

// Where logs are written and db is kept
@property NSString *app_path;
@property NSString *db_path;
@property NSString *log_path;
@property NSString *log_level;

// Environment (development, production, etc)
@property NSString *environment;

// Websocket and API hosts can be overriden
@property NSString *websocket_url_override;
@property NSString *api_url_override;

// For testing crash reporter
@property BOOL forceCrash;

// Avoid showing multiple upgrade dialogs
@property BOOL upgradeDialogVisible;

@property BOOL willTerminate;

@end

@implementation AppDelegate

void *ctx;
const int kDurationStringLength = 20;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	NSLog(@"applicationDidFinishLaunching");

	self.willTerminate = NO;

	self.user_id = 0;

	if (![self.environment isEqualToString:@"production"])
	{
		// Turn on UI constraint debugging, if not in production
		[[NSUserDefaults standardUserDefaults] setBool:YES
												forKey:@"NSConstraintBasedLayoutVisualizeMutuallyExclusiveConstraints"];
	}

	self.mainWindowController = [[MainWindowController alloc] initWithWindowNibName:@"MainWindowController"];
	[self.mainWindowController.window setReleasedWhenClosed:NO];

	PLCrashReporter *crashReporter = [self configuredCrashReporter];

	// Check if we previously crashed
	if ([crashReporter hasPendingCrashReport])
	{
		[self handleCrashReport];
	}

	// Enable the Crash Reporter
	NSError *error;
	if (![crashReporter enableCrashReporterAndReturnError:&error])
	{
		NSLog(@"Warning: Could not enable crash reporter: %@", error);
	}

	if (self.forceCrash)
	{
		abort();
	}

	if (!wasLaunchedAsHiddenLoginItem())
	{
		[self onShowMenuItem:self];
	}

	self.inactiveAppIcon = [NSImage imageNamed:@"app_inactive"];

	self.preferencesWindowController = [[PreferencesWindowController alloc]
										initWithWindowNibName:@"PreferencesWindowController"];

	self.aboutWindowController = [[AboutWindowController alloc]
								  initWithWindowNibName:@"AboutWindowController"];

	self.idleNotificationWindowController = [[IdleNotificationWindowController alloc]
											 initWithWindowNibName:@"IdleNotificationWindowController"];

	self.feedbackWindowController = [[FeedbackWindowController alloc]
									 initWithWindowNibName:@"FeedbackWindowController"];

	[self createStatusItem];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startStopAt:)
												 name:kCommandStopAt
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startStop:)
												 name:kCommandStop
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startNew:)
												 name:kCommandNew
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startContinueTimeEntry:)
												 name:kCommandContinue
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplayUpdate:)
												 name:kDisplayUpdate
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplayOnlineState:)
												 name:kDisplayOnlineState
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplayTimerState:)
												 name:kDisplayTimerState
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplayLogin:)
												 name:kDisplayLogin
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplaySettings:)
												 name:kDisplaySettings
											   object:nil];

	kopsik_set_environment(ctx, [self.environment UTF8String]);

	_Bool started = kopsik_context_start_events(ctx);
	NSAssert(started, @"Failed to start UI");

	[MASShortcut registerGlobalShortcutWithUserDefaultsKey:kPreferenceGlobalShortcutShowHide handler:^{
		 if ([[NSApplication sharedApplication] isActive] && [self.mainWindowController.window isVisible])
		 {
			 [self.mainWindowController.window close];
		 }
		 else
		 {
			 [self onShowMenuItem:self];
		 }
	 }];

	[MASShortcut registerGlobalShortcutWithUserDefaultsKey:kPreferenceGlobalShortcutStartStop handler:^{
		 if (self.lastKnownRunningTimeEntry == nil)
		 {
			 [self onContinueMenuItem:self];
		 }
		 else
		 {
			 [self onStopMenuItem:self];
		 }
	 }];

	[[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
														   selector:@selector(receiveSleepNote:)
															   name:NSWorkspaceWillSleepNotification object:NULL];

	[[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
														   selector:@selector(receiveWakeNote:)
															   name:NSWorkspaceDidWakeNotification object:NULL];

	[[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:self];
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center
	 shouldPresentNotification:(NSUserNotification *)notification
{
	return YES;
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center
	   didActivateNotification:(NSUserNotification *)notification
{
	NSLog(@"didActivateNotification %@", notification);
	[self onShowMenuItem:self];
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center
		didDeliverNotification:(NSUserNotification *)notification
{
	NSLog(@"didDeliverNotification %@", notification);
}

- (void)receiveSleepNote:(NSNotification *)note
{
	NSLog(@"receiveSleepNote: %@", [note name]);
	kopsik_set_sleep(ctx);
}

- (void)receiveWakeNote:(NSNotification *)note
{
	NSLog(@"receiveWakeNote: %@", [note name]);
	kopsik_set_wake(ctx);
}

- (void)startNew:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(new:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)new:(TimeEntryViewItem *)new_time_entry
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	NSAssert(new_time_entry != nil, @"new time entry details cannot be nil");

	kopsik_start(ctx,
				 [new_time_entry.Description UTF8String],
				 [new_time_entry.duration UTF8String],
				 new_time_entry.TaskID,
				 new_time_entry.ProjectID);

	[self onShowMenuItem:self];
}

- (void)startContinueTimeEntry:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(continueTimeEntry:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)continueTimeEntry:(NSString *)guid
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	if (guid == nil)
	{
		kopsik_continue_latest(ctx);
	}
	else
	{
		kopsik_continue(ctx, [guid UTF8String]);
	}

	[self onShowMenuItem:self];
}

- (void)startStop:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(stop)
						   withObject:nil
						waitUntilDone:NO];
}

- (void)stop
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	kopsik_stop(ctx);

	[self onShowMenuItem:self];
}

- (void)startStopAt:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(stopAt:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)stopAt:(IdleEvent *)idleEvent
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSAssert(idleEvent != nil, @"idle event cannot be nil");
	NSLog(@"Idle event: %@", idleEvent);
	NSTimeInterval startedAt = [idleEvent.started timeIntervalSince1970];
	NSLog(@"Time entry stop at %f", startedAt);
	kopsik_stop_running_time_entry_at(ctx, startedAt);

	[self onShowMenuItem:self];
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

	// Start idle detection, if its enabled
	if (cmd.settings.idle_detection)
	{
		NSLog(@"Starting idle detection");
		self.idleTimer = [NSTimer
						  scheduledTimerWithTimeInterval:1.0
												  target:self
												selector:@selector(idleTimerFired:)
												userInfo:nil
												 repeats:YES];
	}
	else
	{
		NSLog(@"Idle detection is disabled. Stopping idle detection.");
		if (self.idleTimer != nil)
		{
			[self.idleTimer invalidate];
			self.idleTimer = nil;
		}
		[self.statusItem setTitle:@""];
	}

	// Start menubar timer if its enabled
	if (cmd.settings.menubar_timer)
	{
		NSLog(@"Starting menubar timer");
		self.menubarTimer = [NSTimer
							 scheduledTimerWithTimeInterval:1.0
													 target:self
												   selector:@selector(menubarTimerFired:)
												   userInfo:nil
													repeats:YES];
	}
	else
	{
		NSLog(@"Menubar timer is disabled. Stopping menubar timer.");
		if (self.menubarTimer != nil)
		{
			[self.menubarTimer invalidate];
			self.menubarTimer = nil;
		}
		[self.statusItem setTitle:@""];
	}

	// Show/Hide dock icon
	ProcessSerialNumber psn = { 0, kCurrentProcess };
	if (cmd.settings.dock_icon)
	{
		NSLog(@"Showing dock icon");
		TransformProcessType(&psn, kProcessTransformToForegroundApplication);
	}
	else
	{
		NSLog(@"Hiding dock icon.");
		TransformProcessType(&psn, kProcessTransformToUIElementApplication);
	}

	// Stay on top
	if (cmd.settings.on_top)
	{
		[self.mainWindowController.window setLevel:NSFloatingWindowLevel];
	}
	else
	{
		[self.mainWindowController.window setLevel:NSNormalWindowLevel];
	}

	if (cmd.open)
	{
		self.preferencesWindowController.originalCmd = cmd;
		self.preferencesWindowController.user_id = self.user_id;
		[self.preferencesWindowController showWindow:self];
		[NSApp activateIgnoringOtherApps:YES];
	}
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
}

- (void)startDisplayOnlineState:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayOnlineState:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayOnlineState:(NSString *)is_online
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	if (is_online)
	{
		self.currentOnImage = self.onImage;
		self.currentOffImage = self.offImage;
	}
	else
	{
		self.currentOnImage = self.offlineOnImage;
		self.currentOffImage = self.offlineOffImage;
	}

	if (self.lastKnownRunningTimeEntry)
	{
		[self.statusItem setImage:self.currentOnImage];
	}
	else
	{
		[self.statusItem setImage:self.currentOffImage];
	}
}

- (void)startDisplayTimerState:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayTimerState:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayTimerState:(TimeEntryViewItem *)timeEntry
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.lastKnownRunningTimeEntry = timeEntry;

	if (timeEntry)
	{
		// Change app dock icon to default, which is red / tracking
		// See https://developer.apple.com/library/mac/documentation/Carbon/Conceptual/customizing_docktile/dockconcepts.pdf
		if (!self.willTerminate)
		{
			[NSApp setApplicationIconImage:nil];
		}

		[self.statusItem setImage:self.currentOnImage];

		// Change tracking time entry row in menu
		NSString *msg = [NSString stringWithFormat:@"Running: %@",
						 self.lastKnownRunningTimeEntry.Description];
		[self.runningTimeEntryMenuItem setTitle:msg];

		return;
	}

	if (!self.willTerminate)
	{
		[NSApp setApplicationIconImage:self.inactiveAppIcon];
	}
	[self.statusItem setTitle:@""];
	[self.statusItem setImage:self.currentOffImage];
	[self.runningTimeEntryMenuItem setTitle:@"Timer is not running."];
}

- (void)createStatusItem
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSMenu *menu = [[NSMenu alloc] init];
	self.runningTimeEntryMenuItem = [menu addItemWithTitle:@"Timer status"
													action:nil
											 keyEquivalent:@""];
	[menu addItem:[NSMenuItem separatorItem]];
	[menu addItemWithTitle:@"New"
					action:@selector(onNewMenuItem:)
			 keyEquivalent:@"n"].tag = kMenuItemTagNew;
	[menu addItemWithTitle:@"Continue"
					action:@selector(onContinueMenuItem:)
			 keyEquivalent:@"o"].tag = kMenuItemTagContinue;
	[menu addItemWithTitle:@"Stop"
					action:@selector(onStopMenuItem:)
			 keyEquivalent:@"s"].tag = kMenuItemTagStop;
	[menu addItem:[NSMenuItem separatorItem]];
	[menu addItemWithTitle:@"Show"
					action:@selector(onShowMenuItem:)
			 keyEquivalent:@"t"];
	[menu addItem:[NSMenuItem separatorItem]];
	[menu addItemWithTitle:@"Sync"
					action:@selector(onSyncMenuItem:)
			 keyEquivalent:@""].tag = kMenuItemTagSync;
	[menu addItemWithTitle:@"Reports"
					action:@selector(onOpenBrowserMenuItem:)
			 keyEquivalent:@""].tag = kMenuItemTagOpenBrowser;
	[menu addItemWithTitle:@"Preferences"
					action:@selector(onPreferencesMenuItem:)
			 keyEquivalent:@""];
	[menu addItem:[NSMenuItem separatorItem]];
	[menu addItemWithTitle:@"About"
					action:@selector(onAboutMenuItem:)
			 keyEquivalent:@""];
	NSMenuItem *sendFeedbackMenuItem = [menu addItemWithTitle:@"Send Feedback"
													   action:@selector(onSendFeedbackMenuItem)
												keyEquivalent:@""];
	sendFeedbackMenuItem.tag = kMenuItemTagSendFeedback;
	[menu addItemWithTitle:@"Logout"
					action:@selector(onLogoutMenuItem:)
			 keyEquivalent:@""].tag = kMenuItemTagLogout;;
	[menu addItemWithTitle:@"Quit"
					action:@selector(onQuitMenuItem)
			 keyEquivalent:@""];

	NSStatusBar *bar = [NSStatusBar systemStatusBar];

	self.onImage = [NSImage imageNamed:@"on"];
	self.offImage = [NSImage imageNamed:@"off"];

	self.offlineOnImage = [NSImage imageNamed:@"offline_on"];
	self.offlineOffImage = [NSImage imageNamed:@"offline_off"];

	self.currentOnImage = self.onImage;
	self.currentOffImage = self.offImage;

	self.statusItem = [bar statusItemWithLength:NSVariableStatusItemLength];
	[self.statusItem setTitle:@""];
	[self.statusItem setHighlightMode:YES];
	[self.statusItem setEnabled:YES];
	[self.statusItem setMenu:menu];
	[self.statusItem setImage:self.currentOffImage];
}

- (void)onNewMenuItem:(id)sender
{
	[[NSNotificationCenter defaultCenter] postNotificationName:kCommandNew
														object:[[TimeEntryViewItem alloc] init]];
}

- (void)onSendFeedbackMenuItem
{
	[self.feedbackWindowController showWindow:self];
	[NSApp activateIgnoringOtherApps:YES];
}

- (void)onSendFeedbackMainMenuItem:(id)sender
{
	[self onSendFeedbackMenuItem];
}

- (IBAction)onContinueMenuItem:(id)sender
{
	[[NSNotificationCenter defaultCenter] postNotificationName:kCommandContinue
														object:nil];
}

- (IBAction)onStopMenuItem:(id)sender
{
	[[NSNotificationCenter defaultCenter] postNotificationName:kCommandStop
														object:nil];
}

- (IBAction)onSyncMenuItem:(id)sender
{
	kopsik_sync(ctx);
}

- (IBAction)onOpenBrowserMenuItem:(id)sender
{
	kopsik_open_in_browser(ctx);
}

- (IBAction)onHelpMenuItem:(id)sender
{
	kopsik_get_support(ctx);
}

- (IBAction)onLogoutMenuItem:(id)sender
{
	kopsik_logout(ctx);
	[self onShowMenuItem:self];
}

- (IBAction)onClearCacheMenuItem:(id)sender
{
	NSAlert *alert = [[NSAlert alloc] init];

	[alert addButtonWithTitle:@"OK"];
	[alert addButtonWithTitle:@"Cancel"];
	[alert setMessageText:@"Clear local data and log out?"];
	[alert setInformativeText:@"Deleted unsynced time entries cannot be restored."];
	[alert setAlertStyle:NSWarningAlertStyle];
	if ([alert runModal] != NSAlertFirstButtonReturn)
	{
		return;
	}
	kopsik_clear_cache(ctx);
}

- (IBAction)onAboutMenuItem:(id)sender
{
	kopsik_about(ctx);
}

- (IBAction)onShowMenuItem:(id)sender
{
	[self.mainWindowController showWindow:self];
	[NSApp activateIgnoringOtherApps:YES];
}

- (IBAction)onPreferencesMenuItem:(id)sender
{
	kopsik_edit_preferences(ctx);
}

- (IBAction)onHideMenuItem:(id)sender
{
	[self.mainWindowController.window close];
}

- (void)onQuitMenuItem
{
	[[NSApplication sharedApplication] terminate:self];
}

- (void)applicationWillTerminate:(NSNotification *)app
{
	NSLog(@"applicationWillTerminate");
	self.willTerminate = YES;
	kopsik_context_clear(ctx);
	ctx = 0;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender
					hasVisibleWindows:(BOOL)flag
{
	[self.mainWindowController.window setIsVisible:YES];
	return YES;
}

const NSString *appName = @"osx_native_app";

- (void)parseCommandLineArguments
{
	NSArray *arguments = [[NSProcessInfo processInfo] arguments];

	NSLog(@"Command line arguments: %@", arguments);

	for (int i = 1; i < arguments.count; i++)
	{
		NSString *argument = arguments[i];

		if (([argument rangeOfString:@"force"].location != NSNotFound) &&
			([argument rangeOfString:@"crash"].location != NSNotFound))
		{
			NSLog(@"forcing crash");
			self.forceCrash = YES;
			continue;
		}
		if (([argument rangeOfString:@"log"].location != NSNotFound) &&
			([argument rangeOfString:@"path"].location != NSNotFound))
		{
			self.log_path = arguments[i + 1];
			NSLog(@"log path overriden with '%@'", self.log_path);
			continue;
		}
		if (([argument rangeOfString:@"db"].location != NSNotFound) &&
			([argument rangeOfString:@"path"].location != NSNotFound))
		{
			self.db_path = arguments[i + 1];
			NSLog(@"db path overriden with '%@'", self.db_path);
			continue;
		}
		if (([argument rangeOfString:@"log"].location != NSNotFound) &&
			([argument rangeOfString:@"level"].location != NSNotFound))
		{
			self.log_level = arguments[i + 1];
			NSLog(@"log level overriden with '%@'", self.log_level);
			continue;
		}
		if (([argument rangeOfString:@"api"].location != NSNotFound) &&
			([argument rangeOfString:@"url"].location != NSNotFound))
		{
			self.api_url_override = arguments[i + 1];
			NSLog(@"API URL overriden with '%@'", self.api_url_override);
			continue;
		}
		if (([argument rangeOfString:@"websocket"].location != NSNotFound) &&
			([argument rangeOfString:@"url"].location != NSNotFound))
		{
			self.websocket_url_override = arguments[i + 1];
			NSLog(@"Websocket URL overriden with '%@'", self.websocket_url_override);
			continue;
		}
	}
}

- (id)init
{
	self = [super init];

	NSDictionary *infoDict = [[NSBundle mainBundle] infoDictionary];
	self.environment = infoDict[@"KopsikEnvironment"];

	// Disallow duplicate instances in production
	if ([self.environment isEqualToString:@"production"])
	{
		[Utils disallowDuplicateInstances];
	}

	[Bugsnag startBugsnagWithApiKey:@"2a46aa1157256f759053289f2d687c2f"];
	NSAssert(self.environment != nil, @"Missing environment in plist");
	[Bugsnag configuration].releaseStage = self.environment;

	self.app_path = [Utils applicationSupportDirectory:self.environment];
	self.db_path = [self.app_path stringByAppendingPathComponent:@"kopsik.db"];
	self.log_path = [self.app_path stringByAppendingPathComponent:@"kopsik.log"];
	self.log_level = @"debug";

	[self parseCommandLineArguments];

	NSLog(@"Starting with db path %@, log path %@, log level %@",
		  self.db_path, self.log_path, self.log_level);

	kopsik_set_log_path([self.log_path UTF8String]);
	kopsik_set_log_level([self.log_level UTF8String]);

	NSString *version = infoDict[@"CFBundleShortVersionString"];

	ctx = kopsik_context_init([appName UTF8String], [version UTF8String]);

	kopsik_on_error(ctx, on_error);
	kopsik_on_update(ctx, on_update);
	kopsik_on_online_state(ctx, on_online_state);
	kopsik_on_login(ctx, on_login);
	kopsik_on_url(ctx, on_url);
	kopsik_on_reminder(ctx, on_reminder);
	kopsik_on_time_entry_list(ctx, on_time_entry_list);
	kopsik_on_time_entry_autocomplete(ctx, on_time_entry_autocomplete);
	kopsik_on_project_autocomplete(ctx, on_project_autocomplete);
	kopsik_on_workspace_select(ctx, on_workspace_select);
	kopsik_on_client_select(ctx, on_client_select);
	kopsik_on_tags(ctx, on_tags);
	kopsik_on_time_entry_editor(ctx, on_time_entry_editor);
	kopsik_on_settings(ctx, on_settings);
	kopsik_on_timer_state(ctx, on_timer_state);

	NSLog(@"Version %@", version);

	_Bool res = kopsik_set_db_path(ctx, [self.db_path UTF8String]);
	NSAssert(res, ([NSString stringWithFormat:@"Failed to initialize DB with path: %@", self.db_path]));

	id logToFile = infoDict[@"KopsikLogUserInterfaceToFile"];
	if ([logToFile boolValue])
	{
		NSLog(@"Redirecting UI log to file");
		NSString *logPath =
			[self.app_path stringByAppendingPathComponent:@"ui.log"];
		freopen([logPath fileSystemRepresentation], "a+", stderr);
	}

	if (self.api_url_override != nil)
	{
		kopsik_set_api_url(ctx, [self.api_url_override UTF8String]);
	}

	if (self.websocket_url_override != nil)
	{
		kopsik_set_websocket_url(ctx, [self.websocket_url_override UTF8String]);
	}

	NSLog(@"AppDelegate init done");

	return self;
}

- (void)menubarTimerFired:(NSTimer *)timer
{
	if (self.lastKnownRunningTimeEntry != nil)
	{
		char str[kDurationStringLength];
		kopsik_format_duration_in_seconds_hhmm(
			self.lastKnownRunningTimeEntry.duration_in_seconds,
			str,
			kDurationStringLength);
		NSString *statusStr = @" ";
		statusStr = [statusStr stringByAppendingString:[NSString stringWithUTF8String:str]];
		[self.statusItem setTitle:statusStr];
	}
}

- (void)idleTimerFired:(NSTimer *)timer
{
	uint64_t idle_seconds = 0;

	if (0 != get_idle_time(&idle_seconds))
	{
		NSLog(@"Achtung! Failed to get idle status.");
		return;
	}

	//  NSLog(@"Idle seconds: %lld", idle_seconds);

	if (idle_seconds >= kIdleThresholdSeconds && self.lastIdleStarted == nil)
	{
		NSTimeInterval since = [[NSDate date] timeIntervalSince1970] - idle_seconds;
		self.lastIdleStarted = [NSDate dateWithTimeIntervalSince1970:since];
		NSLog(@"User is idle since %@", self.lastIdleStarted);
	}
	else if (self.lastIdleStarted != nil &&
			 self.lastIdleSecondsReading >= idle_seconds)
	{
		NSDate *now = [NSDate date];
		if (self.lastKnownRunningTimeEntry)
		{
			IdleEvent *idleEvent = [[IdleEvent alloc] init];
			idleEvent.started = self.lastIdleStarted;
			idleEvent.finished = now;
			idleEvent.seconds = self.lastIdleSecondsReading;
			[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayIdleNotification
																object:idleEvent];
		}
		else
		{
			NSLog(@"Time entry is not running, ignoring idleness");
		}
		NSLog(@"User is not idle since %@", now);
		self.lastIdleStarted = nil;
	}

	self.lastIdleSecondsReading = idle_seconds;
}

- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)anItem
{
	switch ([anItem tag])
	{
		case kMenuItemTagNew :
			if (0 == self.user_id)
			{
				return NO;
			}
			break;
		case kMenuItemTagContinue :
			if (0 == self.user_id)
			{
				return NO;
			}
			if (self.lastKnownRunningTimeEntry != nil)
			{
				return NO;
			}
			break;
		case kMenuItemTagStop :
			if (0 == self.user_id)
			{
				return NO;
			}
			if (self.lastKnownRunningTimeEntry == nil)
			{
				return NO;
			}
			break;
		case kMenuItemTagSync :
			if (0 == self.user_id)
			{
				return NO;
			}
			break;
		case kMenuItemTagLogout :
			if (0 == self.user_id)
			{
				return NO;
			}
			break;
		case kMenuItemTagClearCache :
			if (0 == self.user_id)
			{
				return NO;
			}
			break;
		case kMenuItemTagSendFeedback :
			if (0 == self.user_id)
			{
				return NO;
			}
			break;
		case kMenuItemTagOpenBrowser :
			if (0 == self.user_id)
			{
				return NO;
			}
			break;
		default :
			// Dont care about this stuff
			break;
	}
	return YES;
}

- (void)startDisplayUpdate:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayUpdate:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayUpdate:(DisplayCommand *)cmd
{
	if (cmd.open)
	{
		self.aboutWindowController.displayCommand = cmd;
		[self.aboutWindowController showWindow:self];
		[NSApp activateIgnoringOtherApps:YES];
		return;
	}

	if (!cmd.update.is_update_available)
	{
		return;
	}

	if (self.upgradeDialogVisible || self.aboutWindowController.window.isVisible)
	{
		NSLog(@"Upgrade dialog already visible");
		return;
	}
	self.upgradeDialogVisible = YES;

	NSAlert *alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"Yes"];
	[alert addButtonWithTitle:@"No"];
	[alert setMessageText:@"Download new version?"];
	NSString *informative = [NSString stringWithFormat:
							 @"There's a new version of this app available (%@).", cmd.update.version];
	[alert setInformativeText:informative];
	[alert setAlertStyle:NSWarningAlertStyle];
	if ([alert runModal] != NSAlertFirstButtonReturn)
	{
		self.upgradeDialogVisible = NO;
		return;
	}

	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:cmd.update.URL]];
	[NSApp terminate:nil];
}

- (PLCrashReporter *)configuredCrashReporter
{
	PLCrashReporterConfig *config = [[PLCrashReporterConfig alloc]
									 initWithSignalHandlerType:PLCrashReporterSignalHandlerTypeBSD
										 symbolicationStrategy:PLCrashReporterSymbolicationStrategyAll];

	return [[PLCrashReporter alloc] initWithConfiguration:config];
}

- (void)handleCrashReport
{
	PLCrashReporter *crashReporter = [self configuredCrashReporter];

	NSError *error;
	NSData *crashData = [crashReporter loadPendingCrashReportDataAndReturnError:&error];

	if (crashData == nil)
	{
		NSLog(@"Could not load crash report: %@", error);
		[crashReporter purgePendingCrashReport];
		return;
	}

	PLCrashReport *report = [[PLCrashReport alloc] initWithData:crashData
														  error:&error];
	if (report == nil)
	{
		NSLog(@"Could not parse crash report");
		[crashReporter purgePendingCrashReport];
		return;
	}

	NSString *summary = [NSString stringWithFormat:@"Crashed with signal %@ (code %@)",
						 report.signalInfo.name,
						 report.signalInfo.code];

	NSString *humanReadable = [PLCrashReportTextFormatter stringValueForCrashReport:report
																	 withTextFormat:PLCrashReportTextFormatiOS];
	NSLog(@"Crashed on %@", report.systemInfo.timestamp);
	NSLog(@"Report: %@", humanReadable);

	NSException *exception;
	NSMutableDictionary *data = [[NSMutableDictionary alloc] init];;
	if (report.hasExceptionInfo)
	{
		exception = [NSException
					 exceptionWithName:report.exceptionInfo.exceptionName
								reason:report.exceptionInfo.exceptionReason
							  userInfo:nil];
	}
	else
	{
		exception = [NSException
					 exceptionWithName:summary
								reason:humanReadable
							  userInfo:nil];
	}
	[Bugsnag notify:exception withData:data];

	[crashReporter purgePendingCrashReport];
}

void on_update(
	const _Bool open,
	KopsikUpdateViewItem *view)
{
	Update *update = [[Update alloc] init];

	[update load:view];

	DisplayCommand *cmd = [[DisplayCommand alloc] init];
	cmd.open = open;
	cmd.update = update;

	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayUpdate
														object:cmd];
}

void on_online_state(const _Bool is_online)
{
	NSString *value = is_online ? @"online" : nil;

	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayOnlineState
														object:value];
}

void on_login(const _Bool open, const uint64_t user_id)
{
	[Bugsnag setUserAttribute:@"user_id" withValue:[NSString stringWithFormat:@"%lld", user_id]];

	DisplayCommand *cmd = [[DisplayCommand alloc] init];
	cmd.open = open;
	cmd.user_id = user_id;

	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayLogin
														object:cmd];
}

void on_reminder(const char *title, const char *informative_text)
{
	NSUserNotification *notification = [[NSUserNotification alloc] init];

	[notification setTitle:[NSString stringWithUTF8String:title]];
	[notification setInformativeText:[NSString stringWithUTF8String:informative_text]];
	[notification setDeliveryDate:[NSDate dateWithTimeInterval:0 sinceDate:[NSDate date]]];
	[notification setSoundName:NSUserNotificationDefaultSoundName];
	NSUserNotificationCenter *center = [NSUserNotificationCenter defaultUserNotificationCenter];
	[center scheduleNotification:notification];
}

void on_url(const char *url)
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}

void on_time_entry_list(const _Bool open,
						KopsikTimeEntryViewItem *first)
{
	NSMutableArray *viewitems = [[NSMutableArray alloc] init];
	KopsikTimeEntryViewItem *it = first;

	while (it)
	{
		TimeEntryViewItem *model = [[TimeEntryViewItem alloc] init];
		[model load:it];
		[viewitems addObject:model];
		it = it->Next;
	}
	DisplayCommand *cmd = [[DisplayCommand alloc] init];
	cmd.open = open;
	cmd.timeEntries = viewitems;
	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayTimeEntryList
														object:cmd];
}

void on_time_entry_autocomplete(KopsikAutocompleteItem *first)
{
	NSMutableArray *viewitems = [[NSMutableArray alloc] init];
	KopsikAutocompleteItem *record = first;

	while (record)
	{
		AutocompleteItem *item = [[AutocompleteItem alloc] init];
		[item load:record];
		[viewitems addObject:item];
		record = record->Next;
	}
	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayTimeEntryAutocomplete
														object:viewitems];
}

void on_project_autocomplete(KopsikAutocompleteItem *first)
{
	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayProjectAutocomplete
														object:[AutocompleteItem loadAll:first]];
}

void on_tags(KopsikViewItem *first)
{
	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayTags
														object:[ViewItem loadAll:first]];
}

void on_client_select(KopsikViewItem *first)
{
	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayClientSelect
														object:[ViewItem loadAll:first]];
}

void on_workspace_select(KopsikViewItem *first)
{
	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayWorkspaceSelect
														object:[ViewItem loadAll:first]];
}

void on_time_entry_editor(const _Bool open,
						  KopsikTimeEntryViewItem *te,
						  const char *focused_field_name)
{
	TimeEntryViewItem *item = [[TimeEntryViewItem alloc] init];

	[item load:te];
	DisplayCommand *cmd = [[DisplayCommand alloc] init];
	cmd.open = open;
	cmd.timeEntry = item;
	cmd.timeEntry.focusedFieldName = [NSString stringWithUTF8String:focused_field_name];
	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayTimeEntryEditor
														object:cmd];
}

void on_error(const char *errmsg, const _Bool is_user_error)
{
	NSString *msg = [NSString stringWithUTF8String:errmsg];

	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayError
														object:msg];
	if (!is_user_error)
	{
		[Bugsnag notify:[NSException exceptionWithName:msg reason:msg userInfo:nil]];
	}
}

void on_settings(const _Bool open,
				 KopsikSettingsViewItem *settings)
{
	Settings *s = [[Settings alloc] init];

	[s load:settings];

	DisplayCommand *cmd = [[DisplayCommand alloc] init];
	cmd.open = open;
	cmd.settings = s;
	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplaySettings
														object:cmd];
}

void on_timer_state(KopsikTimeEntryViewItem *te)
{
	TimeEntryViewItem *view_item = nil;

	if (te)
	{
		view_item = [[TimeEntryViewItem alloc] init];
		[view_item load:te];
	}
	[[NSNotificationCenter defaultCenter] postNotificationName:kDisplayTimerState
														object:view_item];
}

@end
