//
//  AppDelegate.m
//  test2
//
//  Created by Alari on 9/15/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "AppDelegate.h"
#import "AboutWindowController.h"
#import "AutocompleteItem.h"
#import "AutotrackerRuleItem.h"
#import <Bugsnag/Bugsnag.h>
#import "DisplayCommand.h"
#import "FeedbackWindowController.h"
#import "IdleEvent.h"
#import "IdleNotificationWindowController.h"
#import "MainWindowController.h"
#import "MenuItemTags.h"
#import "PreferencesWindowController.h"
#import "Settings.h"
#import "TimeEntryViewItem.h"
#import "UIEvents.h"
#import "Utils.h"
#import "ViewItem.h"
#import "CountryViewItem.h"
#import "UnsupportedNotice.h"
#import "idler.h"
#import "UserNotificationCenter.h"
#import "SystemService.h"
#import "TogglDesktop-Swift.h"
#import "AppIconFactory.h"
#import <MASShortcut/Shortcut.h>
#import "TimelineDisplayCommand.h"
#import "Reachability.h"
#import "MenuItemTags.h"
#import <AppAuth/AppAuth.h>

#ifdef SPARKLE
#import <Sparkle/Sparkle.h>
#endif

@interface AppDelegate ()
@property (nonatomic, strong) MainWindowController *mainWindowController;
@property (nonatomic, strong) PreferencesWindowController *preferencesWindowController;
@property (nonatomic, strong) AboutWindowController *aboutWindowController;
@property (nonatomic, strong) IdleNotificationWindowController *idleNotificationWindowController;
@property (nonatomic, strong) FeedbackWindowController *feedbackWindowController;

// Touch Bar items
@property (nonatomic, strong) GlobalTouchbarButton *touchItem __OSX_AVAILABLE_STARTING(__MAC_10_12_2,__IPHONE_NA);
@property (nonatomic, assign) BOOL isAddedTouchBar;

// Remember some app state
@property (nonatomic, strong) TimeEntryViewItem *lastKnownRunningTimeEntry;
@property (nonatomic, assign) BOOL lastKnownOnlineState;
@property (nonatomic, assign) NSUInteger lastKnownUserID;

// We'll change app icon in the tray. So keep the different state images handy
@property (nonatomic, strong) NSMutableDictionary *statusImages;

// Timers to update app state
@property (nonatomic, strong) NSTimer *menubarTimer;
@property (nonatomic, strong) NSTimer *idleTimer;

// We'll be updating running TE as a menu item, too
@property (nonatomic, strong) NSMenuItem *runningTimeEntryMenuItem;

// We'll add user email once userdata has been loaded
@property (nonatomic, strong) NSMenuItem *currentUserEmailMenuItem;

// Where logs are written and db is kept
@property (nonatomic, copy) NSString *app_path;
@property (nonatomic, copy) NSString *db_path;
@property (nonatomic, copy) NSString *log_path;
@property (nonatomic, copy) NSString *log_level;

// Environment (development, production, etc)
@property (nonatomic, copy) NSString *environment;

@property (nonatomic, copy) NSString *version;

// For testing crash reporter
@property (nonatomic, assign) BOOL forceCrash;

// Avoid doing stuff when app is already shutting down
@property (nonatomic, assign) BOOL willTerminate;

// Show or not show menubar timer
@property (nonatomic, assign) BOOL showMenuBarTimer;

// Show or not show menubar project
@property (nonatomic, assign) BOOL showMenuBarProject;

// Manual mode
@property (strong, nonatomic) NSMenuItem *manualModeMenuItem;
@property (strong, nonatomic) _NSKeyValueObservation *effectiveAppearanceObs;

// System Service
@property (strong, nonatomic) SystemService *systemService;
@property (nonatomic, assign) BOOL manualMode;
@property (nonatomic, assign) BOOL onTop;

@end

@implementation AppDelegate

void *ctx;


- (void)applicationWillFinishLaunching:(NSNotification *)not
{
	self.willTerminate = NO;
	self.lastKnownOnlineState = YES;
	self.lastKnownUserID = 0;
	self.showMenuBarTimer = NO;
	self.manualMode = NO;
	self.onTop = NO;
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	NSLog(@"applicationDidFinishLaunching");

	if (![self.environment isEqualToString:@"production"] && ![self.version isEqualToString:@"7.0.0"])
	{
		// Turn on UI constraint debugging, if not in production
		[[NSUserDefaults standardUserDefaults] setBool:YES
												forKey:@"NSConstraintBasedLayoutVisualizeMutuallyExclusiveConstraints"];
	}

	self.mainWindowController = [[MainWindowController alloc] initWithWindowNibName:@"MainWindowController"];
	[self.mainWindowController.window setReleasedWhenClosed:NO];

	if (self.forceCrash)
	{
		abort();
	}

	if (!wasLaunchedAsHiddenLoginItem())
	{
		[self onShowMenuItem:self];
	}

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
											 selector:@selector(startDisplayIdleNotification:)
												 name:kDisplayIdleNotification
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
											 selector:@selector(startDisplayApp:)
												 name:kDisplayApp
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplayOnlineState:)
												 name:kDisplayOnlineState
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplaySyncState:)
												 name:kDisplaySyncState
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplayUnsyncedItems:)
												 name:kDisplayUnsyncedItems
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
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplayPromotion:)
												 name:kDisplayPromotion
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startToggleGroup:)
												 name:kToggleGroup
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startUpdateIconTooltip:)
												 name:kUpdateIconTooltip
											   object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(invalidAppleUserCrendentialNotification:)
                                                 name:kInvalidAppleUserCrendential
                                               object:nil];

	if (@available(macOS 10.14, *))
	{
		self.effectiveAppearanceObs = [self.mainWindowController.window observerEffectiveAppearanceNotification];
	}

	toggl_set_environment(ctx, [self.environment UTF8String]);

	bool_t started = toggl_ui_start(ctx);
	NSAssert(started, @"Failed to start UI");

	__weak typeof(self) weakSelf = self;
	[[MASShortcutBinder sharedBinder] bindShortcutWithDefaultsKey:kPreferenceGlobalShortcutShowHide
														 toAction:^{
		 typeof(self) strongSelf = weakSelf;
		 if ([[NSApplication sharedApplication] isActive] && [strongSelf.mainWindowController.window isVisible])
		 {
			 [self.mainWindowController.window close];
		 }
		 else
		 {
			 [strongSelf onShowMenuItem:strongSelf];
		 }
	 }];
	[[MASShortcutBinder sharedBinder] bindShortcutWithDefaultsKey:kPreferenceGlobalShortcutStartStop
														 toAction:^{
		 typeof(self) strongSelf = weakSelf;
		 if (strongSelf.lastKnownRunningTimeEntry == nil)
		 {
			 [strongSelf onContinueMenuItem:strongSelf];
		 }
		 else
		 {
			 [strongSelf onStopMenuItem:strongSelf];
		 }
	 }];

	// Start system notification
	[self.systemService registerSystemNotification];

	[[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:self];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(reachabilityChanged:)
												 name:kReachabilityChangedNotification
											   object:nil];

	self.reach = [Reachability reachabilityForInternetConnection];
	[self.reach startNotifier];

#ifdef SPARKLE
	if ([self updateCheckEnabled])
	{
		[[SUUpdater sharedUpdater] setAutomaticallyDownloadsUpdates:YES];

		NSAssert(ctx, @"ctx is not initialized, cannot continue");
		char *str = toggl_get_update_channel(ctx);

		NSAssert(str, @"Could not read update channel value");
		NSString *channel = [NSString stringWithUTF8String:str];

		free(str);
		[Utils setUpdaterChannel:channel];

		[[SUUpdater sharedUpdater] setDelegate:self.aboutWindowController];
		[[SUUpdater sharedUpdater] checkForUpdatesInBackground];
	}
#endif

	// Listen for system shutdown, to automatically stop timer. Experimental feature.
	[[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
														   selector:@selector(systemWillPowerOff:)
															   name:NSWorkspaceWillPowerOffNotification
															 object:nil];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(mainWillMinimize:)
												 name:NSWindowWillMiniaturizeNotification
											   object:nil];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(mainWillRestore:)
												 name:NSWindowDidDeminiaturizeNotification
											   object:nil];

	// Setup Google Service Callback
	[self registerGoogleEventHandler];

    // Validate the apple user
    #ifdef APP_STORE
    if (@available(macOS 10.15, *)) {
        [[AppleAuthenticationService shared] validateCredentialState];
    }
    #endif
}

- (void)systemWillPowerOff:(NSNotification *)aNotification
{
	NSLog(@"System will power off");
	// FIXME: we could stop timer here, if its running and user has configured
	// the app to stop the timer automatically.
}

- (void)mainWillMinimize:(NSNotification *)aNotification
{
	if (self.mainWindowController.window.level == NSFloatingWindowLevel)
	{
		self.onTop = YES;
		[self.mainWindowController setWindowMode:WindowModeDefault];
	}
}

- (void)mainWillRestore:(NSNotification *)aNotification
{
	if (self.onTop)
	{
		[self.mainWindowController setWindowMode:WindowModeAlwaysOnTop];
		self.onTop = NO;
	}
}

- (BOOL)updateCheckEnabled
{
	if (![[UnsupportedNotice sharedInstance] validateOSVersion])
	{
		return NO;
	}

	if (![self.environment isEqualToString:@"production"])
	{
		return NO;
	}
#ifdef DEBUG
	return NO;

#else
	return YES;

#endif
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center
	 shouldPresentNotification:(NSUserNotification *)notification
{
	return YES;
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center
			   didDismissAlert:(NSUserNotification *)notification
{
	NSLog(@"didDismissAlert %@", notification);
	// When other notification button is pressed
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center
	   didActivateNotification:(NSUserNotification *)notification
{
	NSLog(@"didActivateNotification %@", notification);

	// handle user clicking on notification alert
	if (NSUserNotificationActivationTypeContentsClicked == notification.activationType)
	{
		[self onShowMenuItem:self];
		return;
	}

	// handle autotracker notification
	if (notification && notification.userInfo)
	{
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			if (notification.userInfo[@"autotracker"] != nil)
			{
				NSNumber *project_id = notification.userInfo[@"project_id"];
				NSNumber *task_id = notification.userInfo[@"task_id"];
				NSLog(@"Handle autotracker notification project_id = %@, task_id = %@", project_id, task_id);
				char_t *guid = toggl_start(ctx, "", "", task_id.longValue, project_id.longValue, 0, "", false, 0, 0);
				free(guid);
				return;
			}

			// handle pomodoro timer
			if (notification.userInfo[@"pomodoro"] != nil)
			{
				if (NSUserNotificationActivationTypeActionButtonClicked != notification.activationType)
				{
					toggl_show_app(ctx);
				}
				else
				{
					toggl_continue_latest(ctx, false);
				}
				return;
			}

			// handle pomodoro_break timer
			if (notification.userInfo[@"pomodoro_break"] != nil)
			{
				if (NSUserNotificationActivationTypeActionButtonClicked != notification.activationType)
				{
					toggl_show_app(ctx);
				}
				else
				{
					toggl_continue_latest(ctx, false);
				}
				return;
			}

			// handle reminder track button press
			if (notification.userInfo[@"reminder"] != nil)
			{
				char_t *guid = toggl_start(ctx, "", "", 0, 0, 0, "", false, 0, 0);
				free(guid);
				return;
			}
		});
	}
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center
		didDeliverNotification:(NSUserNotification *)notification
{
	NSLog(@"didDeliverNotification %@", notification);
}

- (void)startNew:(NSNotification *)notification
{
	[self new:notification.object];
}

- (void)new:(TimeEntryViewItem *)new_time_entry
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	NSAssert(new_time_entry != nil, @"new time entry details cannot be nil");

    // Start or create empty TE from Timer mode
    NSString *duration = self.manualMode ? @"0" : new_time_entry.duration;

	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		const char *tag_list = [[new_time_entry.tags componentsJoinedByString:@"\t"] UTF8String];

		char *guid = toggl_start(ctx,
								 [new_time_entry.Description UTF8String],
								 [duration UTF8String],
								 new_time_entry.TaskID,
								 new_time_entry.ProjectID,
								 0,
								 tag_list,
								 false,
								 0,
								 0);

		if (new_time_entry.billable)
		{
			toggl_set_time_entry_billable(ctx, guid, new_time_entry.billable);
		}
        NSString *GUID = [NSString stringWithUTF8String:guid];
		free(guid);

        // Focus on the created one
        if (self.manualMode) {
            toggl_edit(ctx, [GUID UTF8String], false, kFocusedFieldNameDescription);
        }
    });
}

- (void)startContinueTimeEntry:(NSNotification *)notification
{
	[self continueTimeEntry:notification.object];
}

- (void)continueTimeEntry:(NSString *)guid
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		if (guid == nil)
		{
			toggl_continue_latest(ctx, false);
		}
		else
		{
			toggl_continue(ctx, [guid UTF8String]);
		}
	});
}

- (void)startStop:(NSNotification *)notification
{
	[self stop];
}

- (void)stop
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		toggl_stop(ctx, false);
	});
}

- (void)startToggleGroup:(NSNotification *)notification
{
	[self toggleGroup:notification.object];
}

- (void)toggleGroup:(NSString *)key
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		toggl_toggle_entries_group(ctx, [key UTF8String]);
	});
}

- (void)startUpdateIconTooltip:(NSNotification *)notification
{
	[self updateIconTooltip:notification.object];
}

- (void)updateIconTooltip:(NSString *)text
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	[self.statusItem setToolTip:text];
}

- (void)startDisplaySettings:(NSNotification *)notification
{
	[self displaySettings:notification.object];
}

- (void)displaySettings:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	// Start idle detection, if its enabled
	if (cmd.settings.idle_detection)
	{
		NSLog(@"Starting idle detection");
		if (!self.idleTimer)
		{
			self.idleTimer = [NSTimer
							  scheduledTimerWithTimeInterval:1.0
													  target:self
													selector:@selector(idleTimerFired:)
													userInfo:nil
													 repeats:YES];
		}
	}
	else
	{
		NSLog(@"Idle detection is disabled. Stopping idle detection.");
		if (self.idleTimer != nil)
		{
			[self.idleTimer invalidate];
			self.idleTimer = nil;
		}
	}

	// Touch Bar
	[self handleTouchBarWithSettings:cmd.settings];

	// Start menubar timer if its enabled
	self.showMenuBarTimer = cmd.settings.menubar_timer;
	if (cmd.settings.menubar_timer)
	{
		NSLog(@"Starting menubar timer");
		if (!self.menubarTimer)
		{
			self.menubarTimer = [NSTimer
								 scheduledTimerWithTimeInterval:1.0
														 target:self
													   selector:@selector(menubarTimerFired:)
													   userInfo:nil
														repeats:YES];
		}
	}
	else
	{
		NSLog(@"Menubar timer is disabled. Stopping menubar timer.");
		if (self.menubarTimer != nil)
		{
			[self.menubarTimer invalidate];
			self.menubarTimer = nil;
		}
		[self updateStatusItem];
	}

	if (self.showMenuBarProject != cmd.settings.menubar_project)
	{
		// Show/Hide project in menubar
		self.showMenuBarProject = cmd.settings.menubar_project;
		[self updateStatusItem];
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
		[self.mainWindowController setWindowMode:WindowModeAlwaysOnTop];
	}
	else
	{
		[self.mainWindowController setWindowMode:WindowModeDefault];
	}

	self.onTop = cmd.settings.on_top;

	if (cmd.open)
	{
		self.preferencesWindowController.originalCmd = cmd;
		self.preferencesWindowController.user_id = self.lastKnownUserID;
		[self.preferencesWindowController showWindowAndFocus];
	}

	NSString *mode = kToggleTimerMode;
	self.manualMode = cmd.settings.manual_mode;
	if (cmd.settings.manual_mode)
	{
		mode = kToggleManualMode;
		[self.manualModeMenuItem setTitle:@"Use timer"];
	}
	else
	{
		[self.manualModeMenuItem setTitle:@"Use manual mode"];
	}
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:mode
																object:nil];
}

- (void)startDisplayApp:(NSNotification *)notification
{
	[self displayApp:notification.object];
}

- (void)displayApp:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	if (cmd.open)
	{
		[self onShowMenuItem:self];
	}
}

- (void)startDisplayPromotion:(NSNotification *)notification
{
	[self displayPromotion:notification.object];
}

- (void)displayPromotion:(NSNumber *)promotion_type
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSLog(@"displayPromotion %d", promotion_type.intValue);

	// OSX app knows only about beta channel promotion
	if (kPromotionJoinBetaChannel != promotion_type.intValue)
	{
		return;
	}

	NSAlert *alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"Let's do it!"];
	[alert addButtonWithTitle:@"No thanks"];
	[alert setMessageText:@"Join Team Beta?"];
	[alert setInformativeText:@"Hi there! Would you like to join Toggl Desktop Beta program to get cool gear and check out the hot new features as they come out of the oven?"];
	[alert setAlertStyle:NSInformationalAlertStyle];
	NSInteger result = [alert runModal];

	toggl_set_promotion_response(ctx, promotion_type.intValue, NSAlertFirstButtonReturn == result);
}

- (void)startDisplayLogin:(NSNotification *)notification
{
	[self displayLogin:notification.object];
}

- (void)displayLogin:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.lastKnownUserID = cmd.user_id;

	if (!self.lastKnownUserID)
	{
		// maybe its running, but we dont know any more
		[self indicateStoppedTimer];
	}
	// Set email address
	char *str = toggl_get_user_email(ctx);
	NSString *email = [NSString stringWithUTF8String:str];
	free(str);
	[self.currentUserEmailMenuItem setTitle:email];
}

- (void)startDisplayOnlineState:(NSNotification *)notification
{
	[self displayOnlineState:notification.object];
}

- (void)displayOnlineState:(NSNumber *)state
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.lastKnownOnlineState = ![state intValue];
	[self updateStatusItem];
}

- (void)startDisplaySyncState:(NSNotification *)notification
{
	[self displaySyncState:notification.object];
}

- (void)displaySyncState:(NSNumber *)state
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	if ([state intValue])
	{
		// [[SystemMessage shared] presentSyncing];
	}
	else
	{
		// [[SystemMessage shared] dismissSyncing];
	}
}

- (void)startDisplayUnsyncedItems:(NSNotification *)notification
{
	[self displayUnsyncedItems:notification.object];
}

- (void)displayUnsyncedItems:(NSNumber *)count
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	// FIXME: hide/show number of unsynced time entries
}

- (void)updateStatusItem
{
	NSString *title = @"";

	if (self.lastKnownRunningTimeEntry && self.lastKnownUserID)
	{
		if (self.showMenuBarProject)
		{
			title = [title stringByAppendingString:self.lastKnownRunningTimeEntry.ProjectLabel];
		}

		if (self.showMenuBarTimer)
		{
			char *str = toggl_format_tracked_time_duration(self.lastKnownRunningTimeEntry.duration_in_seconds);

			if (self.showMenuBarProject)
			{
				title = [NSString stringWithFormat:@"%@ (%@)", title, [NSString stringWithUTF8String:str]];
			}
			else
			{
				title = [title stringByAppendingString:[NSString stringWithUTF8String:str]];
			}

			free(str);
		}

		// Update tooltip
		[self.statusItem setToolTip:[NSString stringWithFormat:@"Total today: %@", self.lastKnownRunningTimeEntry.dateDuration]];
	}

	NSString *key = nil;
	if (self.lastKnownRunningTimeEntry && self.lastKnownUserID)
	{
		if (self.lastKnownOnlineState)
		{
			key = @"on";
		}
		else
		{
			key = @"offline_on";
		}
	}
	else
	{
		if (self.lastKnownOnlineState)
		{
			key = @"off";
		}
		else
		{
			key = @"offline_off";
		}
	}
	NSImage *image = [self.statusImages objectForKey:key];
	NSAssert(image, @"status image not found!");

	if (![title isEqualToString:self.statusItem.title])
	{
		if (self.statusItem.title.length == 0)
		{
			// If previous value was empty set title twice to fix cut off issue
			[self.statusItem setTitle:title];
		}
		[self.statusItem setTitle:title];
	}

	if (image != self.statusItem.image)
	{
		[self.statusItem setImage:image];
        if (@available(macOS 10.12.2, *)) {
            [self.touchItem update:image];
        }
	}
}

- (void)startDisplayTimerState:(NSNotification *)notification
{
	[self displayTimerState:notification.object];
}

- (void)displayTimerState:(TimeEntryViewItem *)timeEntry
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.lastKnownRunningTimeEntry = timeEntry;

	if (!timeEntry)
	{
		if (self.idleNotificationWindowController.window.isVisible)
		{
			[self.idleNotificationWindowController.window orderOut:nil];
		}
	}

	if (timeEntry)
	{
		if (!self.willTerminate)
		{
			[NSApp setApplicationIconImage:[AppIconFactory appIconWithType:AppIconTypeActive]];
		}

		[self updateStatusItem];

		// Change tracking time entry row in menu
		if (self.lastKnownRunningTimeEntry.Description
			&& self.lastKnownRunningTimeEntry.Description.length)
		{
			[self.runningTimeEntryMenuItem setTitle:self.lastKnownRunningTimeEntry.Description];
		}
		else
		{
			[self.runningTimeEntryMenuItem setTitle:@"Timer is tracking"];
		}

		return;
	}

	[self indicateStoppedTimer];
}

- (void)indicateStoppedTimer
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	if (!self.willTerminate)
	{
		// Change app dock icon to default
		// See https://developer.apple.com/library/mac/documentation/Carbon/Conceptual/customizing_docktile/dockconcepts.pdf
		[NSApp setApplicationIconImage:[AppIconFactory appIconWithType:AppIconTypeDefault]];
	}

	[self updateStatusItem];

	[self.runningTimeEntryMenuItem setTitle:@"Timer is not tracking"];
}

- (NSMenu *)applicationDockMenu:(NSApplication *)sender {
	NSMenu *menu = [[NSMenu alloc] init];

	[menu addItemWithTitle:@"Start New"
					action:@selector(onNewMenuItem:)
			 keyEquivalent:@"n"].tag = kMenuItemTagNew;
	[menu addItemWithTitle:@"Continue Latest"
					action:@selector(onContinueMenuItem:)
			 keyEquivalent:@"o"].tag = kMenuItemTagContinue;
	[menu addItemWithTitle:@"Stop Timer"
					action:@selector(onStopMenuItem:)
			 keyEquivalent:@"s"].tag = kMenuItemTagStop;
	return menu;
}

- (void)createStatusItem
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	NSMenu *menu = [[NSMenu alloc] init];
	self.currentUserEmailMenuItem = [menu addItemWithTitle:@""
													action:nil
											 keyEquivalent:@""];
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
	[menu addItemWithTitle:@"Edit"
					action:@selector(onEditMenuItem:)
			 keyEquivalent:@"e"].tag = kMenuItemTagEdit;
	[menu addItem:[NSMenuItem separatorItem]];
	[menu addItemWithTitle:@"Sync"
					action:@selector(onSyncMenuItem:)
			 keyEquivalent:@"r"].tag = kMenuItemTagSync;
	[menu addItemWithTitle:@"Reports"
					action:@selector(onOpenBrowserMenuItem:)
			 keyEquivalent:@""].tag = kMenuItemTagOpenBrowser;
	[menu addItemWithTitle:@"Preferences"
					action:@selector(onPreferencesMenuItem:)
			 keyEquivalent:@","];
	self.manualModeMenuItem = [menu addItemWithTitle:@"Use manual mode"
											  action:@selector(onModeChange:)
									   keyEquivalent:@"d"];
	self.manualModeMenuItem.tag = kMenuItemTagMode;
	[menu addItem:[NSMenuItem separatorItem]];
	[menu addItemWithTitle:@"About"
					action:@selector(onAboutMenuItem:)
			 keyEquivalent:@""];
	[menu addItemWithTitle:@"Send Feedback"
					action:@selector(onSendFeedbackMenuItem)
			 keyEquivalent:@""].tag = kMenuItemTagSendFeedBack;
	[menu addItemWithTitle:@"Logout"
					action:@selector(onLogoutMenuItem:)
			 keyEquivalent:@""].tag = kMenuItemTagLogout;
	[menu addItemWithTitle:@"Quit"
					action:@selector(onQuitMenuItem)
			 keyEquivalent:@"q"];

	NSStatusBar *bar = [NSStatusBar systemStatusBar];

	self.statusImages = [[NSMutableDictionary alloc] init];
	for (NSString *key in @[@"on", @"off", @"offline_on", @"offline_off"])
	{
		NSImage *image = [NSImage imageNamed:key];
		[image setTemplate:YES];
		[self.statusImages setObject:image forKey:key];
	}

	self.statusItem = [bar statusItemWithLength:NSVariableStatusItemLength];
	[self.statusItem setHighlightMode:YES];
	[self.statusItem setEnabled:YES];
	[self.statusItem setMenu:menu];

	[self updateStatusItem];
}

- (void)onNewMenuItem:(id)sender
{
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kCommandNew
																object:[[TimeEntryViewItem alloc] init]];
}

- (void)onSendFeedbackMenuItem
{
	[self.feedbackWindowController showWindowAndFocus];
}

- (void)onSendFeedbackMainMenuItem:(id)sender
{
	[self onSendFeedbackMenuItem];
}

- (IBAction)onContinueMenuItem:(id)sender
{
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kCommandContinue
																object:nil];
}

- (IBAction)onStopMenuItem:(id)sender
{
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kCommandStop
																object:nil];
}

- (IBAction)onSyncMenuItem:(id)sender
{
	toggl_sync(ctx);
}

- (IBAction)onModeChange:(id)sender
{
	self.manualMode = !self.manualMode;
	toggl_set_settings_manual_mode(ctx, self.manualMode);
}

- (IBAction)onOpenBrowserMenuItem:(id)sender
{
	toggl_open_in_browser(ctx);
}

- (IBAction)onHelpMenuItem:(id)sender
{
	toggl_get_support(ctx, 1);
}

- (IBAction)onLogoutMenuItem:(id)sender
{
    // Reset the apple state
    #ifdef APP_STORE
    if (@available(macOS 10.15, *)) {
        [[AppleAuthenticationService shared] reset];
    }
    #endif

	// Reset the sign up state for the Empty View
	// Because the Time Entry list present last 9 weeks, so it's no way to know that it's new user or old user
	[[NSUserDefaults standardUserDefaults] setBool:NO forKey:kUserHasBeenSignup];
	[[NSUserDefaults standardUserDefaults] synchronize];

	// Logout
	toggl_logout(ctx);
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
	toggl_clear_cache(ctx);
}

- (IBAction)onAboutMenuItem:(id)sender
{
	[self.aboutWindowController showWindowAndFocus];
	[self.aboutWindowController checkForUpdates];
}

- (IBAction)onViewChangelogMenuItem:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://toggl.github.io/toggldesktop"]];
}

- (IBAction)onShowMenuItem:(id)sender
{
	[self.mainWindowController showWindowAndFocus];
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kFocusTimer
																object:nil];
}

- (IBAction)onEditMenuItem:(id)sender
{
	[self.mainWindowController showWindowAndFocus];
	toggl_edit(ctx, "", true, "description");
}

- (IBAction)onPreferencesMenuItem:(id)sender
{
	toggl_edit_preferences(ctx);
}

- (IBAction)onHideMenuItem:(id)sender
{
	NSArray *orderedWindows = [NSApp orderedWindows];
	NSWindow *frontWindow = orderedWindows[0];

	[frontWindow close];
}

- (void)onQuitMenuItem
{
	[[NSApplication sharedApplication] terminate:self];
}

- (void)applicationWillTerminate:(NSNotification *)app
{
	NSLog(@"applicationWillTerminate");
	self.willTerminate = YES;
	[self.reach stopNotifier];
	toggl_context_clear(ctx);
	ctx = 0;

	if (self.aboutWindowController.restart == YES)
	{
		float seconds = 1.0;
		NSTask *task = [[NSTask alloc] init];
		NSMutableArray *args = [NSMutableArray array];
		[args addObject:@"-c"];
		[args addObject:[NSString stringWithFormat:@"sleep %f; open \"%@\"", seconds, [[NSBundle mainBundle] bundlePath]]];
		[task setLaunchPath:@"/bin/sh"];
		[task setArguments:args];
		[task launch];
	}
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
	}
}

- (id)init
{
	self = [super init];

	BOOL logUIToFile;
#ifdef DEBUG
	self.environment = @"development";
	logUIToFile = NO;
#else
	self.environment = @"production";
	logUIToFile = YES;
#endif

	NSDictionary *infoDict = [[NSBundle mainBundle] infoDictionary];
	self.version = infoDict[@"CFBundleShortVersionString"];

	// Disallow duplicate instances in production
	if ([self.environment isEqualToString:@"production"])
	{
		[Utils disallowDuplicateInstances];
	}

	[Bugsnag startBugsnagWithApiKey:@"aa13053a88d5133b688db0f25ec103b7"];
	NSAssert(self.environment != nil, @"Missing environment in plist");
	[Bugsnag configuration].releaseStage = self.environment;

	self.app_path = [Utils applicationSupportDirectory:self.environment];
	self.db_path = [self.app_path stringByAppendingPathComponent:@"kopsik.db"];
	self.log_path = [self.app_path stringByAppendingPathComponent:@"toggl_desktop.log"];
	self.log_level = @"debug";
	self.systemService = [[SystemService alloc] init];
	self.isAddedTouchBar = NO;

	[self parseCommandLineArguments];

	NSLog(@"Starting with db path %@, log path %@, log level %@",
		  self.db_path, self.log_path, self.log_level);

	toggl_set_log_path([self.log_path UTF8String]);
	toggl_set_log_level([self.log_level UTF8String]);

	ctx = toggl_context_init([appName UTF8String], [self.version UTF8String]);

	// Using sparkle instead of self updater:
	toggl_disable_update_check(ctx);

	toggl_on_sync_state(ctx, on_sync_state);
	toggl_on_unsynced_items(ctx, on_unsynced_items);
	toggl_on_show_app(ctx, on_app);
	toggl_on_error(ctx, on_error);
	toggl_on_overlay(ctx, on_overlay);
	toggl_on_online_state(ctx, on_online_state);
	toggl_on_login(ctx, on_login);
	toggl_on_url(ctx, on_url);
	toggl_on_reminder(ctx, on_reminder);
	toggl_on_pomodoro(ctx, on_pomodoro);
	toggl_on_pomodoro_break(ctx, on_pomodoro_break);
	toggl_on_time_entry_list(ctx, on_time_entry_list);
	toggl_on_time_entry_autocomplete(ctx, on_time_entry_autocomplete);
	toggl_on_mini_timer_autocomplete(ctx, on_mini_timer_autocomplete);
	toggl_on_project_autocomplete(ctx, on_project_autocomplete);
	toggl_on_workspace_select(ctx, on_workspace_select);
	toggl_on_client_select(ctx, on_client_select);
	toggl_on_tags(ctx, on_tags);
	toggl_on_time_entry_editor(ctx, on_time_entry_editor);
	toggl_on_settings(ctx, on_settings);
	toggl_on_timer_state(ctx, on_timer_state);
	toggl_on_idle_notification(ctx, on_idle_notification);
	toggl_on_autotracker_rules(ctx, on_autotracker_rules);
	toggl_on_autotracker_notification(ctx, on_autotracker_notification);
	toggl_on_promotion(ctx, on_promotion);
	toggl_on_project_colors(ctx, on_project_colors);
	toggl_on_countries(ctx, on_countries);
	toggl_on_timeline(ctx, on_timeline);
    toggl_on_message(ctx, on_display_message);
    toggl_on_onboarding(ctx, on_display_onboarding);

	NSLog(@"Version %@", self.version);

	NSString *cacertPath = [[NSBundle mainBundle] pathForResource:@"cacert" ofType:@"pem"];
	toggl_set_cacert_path(ctx, [cacertPath UTF8String]);

	bool_t res = toggl_set_db_path(ctx, [self.db_path UTF8String]);
	if (!res)
	{
		NSLog(@"Failed to initialize database at %@", self.db_path);

		NSAlert *alert = [[NSAlert alloc] init];
		[alert setMessageText:@"Failed to initialize database"];
		NSString *informative = [NSString stringWithFormat:
								 @"Make sure you have permissions to write to folder \"%@\"", self.app_path];
		[alert setInformativeText:informative];
		[alert setAlertStyle:NSCriticalAlertStyle];
		[alert runModal];
		[NSApp terminate:nil];
		return nil;
	}

	if (logUIToFile)
	{
		NSLog(@"Redirecting UI log to file");
		NSString *logPath =
			[self.app_path stringByAppendingPathComponent:@"ui.log"];
		freopen([logPath fileSystemRepresentation], "a+", stderr);
	}

	// Start time entry when user pastes text into the app
	[NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask handler:^NSEvent * (NSEvent *theEvent) {
		 if ([theEvent modifierFlags] & NSCommandKeyMask)
		 {
			 NSString *character = [theEvent charactersIgnoringModifiers];
			 NSString *windowName = [NSApp orderedWindows][0].frameAutosaveName;

			 NSResponder *currentFirstResponder =  [self.mainWindowController.window firstResponder];
			 BOOL timerFocused = [currentFirstResponder isKindOfClass:[NSTextView class]];

	         // Execute only if edit view is not opened and focus is on main window
			 if ([character isEqualToString:@"v"]
				 && ![self.mainWindowController isEditOpened]
				 && [windowName isEqualToString:@"MainWindow"]
				 && !timerFocused)
			 {
				 if (self.lastKnownRunningTimeEntry == nil || self.lastKnownRunningTimeEntry.duration_in_seconds < 0)
				 {
					 NSPasteboard *pasteboard  = [NSPasteboard generalPasteboard];
					 NSString *clipboardText = [pasteboard stringForType:NSPasteboardTypeString];

					 dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
										toggl_start(ctx,
													[clipboardText UTF8String],
													nil,
													0,
													0,
													0,
													0,
													false,
													0,
													0);
									});
				 }
			 }
		 }

		 return theEvent;
	 }];

	NSLog(@"AppDelegate init done");

	return self;
}

- (void)menubarTimerFired:(NSTimer *)timer
{
	[self updateStatusItem];
}

- (void)idleTimerFired:(NSTimer *)timer
{
	uint64_t idle_seconds = 0;

	if (0 != get_idle_time(&idle_seconds))
	{
		NSLog(@"Achtung! Failed to get idle status.");
		return;
	}

	toggl_set_idle_seconds(ctx, idle_seconds);
}

- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)anItem
{
	switch ([anItem tag])
	{
		case kMenuItemTagContinue :
			if (!self.lastKnownUserID)
			{
				return NO;
			}
			if (self.lastKnownRunningTimeEntry != nil)
			{
				return NO;
			}
			break;
		case kMenuItemTagStop :
		case kMenuItemTagEdit :
			if (!self.lastKnownUserID)
			{
				return NO;
			}
			if (self.lastKnownRunningTimeEntry == nil)
			{
				return NO;
			}
			break;
		case kMenuItemTagMode :
		case kMenuItemTagSync :
		case kMenuItemTagLogout :
		case kMenuItemTagClearCache :
		case kMenuItemTagOpenBrowser :
		case kMenuItemTagNew :
		case kMenuItemTagSendFeedBack :
			if (!self.lastKnownUserID)
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

- (void)startDisplayIdleNotification:(NSNotification *)notification
{
	[self displayIdleNotification:notification.object];
}

- (void)displayIdleNotification:(IdleEvent *)idleEvent
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	[self.idleNotificationWindowController displayIdleEvent:idleEvent];
}

- (void)reachabilityChanged:(NSNotification *)notice
{
	NetworkStatus remoteHostStatus = [self.reach currentReachabilityStatus];

	if (ctx && remoteHostStatus != NotReachable)
	{
		toggl_set_online(ctx);
	}
}

void on_online_state(const int64_t state)
{
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayOnlineState
																object:[NSNumber numberWithLong:state]];
}

void on_sync_state(const int64_t state)
{
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplaySyncState
																object:[NSNumber numberWithLong:state]];
}

void on_unsynced_items(const int64_t count)
{
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayUnsyncedItems
																object:[NSNumber numberWithLong:count]];
}

void on_login(const bool_t open, const uint64_t user_id)
{
	[[Bugsnag configuration] setUser:[NSString stringWithFormat:@"%lld", user_id]
							withName:nil
							andEmail:nil];

	DisplayCommand *cmd = [[DisplayCommand alloc] init];
	cmd.open = open;
	cmd.user_id = user_id;

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayLogin
																object:cmd];
}

void on_reminder(const char *title, const char *informative_text)
{
	[[UserNotificationCenter share] scheduleReminderWithTitle:[NSString stringWithUTF8String:title]
											  informativeText:[NSString stringWithUTF8String:informative_text]];
}

void on_pomodoro(const char *title, const char *informative_text)
{
	[[UserNotificationCenter share] schedulePomodoroWithTitle:[NSString stringWithUTF8String:title]
											  informativeText:[NSString stringWithUTF8String:informative_text]];
}

void on_pomodoro_break(const char *title, const char *informative_text)
{
	[[UserNotificationCenter share] schedulePomodoroBreakWithTitle:[NSString stringWithUTF8String:title]
												   informativeText:[NSString stringWithUTF8String:informative_text]];
}

void on_url(const char *url)
{
	// Capture url
	NSString *reportURL = [NSString stringWithUTF8String:url];

	dispatch_async(dispatch_get_main_queue(), ^{
		[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:reportURL]];
	});
}

void on_time_entry_list(const bool_t open,
						TogglTimeEntryView *first,
						const bool_t show_load_more)
{
	NSString *todayTotal = @"Total today: 0h 0min";
	NSMutableArray *viewitems = [[NSMutableArray alloc] init];
	TogglTimeEntryView *it = first;

	while (it)
	{
        TimeEntryViewItem *model = [[TimeEntryViewItem alloc] initWithView:it];
		[viewitems addObject:model];
		if ([model.formattedDate isEqual:@"Today"])
		{
			todayTotal = [NSString stringWithFormat:@"Total today: %@", model.dateDuration];
		}
		it = it->Next;
	}

	DisplayCommand *cmd = [[DisplayCommand alloc] init];
	cmd.open = open;
	cmd.timeEntries = viewitems;
	cmd.show_load_more = show_load_more;
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayTimeEntryList
																object:cmd];
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kUpdateIconTooltip
																object:todayTotal];
}

void on_timeline(const bool_t open,
				 const char_t *date,
				 TogglTimelineChunkView *first,
				 TogglTimeEntryView *first_entry,
				 long start_day,
				 long end_day)
{
	TimelineDisplayCommand *cmd =
		[[TimelineDisplayCommand alloc] initWithOpen:open
												date:[NSString stringWithUTF8String:date]
									   firstActivity:first
										  firstEntry:first_entry
											startDay:start_day
											  endDay:end_day];

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayTimeline
																object:cmd];
}

void on_time_entry_autocomplete(TogglAutocompleteView *first)
{
	NSArray *items = [AutocompleteItem loadAll:first];

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayTimeEntryAutocomplete
																object:items];
	dispatch_async(dispatch_get_main_queue(), ^{
		[[DescriptionTimeEntryStorage shared] updateWith:items];
	});
}

void on_mini_timer_autocomplete(TogglAutocompleteView *first)
{
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayMinitimerAutocomplete
																object:[AutocompleteItem loadAll:first]];
}

void on_project_autocomplete(TogglAutocompleteView *first)
{
	NSArray *items = [AutocompleteItem loadAll:first];

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayProjectAutocomplete
																object:items];

	dispatch_async(dispatch_get_main_queue(), ^{
		[[ProjectStorage shared] updateWith:items];
	});
}

void on_tags(TogglGenericView *first)
{
	NSArray<ViewItem *> *viewItems = [ViewItem loadAll:first];

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayTags
																object:viewItems];
	dispatch_async(dispatch_get_main_queue(), ^{
		[[TagStorage shared] updateWith:viewItems];
	});
}

void on_autotracker_rules(TogglAutotrackerRuleView *first, const uint64_t title_count, char_t *title_list[])
{
	NSMutableArray *titles = [[NSMutableArray alloc] init];

	for (int i = 0; i < title_count; i++)
	{
		NSString *title = [NSString stringWithUTF8String:title_list[i]];
		[titles addObject:title];
	}
	NSDictionary *data = @{
			@"rules": [AutotrackerRuleItem loadAll:first],
			@"titles": titles
	};
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayAutotrackerRules
																object:data];
}

void on_autotracker_notification(const char_t *project_name,
								 const uint64_t project_id,
								 const uint64_t task_id)
{
	[[UserNotificationCenter share] scheduleAutoTrackerWithProjectName:[NSString stringWithUTF8String:project_name]
															 projectID:[NSNumber numberWithLong:project_id]
																taskID:[NSNumber numberWithLong:task_id]];
}

void on_promotion(const int64_t promotion_type)
{
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayPromotion
																object:[NSNumber numberWithLong:promotion_type]];
}

void on_client_select(TogglGenericView *first)
{
	NSArray<ViewItem *> *viewItems = [ViewItem loadAll:first];

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayClientSelect
																object:viewItems];
	dispatch_async(dispatch_get_main_queue(), ^{
		[[ClientStorage shared] updateWith:viewItems];
	});
}

void on_workspace_select(TogglGenericView *first)
{
	NSArray<ViewItem *> *viewItems = [ViewItem loadAll:first];

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayWorkspaceSelect
																object:viewItems];
	dispatch_async(dispatch_get_main_queue(), ^{
		[[WorkspaceStorage shared] updateWith:viewItems];
	});
}

void on_time_entry_editor(const bool_t open,
						  TogglTimeEntryView *te,
						  const char *focused_field_name)
{
    TimeEntryViewItem *item = [[TimeEntryViewItem alloc] initWithView:te];
	DisplayCommand *cmd = [[DisplayCommand alloc] init];
	cmd.open = open;
	cmd.timeEntry = item;
	cmd.timeEntry.focusedFieldName = [NSString stringWithUTF8String:focused_field_name];
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayTimeEntryEditor
																object:cmd];
}

void on_app(const bool_t open)
{
	DisplayCommand *cmd = [[DisplayCommand alloc] init];

	cmd.open = open;
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayApp
																object:cmd];
}

void on_error(const char *errmsg, const bool_t is_user_error)
{
	NSString *msg = [NSString stringWithUTF8String:errmsg];

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayError
																object:msg];
	if (!is_user_error)
	{
		char *str = toggl_get_update_channel(ctx);
		NSString *channel = [NSString stringWithUTF8String:str];
		free(str);

		[Bugsnag notify:[NSException exceptionWithName:msg reason:msg userInfo:nil]
				  block:^(BugsnagCrashReport *report) {
			 NSDictionary *data = @{
					 @"channel": channel
			 };
			 [report addMetadata:data toTabWithName:@"metadata"];
		 }];
	}
}

void on_overlay(const int64_t type)
{
//    [NSNotificationCenter defaultCenter] post
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayOverlay
																object:[NSNumber numberWithLong:type]];
}

void on_settings(const bool_t open,
				 TogglSettingsView *settings)
{
	Settings *s = [[Settings alloc] init];

	[s load:settings];

	DisplayCommand *cmd = [[DisplayCommand alloc] init];
	cmd.open = open;
	cmd.settings = s;
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplaySettings
																object:cmd];
}

void on_timer_state(TogglTimeEntryView *te)
{
	TimeEntryViewItem *view_item = nil;

	if (te)
	{
        view_item = [[TimeEntryViewItem alloc] initWithView:te];
	}
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayTimerState
																object:view_item];
}

void on_idle_notification(
	const char *guid,
	const char *since,
	const char *duration,
	const int64_t started,
	const char *description)
{
	IdleEvent *idleEvent = [[IdleEvent alloc] init];

	idleEvent.guid = [NSString stringWithUTF8String:guid];
	idleEvent.since = [NSString stringWithUTF8String:since];
	idleEvent.duration = [NSString stringWithUTF8String:duration];
	idleEvent.started = started;
	idleEvent.timeEntryDescription = [NSString stringWithUTF8String:description];
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayIdleNotification
																object:idleEvent];
}

void on_project_colors(
	char_t *list[],
	const uint64_t count)
{
	NSMutableArray *colors = [NSMutableArray array];

	for (int i = 0; i < count; i++)
	{
		[colors addObject:[NSString stringWithUTF8String:list[i]]];
	}
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kSetProjectColors
																object:colors];
}

void on_countries(TogglCountryView *first)
{
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayCountries
																object:[CountryViewItem loadAll:first]];
}

#pragma mark - Google Authentication

- (void)registerGoogleEventHandler
{
	NSAppleEventManager *appleEventManager = [NSAppleEventManager sharedAppleEventManager];

	[appleEventManager setEventHandler:self
						   andSelector:@selector(handleGetURLEvent:withReplyEvent:)
						 forEventClass:kInternetEventClass
							andEventID:kAEGetURL];
}

- (void)handleGetURLEvent:(NSAppleEventDescriptor *)event
		   withReplyEvent:(NSAppleEventDescriptor *)replyEvent
{
	NSString *URLString = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];
	NSURL *URL = [NSURL URLWithString:URLString];

	[_currentAuthorizationFlow resumeExternalUserAgentFlowWithURL:URL];
}

- (NSString *)currentChannel
{
	char *str = toggl_get_update_channel(ctx);
	NSString *channel = [NSString stringWithUTF8String:str];

	free(str);
	return channel;
}

- (void)handleTouchBarWithSettings:(Settings *)settings
{
    if (@available(macOS 10.12.2, *)) {
        [TouchBarService shared].isEnabled = settings.showTouchBar;
        self.mainWindowController.touchBar = nil;
    }
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kTouchBarSettingChanged object:@(settings.showTouchBar)];
}

#pragma mark - In app message

void on_display_message(const char *title,
                        const char *text,
                        const char *button,
                        const char *url)
{
    InAppMessage *message = [[InAppMessage alloc] initWithTitle:[NSString stringWithUTF8String:title]
                                                       subTitle:[NSString stringWithUTF8String:text]
                                                    buttonTitle:[NSString stringWithUTF8String:button]
                                                      urlAction:[NSString stringWithUTF8String:url]];
    [[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kStartDisplayInAppMessage object:message];

}

- (void) invalidAppleUserCrendentialNotification:(NSNotification *) noti
{
    [self onLogoutMenuItem:self];
    [[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayError
                                                                object:@"Invalid Apple session. Please try login again."];
}

void on_display_onboarding(const int64_t onboarding_type) {
    [[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kStartDisplayOnboarding object:[NSNumber numberWithInteger:onboarding_type]];
}
@end
