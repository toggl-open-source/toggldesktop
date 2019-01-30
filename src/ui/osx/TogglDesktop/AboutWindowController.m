//
//  AboutWindowController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 29/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "AboutWindowController.h"
#import "UIEvents.h"
#import "toggl_api.h"
#import "DisplayCommand.h"
#import "Utils.h"
#import "UnsupportedNotice.h"
#import "NSTextFieldClickablePointer.h"

static NSString *const kTogglDesktopGithubURL = @"https://github.com/toggl/toggldesktop";

@interface AboutWindowController () <NSTextFieldDelegate>
@property (weak) IBOutlet NSTextField *appnameTextField;
@property (weak) IBOutlet NSTextField *versionTextField;
@property (weak) IBOutlet NSTextField *updateStatusTextField;
@property (weak) IBOutlet NSComboBox *updateChannelComboBox;
@property (weak) IBOutlet NSTextField *updateChannelLabel;
@property (weak) IBOutlet NSButton *restartButton;
@property (weak) IBOutlet NSTextFieldClickablePointer *findUsInGithub;
@property (weak) IBOutlet NSBox *boxView;

@end

@implementation AboutWindowController

extern void *ctx;

- (void)windowDidLoad
{
	[super windowDidLoad];

	[self.window setBackgroundColor:[NSColor colorWithPatternImage:[NSImage imageNamed:@"background-pattern"]]];

	NSDictionary *infoDict = [[NSBundle mainBundle] infoDictionary];
	NSString *version = [infoDict objectForKey:@"CFBundleShortVersionString"];
	[self.versionTextField setStringValue:[NSString stringWithFormat:@"Version %@", version]];
	NSString *appname = [[infoDict objectForKey:@"CFBundleName"] uppercaseString];
	[self.appnameTextField setStringValue:appname];

	self.windowHasLoad = YES;
	self.restart = NO;

	char *str = toggl_get_update_channel(ctx);
	self.updateChannelComboBox.stringValue = [NSString stringWithUTF8String:str];
	free(str);

	if ([self updateCheckEnabled])
	{
		self.updateChannelComboBox.hidden = NO;
		self.updateChannelLabel.hidden = NO;

		if (![[SUUpdater sharedUpdater] updateInProgress])
		{
			[self checkForUpdates];
		}

		[self displayUpdateStatus];
	}

	[self initCommon];
}

- (void)initCommon {
	self.findUsInGithub.delegate = self;

	self.findUsInGithub.titleUnderline = YES;

	self.boxView.wantsLayer = YES;
	self.boxView.layer.masksToBounds = NO;
	self.boxView.shadow = [[NSShadow alloc] init];
	self.boxView.layer.shadowColor = [NSColor colorWithWhite:0 alpha:0.1].CGColor;
	self.boxView.layer.shadowOpacity = 1.0;
	self.boxView.layer.shadowOffset = CGSizeMake(0, -2);
	self.boxView.layer.shadowRadius = 6;
}

- (BOOL)updateCheckEnabled
{
#ifdef DEBUG
	return NO;

#else
	return YES;

#endif
}

- (void)checkForUpdates
{
	if (![[UnsupportedNotice sharedInstance] validateOSVersion])
	{
		return;
	}
	if (![self updateCheckEnabled])
	{
		return;
	}
	[[SUUpdater sharedUpdater] resetUpdateCycle];
	[[SUUpdater sharedUpdater] checkForUpdatesInBackground];
}

- (void)updater:(SUUpdater *)updater willInstallUpdateOnQuit:(SUAppcastItem *)item immediateInstallationInvocation:(NSInvocation *)invocation
{
	NSLog(@"Download finished: %@", item.displayVersionString);

	self.updateStatus =
		[NSString stringWithFormat:@"Restart app to upgrade to %@",
		 item.displayVersionString];

	[self displayUpdateStatus];
	[self.restartButton setHidden:NO];
}

- (void)updater:(SUUpdater *)updater didFindValidUpdate:(SUAppcastItem *)update
{
	NSLog(@"update found: %@", update.displayVersionString);

	self.updateStatus =
		[NSString stringWithFormat:@"Update found: %@. Downloading..",
		 update.displayVersionString];

	[self.restartButton setHidden:YES];
	[self displayUpdateStatus];
	[Utils runClearCommand];
}

- (void)displayUpdateStatus
{
	NSLog(@"automaticallyDownloadsUpdates=%d", [[SUUpdater sharedUpdater] automaticallyDownloadsUpdates]);

	if (self.updateStatus)
	{
		self.updateStatusTextField.stringValue = self.updateStatus;
		[self.updateStatusTextField setHidden:NO];
	}
	else
	{
		[self.updateStatusTextField setHidden:YES];
	}
}

- (BOOL)isVisible
{
	if (!self.windowHasLoad)
	{
		return NO;
	}
	return self.window.isVisible;
}

- (IBAction)updateChannelSelected:(id)sender
{
	NSString *updateChannel = self.updateChannelComboBox.stringValue;

	toggl_set_update_channel(ctx, [updateChannel UTF8String]);

	[Utils setUpdaterChannel:updateChannel];

	[self checkForUpdates];
}

- (IBAction)clickRestartButton:(id)sender;
{
	self.restart = YES;
	[[NSApplication sharedApplication] terminate:nil];
}

- (void)updaterDidNotFindUpdate:(SUUpdater *)update
{
	NSLog(@"No update found");
}

- (void)updater:(SUUpdater *)updater willInstallUpdate:(SUAppcastItem *)update
{
	NSLog(@"Will install update %@", update.displayVersionString);
}

- (void)updater:(SUUpdater *)updater didAbortWithError:(NSError *)error
{
	NSLog(@"Update check failed with error %@", error);
}

- (void)textFieldClicked:(id)sender
{
	if (sender == self.findUsInGithub)
	{
		[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:kTogglDesktopGithubURL]];
		return;
	}
}

@end
