//
//  PreferencesWindowController.m
//
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import "PreferencesWindowController.h"
#import "AutotrackerRuleItem.h"
#import "DisplayCommand.h"
#import "Settings.h"
#import "UIEvents.h"
#import "Utils.h"
#import "AutocompleteItem.h"
#import "AutocompleteDataSource.h"
#import "NSCustomComboBox.h"
#import <MASShortcut/Shortcut.h>
#import "TogglDesktop-Swift.h"
#import "DesktopLibraryBridge.h"

NSString *const kPreferenceGlobalShortcutShowHide = @"TogglDesktopGlobalShortcutShowHide";
NSString *const kPreferenceGlobalShortcutStartStop = @"TogglDesktopGlobalShortcutStartStop";

typedef enum : NSUInteger
{
	TabIndexGeneral,
	TabIndexProxy,
	TabIndexAutotracker,
	TabIndexReminder
} TabIndex;

@interface PreferencesWindowController () <NSTextFieldDelegate, NSTableViewDataSource, NSComboBoxDataSource, NSComboBoxDelegate, NSWindowDelegate>
@property (strong, nonatomic) Settings *settings;
@property (weak) IBOutlet NSButton *stopOnShutdownCheckbox;
@property (weak) IBOutlet NSTextField *hostTextField;
@property (weak) IBOutlet NSTextField *portTextField;
@property (weak) IBOutlet NSTextField *usernameTextField;
@property (weak) IBOutlet NSTextField *passwordTextField;
@property (weak) IBOutlet NSButton *useIdleDetectionButton;
@property (weak) IBOutlet NSButton *usePomodoroButton;
@property (weak) IBOutlet NSButton *usePomodoroBreakButton;
@property (weak) IBOutlet NSButton *menubarTimerCheckbox;
@property (weak) IBOutlet NSButton *menubarProjectCheckbox;
@property (weak) IBOutlet NSButton *dockIconCheckbox;
@property (weak) IBOutlet NSButton *ontopCheckbox;
@property (weak) IBOutlet NSButton *reminderCheckbox;
@property (weak) IBOutlet MASShortcutView *showHideShortcutView;
@property (weak) IBOutlet MASShortcutView *startStopShortcutView;
@property (weak) IBOutlet NSTextField *idleMinutesTextField;
@property (weak) IBOutlet NSTextField *pomodoroMinutesTextField;
@property (weak) IBOutlet NSTextField *pomodoroBreakMinutesTextField;
@property (weak) IBOutlet NSButton *focusOnShortcutCheckbox;
@property (weak) IBOutlet NSTextField *reminderMinutesTextField;
@property (weak) IBOutlet NSComboBox *autotrackerTerm;
@property (weak) IBOutlet NSCustomComboBox *autotrackerProject;
@property (weak) IBOutlet NSCustomComboBox *defaultProject;
@property (weak) IBOutlet NSTableView *autotrackerRulesTableView;
@property (weak) IBOutlet NSButton *remindMon;
@property (weak) IBOutlet NSButton *remindTue;
@property (weak) IBOutlet NSButton *remindWed;
@property (weak) IBOutlet NSButton *remindThu;
@property (weak) IBOutlet NSButton *remindFri;
@property (weak) IBOutlet NSButton *remindSat;
@property (weak) IBOutlet NSButton *remindSun;
@property (weak) IBOutlet NSTextField *remindStarts;
@property (weak) IBOutlet NSTextField *remindEnds;
@property (weak) IBOutlet NSButton *autotrack;
@property (weak) IBOutlet NSButton *openEditorOnShortcut;
@property (weak) IBOutlet NSButton *proxyDoNot;
@property (weak) IBOutlet NSButton *proxySystem;
@property (weak) IBOutlet NSButton *proxyToggl;
@property (weak) IBOutlet NSButton *addAutotrackerRuleButton;
@property (weak) IBOutlet NSButton *changeDurationButton;
@property (weak) IBOutlet NSSegmentedControl *tabSegment;
@property (weak) IBOutlet NSTabView *tabView;
@property (weak) IBOutlet NSButton *showTouchBarButton;
@property (weak) IBOutlet NSLayoutConstraint *bottomContainerHeight;
@property (weak) IBOutlet NSButton *permissionBtn;

@property (nonatomic, assign) NSInteger selectedProxyIndex;
@property (nonatomic, strong) NSArray<AutotrackerRuleItem *> *rules;
@property (nonatomic, strong) AutocompleteDataSource *autotrackerProjectAutocompleteDataSource;
@property (nonatomic, strong) AutocompleteDataSource *defaultProjectAutocompleteDataSource;
@property (nonatomic, strong) NSArray<NSString *> *termAutocompleteItems;
@property (nonatomic, assign) TabIndex currentTab;

- (IBAction)idleMinutesChange:(id)sender;
- (IBAction)pomodoroMinutesChange:(id)sender;
- (IBAction)pomodoroBreakMinutesChange:(id)sender;
- (IBAction)proxyRadioChanged:(id)sender;
- (IBAction)hostTextFieldChanged:(id)sender;
- (IBAction)portTextFieldChanged:(id)sender;
- (IBAction)usernameTextFieldChanged:(id)sender;
- (IBAction)passwordTextFieldChanged:(id)sender;
- (IBAction)useIdleDetectionButtonChanged:(id)sender;
- (IBAction)usePomodoroButtonChanged:(id)sender;
- (IBAction)usePomodoroBreakButtonChanged:(id)sender;
- (IBAction)menubarTimerCheckboxChanged:(id)sender;
- (IBAction)menubarProjectCheckboxChanged:(id)sender;
- (IBAction)dockIconCheckboxChanged:(id)sender;
- (IBAction)ontopCheckboxChanged:(id)sender;
- (IBAction)reminderCheckboxChanged:(id)sender;
- (IBAction)focusOnShortcutCheckboxChanged:(id)sender;
- (IBAction)reminderMinutesChanged:(id)sender;
- (IBAction)addAutotrackerRule:(id)sender;
- (IBAction)remindWeekChanged:(id)sender;
- (IBAction)deleteAutotrackerRule:(id)sender;
- (IBAction)remindTimesChanged:(id)sender;
- (IBAction)autotrackChanged:(id)sender;
- (IBAction)openEditorOnShortcut:(id)sender;
- (IBAction)defaultProjectSelected:(id)sender;
- (IBAction)changeDurationButtonChanged:(id)sender;
- (IBAction)touchBarButtonChanged:(id)sender;

@end

@implementation PreferencesWindowController

extern void *ctx;

- (id)initWithWindowNibName:(NSString *)nibNameOrNil
{
	self = [super initWithWindowNibName:nibNameOrNil];
	if (self)
	{
		self.autotrackerProjectAutocompleteDataSource = [[AutocompleteDataSource alloc] initWithNotificationName:kDisplayProjectAutocomplete];
		self.defaultProjectAutocompleteDataSource = [[AutocompleteDataSource alloc] initWithNotificationName:kDisplayProjectAutocomplete];

		self.termAutocompleteItems = [NSArray<NSString *> array];

		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(startDisplayAutotrackerRules:)
													 name:kDisplayAutotrackerRules
												   object:nil];
	}
	return self;
}

- (void)windowDidLoad
{
	[super windowDidLoad];

	// Clean window titlebar
	self.window.delegate = self;
	self.window.titleVisibility = NSWindowTitleHidden;
	self.window.titlebarAppearsTransparent = YES;
	self.window.styleMask |= NSFullSizeContentViewWindowMask;

	self.currentTab = TabIndexGeneral;
	self.autotrackerProjectAutocompleteDataSource.combobox = self.autotrackerProject;
	self.autotrackerProjectAutocompleteDataSource.combobox.dataSource = self.autotrackerProjectAutocompleteDataSource;
	[self.autotrackerProjectAutocompleteDataSource setFilter:@""];

	self.defaultProjectAutocompleteDataSource.combobox = self.defaultProject;
	self.defaultProjectAutocompleteDataSource.combobox.dataSource = self.defaultProjectAutocompleteDataSource;
	[self.defaultProjectAutocompleteDataSource setFilter:@""];

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

	[self enableLoggedInUserControls];

	[self displaySettings:self.originalCmd];

	NSMutableDictionary *autotrackerData = [[NSMutableDictionary alloc] init];
	if (self.rules != nil)
	{
		autotrackerData[@"rules"] = self.rules;
	}
	if (self.termAutocompleteItems != nil)
	{
		autotrackerData[@"titles"] = self.termAutocompleteItems;
	}
	[self displayAutotrackerRules:autotrackerData];

	[self.idleMinutesTextField setDelegate:self];
	[self.pomodoroMinutesTextField setDelegate:self];
	[self.pomodoroBreakMinutesTextField setDelegate:self];
	[self.reminderMinutesTextField setDelegate:self];
}

- (void)enableLoggedInUserControls
{
	[self.defaultProject setEnabled:self.user_id != 0];
	[self.autotrack setEnabled:self.user_id != 0];
	[self.autotrackerTerm setEnabled:self.user_id != 0];
	[self.autotrackerProject setEnabled:self.user_id != 0];
	[self.addAutotrackerRuleButton setEnabled:self.user_id != 0];
}

- (IBAction)changeDurationButtonChanged:(id)sender
{
	toggl_set_keep_end_time_fixed(ctx, [Utils stateToBool:[self.changeDurationButton state]]);
}

- (IBAction)touchBarButtonChanged:(id)sender
{
	toggl_set_settings_show_touch_bar(ctx, [Utils stateToBool:[self.showTouchBarButton state]]);
}

- (IBAction)proxyRadioChanged:(id)sender
{
	NSButton *radioBtn = (NSButton *)sender;

	self.selectedProxyIndex = radioBtn.tag;

	[self saveProxySettings];

	toggl_set_settings_autodetect_proxy(ctx,
										(kUseSystemProxySettings == self.selectedProxyIndex));
}

- (IBAction)defaultProjectSelected:(id)sender
{
	NSString *key = self.defaultProject.stringValue;
	AutocompleteItem *autocomplete = [self.defaultProjectAutocompleteDataSource get:key];
	uint64_t pid = 0;
	uint64_t tid = 0;

	if (autocomplete != nil)
	{
		pid = autocomplete.ProjectID;
		tid = autocomplete.TaskID;
	}

	toggl_set_default_project(ctx, pid, tid);
}

- (IBAction)useIdleDetectionButtonChanged:(id)sender
{
	toggl_set_settings_use_idle_detection(ctx,
										  [Utils stateToBool:[self.useIdleDetectionButton state]]);
}

- (IBAction)usePomodoroButtonChanged:(id)sender
{
	NSLog(@"Changing pomodoro mode");
	toggl_set_settings_pomodoro(ctx,
								[Utils stateToBool:[self.usePomodoroButton state]]);
}

- (IBAction)usePomodoroBreakButtonChanged:(id)sender
{
	NSLog(@"Changing pomodoro_break mode");
	toggl_set_settings_pomodoro_break(ctx,
									  [Utils stateToBool:[self.usePomodoroBreakButton state]]);
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

const int kUseNoProxy = 0;
const int kUseSystemProxySettings = 1;
const int kUseProxyToConnectToToggl = 2;

- (void)saveProxySettings
{
	NSLog(@"saveProxySettings");

	NSString *host = [self.hostTextField stringValue];
	NSInteger port = [self.portTextField integerValue];
	NSString *username = [self.usernameTextField stringValue];
	NSString *password = [self.passwordTextField stringValue];

	toggl_set_proxy_settings(ctx,
							 (kUseProxyToConnectToToggl == self.selectedProxyIndex),
							 [host UTF8String],
							 (unsigned int)port,
							 [username UTF8String],
							 [password UTF8String]);
}

- (void)remindWeekChanged:(id)sender
{
	toggl_set_settings_remind_days(ctx,
								   [Utils stateToBool:self.remindMon.state],
								   [Utils stateToBool:self.remindTue.state],
								   [Utils stateToBool:self.remindWed.state],
								   [Utils stateToBool:self.remindThu.state],
								   [Utils stateToBool:self.remindFri.state],
								   [Utils stateToBool:self.remindSat.state],
								   [Utils stateToBool:self.remindSun.state]);
}

- (IBAction)deleteAutotrackerRule:(id)sender
{
	NSLog(@"deleteAutotrackerRule");

	NSInteger i = self.autotrackerRulesTableView.selectedRow;
	if (self.rules && i >= 0 && i < self.rules.count)
	{
		AutotrackerRuleItem *view = self.rules[i];
		toggl_autotracker_delete_rule(ctx, view.ID);
	}
}

- (void)remindTimesChanged:(id)sender
{
	toggl_set_settings_remind_times(ctx,
									[self.remindStarts.stringValue UTF8String],
									[self.remindEnds.stringValue UTF8String]);
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
	[self displayLogin:notification.object];
}

- (void)displayLogin:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.user_id = cmd.user_id;

	[self enableLoggedInUserControls];

	if (!self.user_id)
	{
		self.defaultProject.stringValue = @"";

		[self displayAutotrackerRules:@{}];
	}
}

- (void)startDisplayAutotrackerRules:(NSNotification *)notification
{
	[self displayAutotrackerRules:notification.object];
}

- (void)displayAutotrackerRules:(NSDictionary *)data
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	@synchronized(self)
	{
		self.rules = [data[@"rules"] copy];
		self.termAutocompleteItems = [data[@"titles"] copy];
	}
	[self.autotrackerRulesTableView reloadData];
	[self.autotrackerTerm reloadData];
}

- (void)startDisplaySettings:(NSNotification *)notification
{
	[self displaySettings:notification.object];
}

- (void)displaySettings:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	Settings *settings = cmd.settings;
    self.settings = settings;

	[self.useIdleDetectionButton setState:[Utils boolToState:settings.idle_detection]];
	[self.usePomodoroButton setState:[Utils boolToState:settings.pomodoro]];
	[self.usePomodoroBreakButton setState:[Utils boolToState:settings.pomodoro_break]];
	[self.menubarTimerCheckbox setState:[Utils boolToState:settings.menubar_timer]];
	[self.menubarProjectCheckbox setState:[Utils boolToState:settings.menubar_project]];
	[self.dockIconCheckbox setState:[Utils boolToState:settings.dock_icon]];
	[self.ontopCheckbox setState:[Utils boolToState:settings.on_top]];
	[self.reminderCheckbox setState:[Utils boolToState:settings.reminder]];
	[self.focusOnShortcutCheckbox setState:[Utils boolToState:settings.focus_on_shortcut]];
	[self.stopOnShutdownCheckbox setState:[Utils boolToState:settings.stopWhenShutdown]];

	if (!settings.use_proxy && !settings.autodetect_proxy)
	{
		[self selectProxyRadioWithTag:kUseNoProxy];
	}

	if (settings.use_proxy)
	{
		[self selectProxyRadioWithTag:kUseProxyToConnectToToggl];
	}
	[self.hostTextField setStringValue:settings.proxy_host];
	[self.portTextField setIntegerValue:settings.proxy_port];
	[self.usernameTextField setStringValue:settings.proxy_username];
	[self.passwordTextField setStringValue:settings.proxy_password];

	[self.hostTextField setEnabled:settings.use_proxy];
	[self.portTextField setEnabled:settings.use_proxy];
	[self.usernameTextField setEnabled:settings.use_proxy];
	[self.passwordTextField setEnabled:settings.use_proxy];

	self.idleMinutesTextField.integerValue = settings.idle_minutes;
	self.idleMinutesTextField.enabled = settings.idle_detection;

	self.usePomodoroBreakButton.enabled = settings.pomodoro;
	self.pomodoroMinutesTextField.integerValue = settings.pomodoro_minutes;
	self.pomodoroMinutesTextField.enabled = settings.pomodoro;
	self.pomodoroBreakMinutesTextField.integerValue = settings.pomodoro_break_minutes;
	self.pomodoroBreakMinutesTextField.enabled = settings.pomodoro_break;

	self.reminderMinutesTextField.integerValue = settings.reminder_minutes;
	self.reminderMinutesTextField.enabled = settings.reminder;

	if (settings.autodetect_proxy)
	{
		[self selectProxyRadioWithTag:kUseSystemProxySettings];
	}

	[self.remindMon setState:[Utils boolToState:settings.remind_mon]];
	[self.remindTue setState:[Utils boolToState:settings.remind_tue]];
	[self.remindWed setState:[Utils boolToState:settings.remind_wed]];
	[self.remindThu setState:[Utils boolToState:settings.remind_thu]];
	[self.remindFri setState:[Utils boolToState:settings.remind_fri]];
	[self.remindSat setState:[Utils boolToState:settings.remind_sat]];
	[self.remindSun setState:[Utils boolToState:settings.remind_sun]];

	self.remindStarts.stringValue = settings.remind_starts;
	self.remindEnds.stringValue = settings.remind_ends;

	[self.autotrack setState:[Utils boolToState:settings.autotrack]];

	[self.openEditorOnShortcut setState:[Utils boolToState:settings.open_editor_on_shortcut]];

	char_t *default_project_name = toggl_get_default_project_name(ctx);
	if (default_project_name)
	{
		self.defaultProject.stringValue = [NSString stringWithUTF8String:default_project_name];
	}
	else
	{
		self.defaultProject.stringValue = @"";
	}
	free(default_project_name);

	[self.changeDurationButton setState:[Utils boolToState:toggl_get_keep_end_time_fixed(ctx)]];

	if (@available(macOS 10.12.2, *))
	{
		self.showTouchBarButton.hidden = NO;
		self.bottomContainerHeight.constant = 58;
		[self.showTouchBarButton setState:[Utils boolToState:toggl_get_show_touch_bar(ctx)]];
	}
	else
	{
		self.showTouchBarButton.hidden = YES;
		self.bottomContainerHeight.constant = 38;
	}

    // Permission for Auto Tracker
    self.permissionBtn.hidden = settings.autotrack ? [ObjcSystemPermissionManager isScreenRecordingPermissionGranted] : YES;
}

- (void)selectProxyRadioWithTag:(NSInteger)tag
{
	self.proxyDoNot.state = NSControlStateValueOff;
	self.proxyToggl.state = NSControlStateValueOff;
	self.proxySystem.state = NSControlStateValueOff;

	switch (tag)
	{
		case kUseNoProxy :
			self.proxyDoNot.state = NSControlStateValueOn;
			break;
		case kUseSystemProxySettings :
			self.proxySystem.state = NSControlStateValueOn;
			break;
		case kUseProxyToConnectToToggl :
			self.proxyToggl.state = NSControlStateValueOn;
			break;
		default :
			break;
	}
}

- (IBAction)idleMinutesChange:(id)sender
{
	toggl_set_settings_idle_minutes(ctx,
									[self.idleMinutesTextField.stringValue intValue]);
}

- (IBAction)pomodoroMinutesChange:(id)sender
{
	NSLog(@"Changing pomodoro minutes");
	toggl_set_settings_pomodoro_minutes(ctx,
										[self.pomodoroMinutesTextField.stringValue intValue]);
}

- (IBAction)pomodoroBreakMinutesChange:(id)sender
{
	NSLog(@"Changing pomodoro_break minutes");
	toggl_set_settings_pomodoro_break_minutes(ctx,
											  [self.pomodoroBreakMinutesTextField.stringValue intValue]);
}

- (IBAction)reminderMinutesChanged:(id)sender
{
	toggl_set_settings_reminder_minutes(ctx,
										[self.reminderMinutesTextField.stringValue intValue]);
}

- (IBAction)autotrackChanged:(id)sender
{
    [[DesktopLibraryBridge shared] enableAutoTracker:[Utils stateToBool:self.autotrack.state]];
}

- (IBAction)openEditorOnShortcut:(id)sender
{
	toggl_set_settings_open_editor_on_shortcut(ctx, [Utils stateToBool:self.openEditorOnShortcut.state]);
}

- (IBAction)addAutotrackerRule:(id)sender
{
	NSString *term = [self.autotrackerTerm stringValue];

	if (nil == term || 0 == term.length)
	{
		[self.autotrackerTerm.window makeFirstResponder:self.autotrackerTerm];
		return;
	}

	NSString *key = self.autotrackerProject.stringValue;
	AutocompleteItem *autocomplete = [self.autotrackerProjectAutocompleteDataSource get:key];
	uint64_t pid = 0;
	uint64_t tid = 0;
	if (autocomplete != nil)
	{
		pid = autocomplete.ProjectID;
		tid = autocomplete.TaskID;
	}
	if (!pid && !tid)
	{
		[self.autotrackerProject.window makeFirstResponder:self.autotrackerProject];
		return;
	}

	if (!toggl_autotracker_add_rule(ctx, [term UTF8String], pid, tid, "", "", 0))
	{
		return;
	}

	self.autotrackerTerm.stringValue = @"";
	self.autotrackerProject.stringValue = @"";
}

- (IBAction)stopOnShutdownAndSleepOnChange:(NSButton *)sender
{
	toggl_set_settings_stop_entry_on_shutdown_sleep(ctx, [Utils stateToBool:[self.stopOnShutdownCheckbox state]]);
}

// NSTableViewDataSource - autotracker rules table

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	if (!self.rules)
	{
		return 0;
	}
	return self.rules.count;
}

- (id)              tableView:(NSTableView *)aTableView
	objectValueForTableColumn:(NSTableColumn *)aTableColumn
						  row:(NSInteger)rowIndex
{
	if (rowIndex < 0 || !self.rules || rowIndex > self.rules.count)
	{
		return nil;
	}
	AutotrackerRuleItem *view = self.rules[rowIndex];

	NSString *columnIdentifer = [aTableColumn identifier];
	if ([columnIdentifer isEqualTo:@"project"])
	{
		return view.ProjectAndTaskLabel;
	}
	if ([columnIdentifer isEqualTo:@"term"])
	{
		return view.Term;
	}
	return nil;
}

// NSTextFieldDelegate

- (void)controlTextDidChange:(NSNotification *)aNotification
{
	if ([aNotification object] == self.idleMinutesTextField)
	{
		toggl_set_settings_idle_minutes(ctx,
										[self.idleMinutesTextField.stringValue intValue]);
	}

	if ([aNotification object] == self.pomodoroMinutesTextField)
	{
		toggl_set_settings_pomodoro_minutes(ctx,
											[self.pomodoroMinutesTextField.stringValue intValue]);
	}

	if ([aNotification object] == self.pomodoroBreakMinutesTextField)
	{
		toggl_set_settings_pomodoro_break_minutes(ctx,
												  [self.pomodoroBreakMinutesTextField.stringValue intValue]);
	}

	if ([aNotification object] == self.reminderMinutesTextField)
	{
		toggl_set_settings_reminder_minutes(ctx,
											[self.reminderMinutesTextField.stringValue intValue]);
	}

	if ([aNotification object] == self.autotrackerProject)
	{
		NSCustomComboBox *comboBox = [aNotification object];
		NSString *filter = [comboBox stringValue];

		AutocompleteDataSource *dataSource = self.autotrackerProjectAutocompleteDataSource;

		if (dataSource != nil)
		{
			[dataSource setFilter:filter];
		}

		if (!filter || ![filter length] || (dataSource != nil && !dataSource.count))
		{
			if ([comboBox isExpanded] == YES)
			{
				[comboBox setExpanded:NO];
			}
			return;
		}

		if ([comboBox isExpanded] == NO)
		{
			[comboBox setExpanded:YES];
		}
	}
}

// Term autocomplete datasource

- (NSString *)comboBox:(NSComboBox *)comboBox completedString:(NSString *)partialString
{
	for (int i = 0; i < self.termAutocompleteItems.count; i++)
	{
		NSString *s = self.termAutocompleteItems[i];
		if ([[s commonPrefixWithString:partialString options:NSCaseInsensitiveSearch] length] == [partialString length])
		{
			return s;
		}
	}
	return @"";
}

// Term combobox delegate

- (NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox
{
	return self.termAutocompleteItems.count;
}

- (id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)index
{
	return self.termAutocompleteItems[index];
}

- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)string
{
	return [self.termAutocompleteItems indexOfObject:string];
}

// Handle autotracker rule delete using backspace key

- (void)keyDown:(NSEvent *)event
{
	if ((event.keyCode == kVK_Delete) && ([self.window firstResponder] == self.autotrackerRulesTableView))
	{
		[self deleteAutotrackerRule:self];
	}
	else
	{
		[super keyDown:event];
	}
}

- (IBAction)tabSegmentOnChange:(id)sender
{
	[self.tabView selectTabViewItemAtIndex:self.tabSegment.selectedSegment];
}

- (IBAction)permissionBtnOnClick:(id)sender
{
    [ObjcSystemPermissionManager tryGrantScreenRecordingPermission];
}

@end
